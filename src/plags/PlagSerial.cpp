/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagSerial.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagSerial class
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
#ifndef _WIN32
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif
// own includes
#include "DatagramUdp.hpp"

// self include
#include "PlagSerial.hpp"

using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagSerial object and assigns default values
 * 
 */
PlagSerial::PlagSerial(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::Serial),
    m_serialPort(m_ioContext)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagSerial object, closes the serial port
 * 
 */
PlagSerial::~PlagSerial()
{
    if (!m_stopToken) stopWork();
    try
    {
        if (m_serialPort.is_open())
        {
            m_serialPort.close();
        }
    }
    catch (exception & e)
    {
        cerr << "Could not close PlagSerial, because of " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "Could not close PlagSerial, because for unknown reason!" << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: portName (e.g. /dev/ttyUSB0) and baudRate
 * 
 */
void PlagSerial::readConfig() try
{
    m_portName = getParameter<string>("portName");
    m_baudRate = getOptionalParameter<uint32_t>("baudRate", 9600);
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagSerial::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagSerial::init() opens and configures the serial port
 * 
 */
void PlagSerial::init() try
{
    m_serialPort.open(m_portName);
    m_serialPort.set_option(boost::asio::serial_port_base::baud_rate(m_baudRate));
    m_serialPort.set_option(boost::asio::serial_port_base::character_size(8));
    m_serialPort.set_option(boost::asio::serial_port_base::stop_bits(
        boost::asio::serial_port_base::stop_bits::one));
    m_serialPort.set_option(boost::asio::serial_port_base::parity(
        boost::asio::serial_port_base::parity::none));
    m_serialPort.set_option(boost::asio::serial_port_base::flow_control(
        boost::asio::serial_port_base::flow_control::none));
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagSerial::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagSerial::loopWork reads available serial data and sends queued outgoing payloads
 * 
 */
bool PlagSerial::loopWork() try
{
    bool somethingDone = false;

    // check for available bytes on the serial port via ioctl
    boost::asio::serial_port_base::baud_rate opt;
    boost::system::error_code ec;
    int bytesAvailable = 0;
#ifndef _WIN32
    ::ioctl(m_serialPort.native_handle(), FIONREAD, &bytesAvailable);
#else
    COMSTAT comStat;
    DWORD errors;
    ClearCommError(m_serialPort.native_handle(), &errors, &comStat);
    bytesAvailable = comStat.cbInQue;
#endif

    if (bytesAvailable > 0)
    {
        char recvBuff[1024] = { 0 };
        size_t toRead = static_cast<size_t>(bytesAvailable) < sizeof(recvBuff)
                        ? static_cast<size_t>(bytesAvailable)
                        : sizeof(recvBuff) - 1;
        size_t length = boost::asio::read(m_serialPort, boost::asio::buffer(recvBuff, toRead));
        if (length > 0)
        {
            string data = string(recvBuff, length);
            // re-use DatagramUdp as a generic raw payload datagram
            shared_ptr<DatagramUdp> datagram(new DatagramUdp(getName(), m_portName, 0, data));
            appendToDistribution(datagram);
            somethingDone = true;
        }
    }

    // send outgoing payloads
    if (m_incommingDatagrams.begin() != m_incommingDatagrams.end())
    {
        shared_ptr<DatagramUdp> castPtr = dynamic_pointer_cast<DatagramUdp>(m_incommingDatagrams.front());
        m_incommingDatagrams.pop_front();
        if (castPtr != nullptr && m_serialPort.is_open())
        {
            string payload = castPtr->getPayload();
            boost::asio::write(m_serialPort, boost::asio::buffer(payload, payload.size()));
            somethingDone = true;
        }
    }

    return somethingDone;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagSerial::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramUdp datagrams and queues them for serial transmission
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagSerial::placeDatagram(const shared_ptr<Datagram> datagram) try
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
    errorMsg += "\nSomething happened in PlagSerial::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
