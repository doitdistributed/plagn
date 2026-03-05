/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagModbus.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagModbus class
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

#ifndef PLAGMODBUS_HPP
#define PLAGMODBUS_HPP

// std includes
#include <string>
#include <vector>

// libmodbus includes
#include <modbus.h>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagModbus class is a Plag to interact via Modbus TCP
 *
 * @details Acts as a Modbus master (client) over TCP. Connects to a Modbus slave (server),
 * polls a configured range of holding registers in loopWork(), and distributes the results as
 * DatagramMap key/value pairs. Incoming DatagramMap datagrams can write values to output
 * coils or registers on the slave.
 */
class PlagModbus : public Plag
{
public:
    PlagModbus(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagModbus();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    // config parameters
    std::string m_host;         //!< Modbus TCP server host
    uint16_t m_port;            //!< Modbus TCP server port (default: 502)
    int m_slaveId;              //!< Modbus slave/unit ID
    int m_startRegister;        //!< first holding register address to poll
    int m_registerCount;        //!< number of holding registers to poll

    // worker members
    modbus_t * m_modbusCtx;     //!< libmodbus context
};

#endif // PLAGMODBUS_HPP
