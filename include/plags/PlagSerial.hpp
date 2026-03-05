/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagSerial.hpp
 * @author Gerrit Erichsen (saxomophon@gmx.de)
 * @contributors:
 * @brief Holds the PlagSerial class
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

#ifndef PLAGSERIAL_HPP
#define PLAGSERIAL_HPP

// std includes
#include <string>

// boost includes
#include <boost/asio.hpp>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagSerial class is a Plag to interact via Serial / UART interfaces
 *
 * @details Opens a serial port (e.g. /dev/ttyUSB0 on Linux, COM1 on Windows), reads incoming
 * data and sends outgoing string payloads from the datagram queue.
 */
class PlagSerial : public Plag
{
public:
    PlagSerial(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagSerial();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    // config parameters
    std::string m_portName;     //!< OS path to the serial port, e.g. /dev/ttyUSB0
    uint32_t m_baudRate;        //!< baud rate, e.g. 9600, 115200

    // worker members
    boost::asio::io_context m_ioContext;            //!< boost IO context
    boost::asio::serial_port m_serialPort;          //!< the serial port connection
};

#endif // PLAGSERIAL_HPP
