/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagWebSocket.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagWebSocket class
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
#include <iostream>

// boost includes
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

// own includes
#include "DatagramUdp.hpp"

// self include
#include "PlagWebSocket.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;
using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagWebSocket object and assigns default values
 * 
 */
PlagWebSocket::PlagWebSocket(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::WebSocket)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagWebSocket object, closes the WS connection cleanly
 * 
 */
PlagWebSocket::~PlagWebSocket()
{
    if (!m_stopToken) stopWork();
    try
    {
        if (m_ws && m_ws->is_open())
        {
            m_ws->close(websocket::close_code::normal);
        }
    }
    catch (exception & e)
    {
        cerr << "Could not close PlagWebSocket: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: host, port, and WebSocket path
 * 
 */
void PlagWebSocket::readConfig() try
{
    m_host = getParameter<string>("host");
    m_port = getOptionalParameter<uint16_t>("port", 80);
    m_path = getOptionalParameter<string>("path", string("/"));
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagWebSocket::readConfig()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagWebSocket::init() establishes the TCP+WebSocket connection
 * 
 */
void PlagWebSocket::init() try
{
    connectWs();
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagWebSocket::init()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief connectWs performs the TCP resolve, connect, and WebSocket handshake
 * 
 */
void PlagWebSocket::connectWs()
{
    tcp::resolver resolver(m_ioContext);
    auto const results = resolver.resolve(m_host, to_string(m_port));

    m_ws = make_unique<websocket::stream<tcp::socket>>(m_ioContext);
    boost::asio::connect(m_ws->next_layer(), results.begin(), results.end());

    // set the host header required for the WS handshake
    m_ws->set_option(websocket::stream_base::decorator(
        [this](websocket::request_type & req)
        {
            req.set(http::field::host, m_host);
            req.set(http::field::user_agent, "plagn/" + getName());
        }));

    m_ws->handshake(m_host, m_path);
    m_ws->text(true);   // send as text frames by default
    cout << "PlagWebSocket: connected to ws://" << m_host << ":" << m_port << m_path << endl;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagWebSocket::loopWork reads incoming WS frames and sends queued outgoing datagrams
 * 
 */
bool PlagWebSocket::loopWork() try
{
    bool somethingDone = false;

    if (!m_ws || !m_ws->is_open())
    {
        connectWs();
        return true;
    }

    // non-blocking check: try to read if data is ready
    if (m_ws->next_layer().available() > 0)
    {
        beast::flat_buffer buffer;
        m_ws->read(buffer);
        string payload = beast::buffers_to_string(buffer.data());
        shared_ptr<DatagramUdp> datagram(new DatagramUdp(getName(), m_host, m_port, payload));
        appendToDistribution(datagram);
        somethingDone = true;
    }

    // send outgoing datagrams as WS text frames
    if (m_incommingDatagrams.begin() != m_incommingDatagrams.end())
    {
        shared_ptr<DatagramUdp> castPtr = dynamic_pointer_cast<DatagramUdp>(m_incommingDatagrams.front());
        m_incommingDatagrams.pop_front();
        if (castPtr != nullptr && m_ws->is_open())
        {
            m_ws->write(boost::asio::buffer(castPtr->getPayload()));
            somethingDone = true;
        }
    }

    return somethingDone;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagWebSocket::loopWork()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramUdp datagrams and queues them for WS transmission
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagWebSocket::placeDatagram(const shared_ptr<Datagram> datagram) try
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
    errorMsg += "\nSomething happened in PlagWebSocket::placeDatagram()";
    throw runtime_error(errorMsg);
}
