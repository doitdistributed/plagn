/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagTcp.hpp
 * @author Gerrit Erichsen (saxomophon@gmx.de)
 * @contributors:
 * @brief Holds the PlagTcp class
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

#ifndef PLAGTCP_HPP
#define PLAGTCP_HPP

// std includes
#include <string>

// own includes
#include "Plag.hpp"
#include "TcpClient.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagTcp class is a Plag to interact via raw TCP sockets
 *
 * @details Acts as a TCP client. Configured with a remote IP and port, it connects and
 * forwards raw string payloads in both directions.
 */
class PlagTcp : public Plag
{
public:
    PlagTcp(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagTcp();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    // config parameters
    std::string m_remoteIp;     //!< IP address of the remote TCP server
    uint16_t m_port;            //!< port of the remote TCP server

    // worker members
    std::shared_ptr<TcpClient> m_tcpClient; //!< TCP client transport layer
};

#endif // PLAGTCP_HPP
