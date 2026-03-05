/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagBle.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagBle class
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

#ifndef PLAGBLE_HPP
#define PLAGBLE_HPP

// std includes
#include <string>
#include <optional>

// simpleble includes
#include <simpleble/SimpleBLE.h>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagBle class is a Plag to interact via Bluetooth Low Energy (BLE)
 *
 * @details Scans for a BLE peripheral by configured name, connects to it, and subscribes to
 * a GATT notification characteristic. Incoming notifications are distributed as DatagramUdp
 * payloads. placeDatagram() writes data to a configured write characteristic.
 */
class PlagBle : public Plag
{
public:
    PlagBle(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagBle();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    // config parameters
    std::string m_deviceName;       //!< BLE peripheral name to connect to
    std::string m_serviceUuid;      //!< GATT service UUID
    std::string m_notifyCharUuid;   //!< GATT characteristic UUID for notifications
    std::string m_writeCharUuid;    //!< GATT characteristic UUID for writes

    // worker members
    std::optional<SimpleBLE::Peripheral> m_peripheral; //!< the connected BLE peripheral
};

#endif // PLAGBLE_HPP
