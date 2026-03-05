/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagWebSocket.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagWebSocket class
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

#ifndef PLAGWEBSOCKET_HPP
#define PLAGWEBSOCKET_HPP

// std includes
#include <string>
#include <memory>

// boost includes
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagWebSocket class is a Plag to interact via WebSocket (RFC 6455)
 *
 * @details Acts as a WebSocket client. Connects to a WS server, reads incoming text/binary
 * frames as DatagramUdp payloads, and sends outgoing datagrams as WebSocket text frames.
 */
class PlagWebSocket : public Plag
{
public:
    PlagWebSocket(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagWebSocket();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    void connectWs();

private:
    // config parameters
    std::string m_host;     //!< WebSocket server host
    uint16_t m_port;        //!< WebSocket server port
    std::string m_path;     //!< WebSocket endpoint path (e.g. "/ws")

    // worker members
    boost::asio::io_context m_ioContext;
    std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> m_ws;
};

#endif // PLAGWEBSOCKET_HPP
