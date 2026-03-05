/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagMcp.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagMcp class
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

#ifndef PLAGMCP_HPP
#define PLAGMCP_HPP

// std includes

// own includes
#include "Plag.hpp"
#include "layer/TcpClient.hpp"
#include "datagrams/DatagramMcp.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagMcp class is a Plag to interact via Model Context Protocol (MCP)
 * 
 */
class PlagMcp : public Plag
{
public:
    PlagMcp(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagMcp();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:

private:
    // config parameters
    std::string m_serverIp;
    uint16_t m_port;

    // worker members
    std::shared_ptr<TcpClient> m_tcpClient;
};

#endif // PLAGMCP_HPP
