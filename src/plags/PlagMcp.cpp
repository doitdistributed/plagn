/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagMcp.cpp
 * @author plagn AI Assitant
 * @contributors:
 * @brief Implements the PlagMcp class
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

// own includes

// self include
#include "PlagMcp.hpp"

using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagMcp object and assigns default values
 * 
 */
PlagMcp::PlagMcp(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::MCP)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagMcp object
 * 
 */
PlagMcp::~PlagMcp()
{
    if (!m_stopToken) stopWork();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads many optional parameters from config and assigns them to member values
 * 
 */
void PlagMcp::readConfig() try
{
    m_serverIp = getOptionalParameter<std::string>("serverIp", "127.0.0.1");
    m_port = getOptionalParameter<uint16_t>("port", 8080);
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagMcp::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagMcp::init() configures the interface
 * 
 */
void PlagMcp::init() try
{
    m_tcpClient = std::make_shared<TcpClient>(std::chrono::milliseconds(1000), *this, m_serverIp, m_port);
    m_tcpClient->connect();
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagMcp::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// ... (rest of the includes)

// ... (constructor and destructor)

// ... (readConfig and init)

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagMcp::loopWork regularly reads on the socket, if data popped in or sends data
 * 
 */
bool PlagMcp::loopWork() try
{
    if (m_tcpClient->isConnected() && m_tcpClient->getAvailableBytesCount() > 0)
    {
        std::string response = m_tcpClient->receiveBytes();
        
        // Parse the JSON response
        boost::property_tree::ptree pt;
        std::stringstream ss(response);
        boost::property_tree::read_json(ss, pt);

        // For now, just print the received JSON
        std::cout << "Received MCP message: " << response << std::endl;
    }
    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagMcp::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram is a function to place a Datagram here.
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagMcp::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    if (auto mcpDatagram = std::dynamic_pointer_cast<DatagramMcp>(datagram))
    {
        if (m_tcpClient->isConnected())
        {
            m_tcpClient->transmit(mcpDatagram->getJson());
        }
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagMcp::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
