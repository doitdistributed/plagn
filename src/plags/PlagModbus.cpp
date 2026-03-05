/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagModbus.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagModbus class
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
#include "DatagramMap.hpp"

// self include
#include "PlagModbus.hpp"

using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagModbus object and assigns default values
 * 
 */
PlagModbus::PlagModbus(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::Modbus),
    m_modbusCtx(nullptr)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagModbus object; closes and frees the libmodbus context
 * 
 */
PlagModbus::~PlagModbus()
{
    if (!m_stopToken) stopWork();
    if (m_modbusCtx)
    {
        modbus_close(m_modbusCtx);
        modbus_free(m_modbusCtx);
        m_modbusCtx = nullptr;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: host, port, slaveId, startRegister, registerCount
 * 
 */
void PlagModbus::readConfig() try
{
    m_host = getParameter<string>("host");
    m_port = getOptionalParameter<uint16_t>("port", MODBUS_TCP_DEFAULT_PORT);
    m_slaveId = getOptionalParameter<int>("slaveId", 1);
    m_startRegister = getOptionalParameter<int>("startRegister", 0);
    m_registerCount = getOptionalParameter<int>("registerCount", 10);
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagModbus::readConfig()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagModbus::init() creates the libmodbus TCP context and connects to the slave
 * 
 */
void PlagModbus::init() try
{
    m_modbusCtx = modbus_new_tcp(m_host.c_str(), m_port);
    if (!m_modbusCtx)
    {
        throw runtime_error("PlagModbus: failed to create Modbus TCP context for " + m_host);
    }
    modbus_set_slave(m_modbusCtx, m_slaveId);

    if (modbus_connect(m_modbusCtx) == -1)
    {
        string err = modbus_strerror(errno);
        modbus_free(m_modbusCtx);
        m_modbusCtx = nullptr;
        throw runtime_error("PlagModbus: connect failed: " + err);
    }

    cout << "PlagModbus: connected to " << m_host << ":" << m_port
         << " slave=" << m_slaveId << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagModbus::init()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagModbus::loopWork polls the configured holding registers and distributes a DatagramMap
 * 
 */
bool PlagModbus::loopWork() try
{
    bool somethingDone = false;

    if (!m_modbusCtx)
    {
        return false;
    }

    // poll holding registers
    vector<uint16_t> registers(static_cast<size_t>(m_registerCount));
    int rc = modbus_read_registers(m_modbusCtx, m_startRegister, m_registerCount, registers.data());
    if (rc == -1)
    {
        cerr << "PlagModbus: read_registers failed: " << modbus_strerror(errno) << endl;
        // attempt reconnect
        modbus_close(m_modbusCtx);
        modbus_connect(m_modbusCtx);
        return true;
    }

    // build a DatagramMap from the register values
    map<string, DataType> dataMap;
    for (int i = 0; i < m_registerCount; ++i)
    {
        string key = "reg_" + to_string(m_startRegister + i);
        dataMap[key] = static_cast<double>(registers[static_cast<size_t>(i)]);
    }
    shared_ptr<DatagramMap> datagram(new DatagramMap(getName(), dataMap));
    appendToDistribution(datagram);
    somethingDone = true;

    // send outgoing write requests from the datagram queue
    if (m_incommingDatagrams.begin() != m_incommingDatagrams.end())
    {
        shared_ptr<DatagramMap> castPtr = dynamic_pointer_cast<DatagramMap>(m_incommingDatagrams.front());
        m_incommingDatagrams.pop_front();
        if (castPtr != nullptr)
        {
            for (const auto & kv : castPtr->getMap())
            {
                // keys must be "reg_<address>"
                if (kv.first.substr(0, 4) == "reg_")
                {
                    int addr = stoi(kv.first.substr(4));
                    uint16_t val = static_cast<uint16_t>(get<double>(kv.second));
                    modbus_write_register(m_modbusCtx, addr, val);
                }
            }
            somethingDone = true;
        }
    }

    return somethingDone;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagModbus::loopWork()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramMap datagrams and queues them for Modbus write
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagModbus::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    const shared_ptr<DatagramMap> castPtr = dynamic_pointer_cast<DatagramMap>(datagram);
    if (castPtr != nullptr)
    {
        m_incommingDatagrams.push_back(datagram);
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagModbus::placeDatagram()";
    throw runtime_error(errorMsg);
}
