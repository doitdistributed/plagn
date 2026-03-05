/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagTcp.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagTcp class
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
#include "DatagramUdp.hpp"

// self include
#include "PlagTcp.hpp"

using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagTcp object and assigns default values
 * 
 */
PlagTcp::PlagTcp(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::TCP)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagTcp object; disconnects the TCP client
 * 
 */
PlagTcp::~PlagTcp()
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
        cerr << "Could not close PlagTcp, because of " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "Could not close PlagTcp, because for unknown reason!" << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration parameters: remoteIp and port
 * 
 */
void PlagTcp::readConfig() try
{
    m_remoteIp = getParameter<string>("remoteIp");
    m_port = getParameter<uint16_t>("port");
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagTcp::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagTcp::init() creates the TCP client and connects to the remote host
 * 
 */
void PlagTcp::init() try
{
    m_tcpClient = make_shared<TcpClient>(chrono::milliseconds(1000), *this, m_remoteIp, m_port);
    m_tcpClient->connect();
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagTcp::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagTcp::loopWork reads incoming data and sends queued outgoing datagrams
 * 
 */
bool PlagTcp::loopWork() try
{
    bool somethingDone = false;

    if (!m_tcpClient->isConnected())
    {
        // attempt reconnect
        m_tcpClient->connect();
        return true;
    }

    // receive available data and wrap into a DatagramUdp (generic raw payload container)
    if (m_tcpClient->getAvailableBytesCount() > 0)
    {
        string data = m_tcpClient->receiveBytes();
        shared_ptr<DatagramUdp> datagram(new DatagramUdp(getName(), m_remoteIp, m_port, data));
        appendToDistribution(datagram);
        somethingDone = true;
    }

    // send outgoing payloads
    if (m_incommingDatagrams.begin() != m_incommingDatagrams.end())
    {
        shared_ptr<DatagramUdp> castPtr = dynamic_pointer_cast<DatagramUdp>(m_incommingDatagrams.front());
        m_incommingDatagrams.pop_front();
        if (castPtr != nullptr && m_tcpClient->isConnected())
        {
            m_tcpClient->transmit(castPtr->getPayload());
            somethingDone = true;
        }
    }

    return somethingDone;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagTcp::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramUdp datagrams and queues them for TCP transmission
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagTcp::placeDatagram(const shared_ptr<Datagram> datagram) try
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
    errorMsg += "\nSomething happened in PlagTcp::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
