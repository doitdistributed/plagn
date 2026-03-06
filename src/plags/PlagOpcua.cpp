/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagOpcua.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagOpcua class
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

// open62541pp includes
#include <open62541pp/open62541pp.hpp>


// own includes
#include "DatagramMap.hpp"

// self include
#include "PlagOpcua.hpp"

using namespace std;

// helper: parse "ns=N;i=M" or "ns=N;s=MyName" into an opcua::NodeId
static opcua::NodeId parseNodeId(const string & nodeIdStr)
{
    uint16_t ns = 0;
    size_t nsPos = nodeIdStr.find("ns=");
    if (nsPos != string::npos)
    {
        size_t end = nodeIdStr.find(';', nsPos);
        ns = static_cast<uint16_t>(stoul(nodeIdStr.substr(nsPos + 3, end - (nsPos + 3))));
    }
    size_t iPos = nodeIdStr.find(";i=");
    if (iPos != string::npos)
    {
        return opcua::NodeId(ns, static_cast<uint32_t>(stoul(nodeIdStr.substr(iPos + 3))));
    }
    size_t sPos = nodeIdStr.find(";s=");
    if (sPos != string::npos)
    {
        return opcua::NodeId(ns, string_view(nodeIdStr).substr(sPos + 3));
    }
    throw invalid_argument("PlagOpcua: cannot parse NodeId: " + nodeIdStr);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagOpcua object and assigns default values
 * 
 */
PlagOpcua::PlagOpcua(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::OPCUA)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagOpcua object; disconnects the OPC UA client
 * 
 */
PlagOpcua::~PlagOpcua()
{
    if (!m_stopToken) stopWork();
    try
    {
        if (m_client)
        {
            m_client->disconnect();
        }
    }
    catch (exception & e)
    {
        cerr << "PlagOpcua: could not disconnect: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: serverUrl and comma-separated nodeIds
 * 
 */
void PlagOpcua::readConfig() try
{
    m_serverUrl = getParameter<string>("serverUrl");

    string nodeIdsStr = getOptionalParameter<string>("nodeIds", string(""));
    if (!nodeIdsStr.empty())
    {
        stringstream ss(nodeIdsStr);
        string token;
        while (getline(ss, token, ','))
        {
            m_nodeIds.push_back(token);
        }
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagOpcua::readConfig()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagOpcua::init() creates the open62541pp client and connects to the server
 * 
 */
void PlagOpcua::init() try
{
    m_client = make_unique<opcua::Client>();
    m_client->connect(m_serverUrl);
    cout << "PlagOpcua: connected to " << m_serverUrl << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagOpcua::init()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagOpcua::loopWork reads all configured node values and distributes a DatagramMap
 * 
 */
bool PlagOpcua::loopWork() try
{
    if (!m_client || m_nodeIds.empty())
    {
        return false;
    }

    map<string, DataType> dataMap;
    for (const string & nodeIdStr : m_nodeIds)
    {
        try
        {
            opcua::NodeId nodeId = parseNodeId(nodeIdStr);
            opcua::Result<opcua::Variant> result = opcua::services::readValue(*m_client, nodeId);
            if (result.hasValue())
            {
                const opcua::Variant & v = result.value();
                if (v.isScalar())
                {
                    // Try to cast to double; fall back to float
                    try {
                        dataMap[nodeIdStr] = v.scalar<double>();
                    }
                    catch (...) {
                        try {
                            dataMap[nodeIdStr] = static_cast<double>(v.scalar<float>());
                        }
                        catch (...) {}
                    }
                }
            }
        }
        catch (exception & e)
        {
            cerr << "PlagOpcua: read node " << nodeIdStr << " failed: " << e.what() << endl;
        }
    }

    if (!dataMap.empty())
    {
        appendToDistribution(make_shared<DatagramMap>(getName(), dataMap));
        return true;
    }

    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagOpcua::loopWork()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramMap datagrams and writes values to OPC UA nodes
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagOpcua::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    const shared_ptr<DatagramMap> castPtr = dynamic_pointer_cast<DatagramMap>(datagram);
    if (castPtr != nullptr && m_client)
    {
        for (const auto & kv : castPtr->getMap())
        {
            try
            {
                opcua::NodeId nodeId = parseNodeId(kv.first);
                opcua::Variant value;
                if (holds_alternative<double>(kv.second))
                {
                    value.assign(get<double>(kv.second));
                }
                else if (holds_alternative<string>(kv.second))
                {
                    value.assign(get<string>(kv.second));
                }
                opcua::services::writeValue(*m_client, nodeId, value);
            }
            catch (exception & e)
            {
                cerr << "PlagOpcua: write node " << kv.first << " failed: " << e.what() << endl;
            }
        }
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagOpcua::placeDatagram()";
    throw runtime_error(errorMsg);
}
