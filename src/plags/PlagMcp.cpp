/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagMcp.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
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
#include <sstream>

// boost includes
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// own includes
#include "DatagramMcp.hpp"
#include "TcpClient.hpp"

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
 * @brief Destroy the PlagMcp object; disconnects the TCP client
 * 
 */
PlagMcp::~PlagMcp()
{
    if (!m_stopToken) stopWork();
    try
    {
        if (m_tcpClient && m_tcpClient->isConnected())
        {
            m_tcpClient->disconnect();
        }
    }
    catch (exception & e)
    {
        cerr << "Could not close PlagMcp, because of " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "Could not close PlagMcp, because for unknown reason!" << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration parameters: serverIp and port
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
 * @brief PlagMcp::init() creates the TCP client and connects to the MCP server
 * 
 */
void PlagMcp::init() try
{
    m_tcpClient = make_shared<TcpClient>(chrono::milliseconds(1000), *this, m_serverIp, m_port);
    m_tcpClient->connect();
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagMcp::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagMcp::loopWork reads incoming JSON-RPC messages and dispatches outgoing ones
 * 
 */
bool PlagMcp::loopWork() try
{
    bool somethingDone = false;

    if (!m_tcpClient->isConnected())
    {
        m_tcpClient->connect();
        return true;
    }

    // receive and dispatch incoming MCP messages
    if (m_tcpClient->getAvailableBytesCount() > 0)
    {
        string response = m_tcpClient->receiveBytes();
        boost::property_tree::ptree pt;
        stringstream ss(response);
        boost::property_tree::read_json(ss, pt);

        shared_ptr<DatagramMcp> datagram(new DatagramMcp(getName(), response));
        appendToDistribution(datagram);
        somethingDone = true;
    }

    // send outgoing MCP messages
    if (m_incommingDatagrams.begin() != m_incommingDatagrams.end())
    {
        shared_ptr<DatagramMcp> castPtr = dynamic_pointer_cast<DatagramMcp>(m_incommingDatagrams.front());
        m_incommingDatagrams.pop_front();
        if (castPtr != nullptr && m_tcpClient->isConnected())
        {
            m_tcpClient->transmit(castPtr->getJson());
            somethingDone = true;
        }
    }

    return somethingDone;
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
 * @brief placeDatagram accepts only DatagramMcp datagrams and queues them for transmission
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagMcp::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    const shared_ptr<DatagramMcp> castPtr = dynamic_pointer_cast<DatagramMcp>(datagram);
    if (castPtr != nullptr)
    {
        m_incommingDatagrams.push_back(datagram);
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagMcp::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
