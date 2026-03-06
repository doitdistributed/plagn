/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagCoap.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagCoap class
 * @version 0.1
 * @date 2026-03-05
 *
 * @copyright LGPL v2.1
 *
 * Targets of chosen license for:
 *      Users    : Please be so kind as to indicate your usage of this library by linking to the project
 *                 page, currently being: https://github.com/saxomophon/plagn
 *      Devs     : Your improvements to the code, should be available publicly under the same license.
 *                 That way, anyone will benefit from it.
 *      Corporate: Even you are either a User or a Developer. No charge will apply, no guarantee or
 *                 warranty will be given.
 *
 */

// std include
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
#include <iostream>
#include <cstring>

// own includes
#include "DatagramUdp.hpp"

// self include
#include "PlagCoap.hpp"

using namespace std;

// static instance pointer for C-style callback routing
PlagCoap * PlagCoap::s_instance = nullptr;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagCoap object and assigns default values
 * 
 */
PlagCoap::PlagCoap(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::CoAP),
    m_coapContext(nullptr),
    m_coapSession(nullptr)
{
    s_instance = this;
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagCoap object; frees the libcoap context and session
 * 
 */
PlagCoap::~PlagCoap()
{
    if (!m_stopToken) stopWork();
    if (m_coapSession)
    {
        coap_session_release(m_coapSession);
        m_coapSession = nullptr;
    }
    if (m_coapContext)
    {
        coap_free_context(m_coapContext);
        m_coapContext = nullptr;
    }
    coap_cleanup();
    s_instance = nullptr;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: host, port, resourcePath, and observe flag
 * 
 */
void PlagCoap::readConfig() try
{
    m_host = getParameter<string>("host");
    m_port = getOptionalParameter<uint16_t>("port", COAP_DEFAULT_PORT);
    m_resourcePath = getOptionalParameter<string>("resourcePath", string("/"));
    m_observe = getOptionalParameter<bool>("observe", false);
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagCoap::readConfig()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagCoap::init() creates the libcoap context and connects to the CoAP server
 * 
 */
void PlagCoap::init() try
{
    coap_startup();

    m_coapContext = coap_new_context(nullptr);
    if (!m_coapContext)
    {
        throw runtime_error("PlagCoap: failed to create CoAP context");
    }

    coap_register_response_handler(m_coapContext, PlagCoap::onResponse);

    // resolve the server address
    coap_address_t dst;
    coap_address_init(&dst);
    dst.addr.sin.sin_family = AF_INET;
    dst.addr.sin.sin_port = htons(m_port);
    if (inet_pton(AF_INET, m_host.c_str(), &dst.addr.sin.sin_addr) <= 0)
    {
        throw runtime_error("PlagCoap: invalid host address: " + m_host);
    }

    m_coapSession = coap_new_client_session(m_coapContext, nullptr, &dst, COAP_PROTO_UDP);
    if (!m_coapSession)
    {
        throw runtime_error("PlagCoap: failed to create CoAP session");
    }

    // send initial GET (with observe option if configured)
    sendRequest("", m_observe ? COAP_REQUEST_CODE_GET : COAP_REQUEST_CODE_GET);

    cout << "PlagCoap: connected to coap://" << m_host << ":" << m_port << m_resourcePath << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagCoap::init()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagCoap::loopWork dispatches libcoap I/O events and sends queued outgoing PUT requests
 * 
 */
bool PlagCoap::loopWork() try
{
    bool somethingDone = false;

    // run libcoap event loop once with a short timeout
    int result = coap_io_process(m_coapContext, 50);
    if (result > 0)
    {
        somethingDone = true;
    }

    // send outgoing datagrams as CoAP PUT requests
    if (m_incommingDatagrams.begin() != m_incommingDatagrams.end())
    {
        shared_ptr<DatagramUdp> castPtr = dynamic_pointer_cast<DatagramUdp>(m_incommingDatagrams.front());
        m_incommingDatagrams.pop_front();
        if (castPtr != nullptr)
        {
            sendRequest(castPtr->getPayload(), COAP_REQUEST_CODE_PUT);
            somethingDone = true;
        }
    }

    return somethingDone;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagCoap::loopWork()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief sendRequest builds and sends a CoAP PDU to the configured resource path
 *
 * @param payload optional payload for PUT/POST requests
 * @param method  CoAP method code
 */
void PlagCoap::sendRequest(const string & payload, coap_pdu_code_t method)
{
    coap_pdu_t * pdu = coap_pdu_init(COAP_MESSAGE_CON, method,
                                      coap_new_message_id(m_coapSession),
                                      coap_session_max_pdu_size(m_coapSession));
    if (!pdu)
    {
        cerr << "PlagCoap: failed to create PDU" << endl;
        return;
    }

    // add Uri-Path options
    coap_optlist_t * optList = nullptr;
    string path = m_resourcePath;
    if (!path.empty() && path[0] == '/')
    {
        path = path.substr(1);
    }
    coap_path_into_optlist(reinterpret_cast<const uint8_t *>(path.c_str()),
                           path.size(), COAP_OPTION_URI_PATH, &optList);
    if (m_observe)
    {
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_OBSERVE,
                                             COAP_OBSERVE_ESTABLISH, nullptr));
    }
    coap_add_optlist_pdu(pdu, &optList);
    coap_delete_optlist(optList);

    if (!payload.empty())
    {
        coap_add_data(pdu, payload.size(),
                      reinterpret_cast<const uint8_t *>(payload.c_str()));
    }

    coap_send(m_coapSession, pdu);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief onResponse is the C-style libcoap response callback
 * 
 */
coap_response_t PlagCoap::onResponse(coap_session_t * /*session*/,
                           const coap_pdu_t * /*sent*/,
                           const coap_pdu_t * received,
                           const coap_mid_t /*id*/)
{
    if (s_instance)
    {
        s_instance->handleResponse(received);
    }
    return COAP_RESPONSE_OK;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief handleResponse extracts the payload from a CoAP response and distributes it
 * 
 */
void PlagCoap::handleResponse(const coap_pdu_t * received)
{
    size_t len = 0;
    const uint8_t * data = nullptr;
    if (coap_get_data(received, &len, &data) && len > 0)
    {
        string payload(reinterpret_cast<const char *>(data), len);
        shared_ptr<DatagramUdp> datagram(new DatagramUdp(getName(), m_host, m_port, payload));
        appendToDistribution(datagram);
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramUdp datagrams and queues them for CoAP PUT transmission
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagCoap::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    const shared_ptr<DatagramUdp> castPtr = dynamic_pointer_cast<DatagramUdp>(datagram);
    if (castPtr != nullptr)
    {
        m_incommingDatagrams.push_back(datagram);
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagCoap::placeDatagram()";
    throw runtime_error(errorMsg);
}
