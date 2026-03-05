/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagOpcua.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagOpcua class
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

#ifndef PLAGOPCUA_HPP
#define PLAGOPCUA_HPP

// std includes
#include <string>
#include <vector>

// open62541pp includes
#include <open62541pp/open62541pp.hpp>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagOpcua class is a Plag to interact via OPC UA (IEC 62541)
 *
 * @details Acts as an OPC UA client using open62541pp. Connects to a server, subscribes to
 * a configured list of node IDs, receives change notifications in loopWork(), and distributes
 * them as DatagramMap key/value pairs keyed by node display name.
 */
class PlagOpcua : public Plag
{
public:
    PlagOpcua(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagOpcua();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    // config parameters
    std::string m_serverUrl;                //!< OPC UA server URL, e.g. opc.tcp://localhost:4840
    std::vector<std::string> m_nodeIds;     //!< list of node IDs to read/monitor

    // worker members
    std::unique_ptr<opcua::Client> m_client; //!< open62541pp client
};

#endif // PLAGOPCUA_HPP
