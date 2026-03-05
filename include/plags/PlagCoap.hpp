/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagCoap.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagCoap class
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

#ifndef PLAGCOAP_HPP
#define PLAGCOAP_HPP

// std includes
#include <string>

// libcoap includes
#include <coap3/coap.h>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagCoap class is a Plag to interact via CoAP (RFC 7252)
 *
 * @details Acts as a CoAP client. Connects to a CoAP server and GETs or observes a configured
 * resource. Incoming responses are distributed as DatagramUdp payloads. Outgoing datagrams
 * are sent as CoAP PUT requests to the configured resource path.
 */
class PlagCoap : public Plag
{
public:
    PlagCoap(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagCoap();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    static coap_response_t onResponse(coap_session_t * session,
                           const coap_pdu_t * sent,
                           const coap_pdu_t * received,
                           const coap_mid_t id);
    void handleResponse(const coap_pdu_t * received);
    void sendRequest(const std::string & payload, coap_pdu_code_t method);

private:
    // config parameters
    std::string m_host;         //!< CoAP server host
    uint16_t m_port;            //!< CoAP server port (default: 5683)
    std::string m_resourcePath; //!< resource path (e.g. "/sensors/temperature")
    bool m_observe;             //!< if true, send an Observe GET

    // worker members
    coap_context_t * m_coapContext;   //!< the libcoap context
    coap_session_t * m_coapSession;   //!< the CoAP session

    // static pointer to self for C-style callback routing
    static PlagCoap * s_instance;
};

#endif // PLAGCOAP_HPP
