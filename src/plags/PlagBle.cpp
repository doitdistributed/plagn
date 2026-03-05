/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagBle.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagBle class
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
#include <stdexcept>

// own includes
#include "DatagramUdp.hpp"

// self include
#include "PlagBle.hpp"

using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagBle object and assigns default values
 * 
 */
PlagBle::PlagBle(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::BLE)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagBle object; disconnects from the BLE peripheral
 * 
 */
PlagBle::~PlagBle()
{
    if (!m_stopToken) stopWork();
    try
    {
        if (m_peripheral.has_value() && m_peripheral->is_connected())
        {
            m_peripheral->disconnect();
        }
    }
    catch (exception & e)
    {
        cerr << "PlagBle: could not disconnect: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: deviceName, serviceUuid, notifyCharUuid, writeCharUuid
 * 
 */
void PlagBle::readConfig() try
{
    m_deviceName = getParameter<string>("deviceName");
    m_serviceUuid = getParameter<string>("serviceUuid");
    m_notifyCharUuid = getOptionalParameter<string>("notifyCharUuid", string(""));
    m_writeCharUuid = getOptionalParameter<string>("writeCharUuid", string(""));
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagBle::readConfig()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagBle::init() scans for the configured peripheral, connects, and subscribes to notifications
 * 
 */
void PlagBle::init() try
{
    if (!SimpleBLE::Adapter::bluetooth_enabled())
    {
        throw runtime_error("PlagBle: Bluetooth is not enabled on this system");
    }

    auto adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.empty())
    {
        throw runtime_error("PlagBle: no Bluetooth adapters found");
    }

    SimpleBLE::Adapter & adapter = adapters[0];

    // scan until we find our target device
    optional<SimpleBLE::Peripheral> found;
    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral)
    {
        if (peripheral.identifier() == m_deviceName)
        {
            found = peripheral;
            adapter.scan_stop();
        }
    });

    adapter.scan_start();
    // wait up to 10 seconds for the device to appear
    for (int i = 0; i < 100 && !found.has_value(); ++i)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    adapter.scan_stop();

    if (!found.has_value())
    {
        throw runtime_error("PlagBle: device '" + m_deviceName + "' not found during scan");
    }

    m_peripheral = found;
    m_peripheral->connect();

    // subscribe to GATT notifications if a notify characteristic was configured
    if (!m_notifyCharUuid.empty())
    {
        m_peripheral->notify(m_serviceUuid, m_notifyCharUuid,
            [this](SimpleBLE::ByteArray bytes)
            {
                string payload(bytes.begin(), bytes.end());
                shared_ptr<DatagramUdp> datagram(
                    new DatagramUdp(getName(), m_deviceName, 0, payload));
                appendToDistribution(datagram);
            });
    }

    cout << "PlagBle: connected to '" << m_deviceName << "'" << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagBle::init()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagBle::loopWork sends queued outgoing writes; notifications arrive via the callback
 * 
 */
bool PlagBle::loopWork() try
{
    // notifications arrive via async callback — send queued write datagrams
    if (m_incommingDatagrams.begin() == m_incommingDatagrams.end())
    {
        return false;
    }

    shared_ptr<DatagramUdp> castPtr = dynamic_pointer_cast<DatagramUdp>(m_incommingDatagrams.front());
    m_incommingDatagrams.pop_front();

    if (castPtr != nullptr && m_peripheral.has_value() && m_peripheral->is_connected()
        && !m_writeCharUuid.empty())
    {
        string payload = castPtr->getPayload();
        SimpleBLE::ByteArray bytes(payload.begin(), payload.end());
        m_peripheral->write_command(m_serviceUuid, m_writeCharUuid, bytes);
        return true;
    }
    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagBle::loopWork()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramUdp datagrams and queues them for BLE write
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagBle::placeDatagram(const shared_ptr<Datagram> datagram) try
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
    errorMsg += "\nSomething happened in PlagBle::placeDatagram()";
    throw runtime_error(errorMsg);
}
