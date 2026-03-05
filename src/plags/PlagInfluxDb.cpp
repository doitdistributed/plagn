/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagInfluxDb.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagInfluxDb class
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
#include <string>

// boost includes
#include <boost/asio.hpp>

// own includes
#include "DatagramUdp.hpp"

// self include
#include "PlagInfluxDb.hpp"

using namespace std;
using boost::asio::ip::tcp;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagInfluxDb object and assigns default values
 * 
 */
PlagInfluxDb::PlagInfluxDb(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::InfluxDb)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagInfluxDb object
 * 
 */
PlagInfluxDb::~PlagInfluxDb()
{
    if (!m_stopToken) stopWork();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: host, port, database/bucket, measurement, optional v2 token/org
 * 
 */
void PlagInfluxDb::readConfig() try
{
    m_host = getOptionalParameter<string>("host", "localhost");
    m_port = getOptionalParameter<uint16_t>("port", 8086);
    m_database = getParameter<string>("database");
    m_measurement = getOptionalParameter<string>("measurement", "plagn");
    m_token = getOptionalParameter<string>("token", string(""));
    m_org = getOptionalParameter<string>("org", string(""));
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagInfluxDb::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagInfluxDb::init() verifies connectivity to the InfluxDB endpoint
 * 
 */
void PlagInfluxDb::init() try
{
    // attempt a test connection to make sure InfluxDB is reachable at startup
    boost::asio::io_context ioContext;
    tcp::resolver resolver(ioContext);
    tcp::resolver::results_type endpoints = resolver.resolve(m_host, to_string(m_port));
    tcp::socket socket(ioContext);
    boost::asio::connect(socket, endpoints);
    socket.close();
    cout << "PlagInfluxDb: reached InfluxDB at " << m_host << ":" << m_port << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagInfluxDb::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagInfluxDb::loopWork sends any queued datagrams to InfluxDB as line protocol
 * 
 */
bool PlagInfluxDb::loopWork() try
{
    if (m_incommingDatagrams.begin() == m_incommingDatagrams.end())
    {
        return false;
    }

    shared_ptr<Datagram> datagram = m_incommingDatagrams.front();
    m_incommingDatagrams.pop_front();

    // Build InfluxDB line protocol: measurement,tag_key=tag_value field_key=field_value timestamp
    // We use the payload from a DatagramUdp as a pre-formed line protocol string if available,
    // otherwise we use the datagram's toString() and let the user pre-format it via a Kable.
    string lineData;
    shared_ptr<DatagramUdp> udpPtr = dynamic_pointer_cast<DatagramUdp>(datagram);
    if (udpPtr != nullptr)
    {
        lineData = udpPtr->getPayload();
    }
    else
    {
        lineData = m_measurement + " value=" + datagram->toString();
    }

    return postLineProtocol(lineData);
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagInfluxDb::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief postLineProtocol sends a single line of InfluxDB line protocol via HTTP POST
 *
 * @param lineData the fully-formed InfluxDB line protocol string
 * @return true if the server returned HTTP 204
 * @return false on failure
 */
bool PlagInfluxDb::postLineProtocol(const string & lineData) try
{
    boost::asio::io_context ioContext;
    tcp::resolver resolver(ioContext);
    tcp::resolver::results_type endpoints = resolver.resolve(m_host, to_string(m_port));
    tcp::socket socket(ioContext);
    boost::asio::connect(socket, endpoints);

    // build the HTTP POST request
    string path;
    string authHeader;
    if (!m_token.empty())
    {
        // InfluxDB v2
        path = "/api/v2/write?org=" + m_org + "&bucket=" + m_database + "&precision=ns";
        authHeader = "Authorization: Token " + m_token + "\r\n";
    }
    else
    {
        // InfluxDB v1
        path = "/write?db=" + m_database;
        authHeader = "";
    }

    string request = "POST " + path + " HTTP/1.1\r\n"
                     "Host: " + m_host + "\r\n"
                     + authHeader +
                     "Content-Type: application/octet-stream\r\n"
                     "Content-Length: " + to_string(lineData.size()) + "\r\n"
                     "Connection: close\r\n"
                     "\r\n"
                     + lineData;

    boost::asio::write(socket, boost::asio::buffer(request));

    // read the status line
    boost::asio::streambuf responseBuf;
    boost::asio::read_until(socket, responseBuf, "\r\n");
    istream responseStream(&responseBuf);
    string httpVersion, statusMessage;
    unsigned int statusCode;
    responseStream >> httpVersion >> statusCode;
    getline(responseStream, statusMessage);

    return (statusCode == 204 || statusCode == 200);
}
catch (exception & e)
{
    cerr << "PlagInfluxDb: failed to post - " << e.what() << endl;
    return false;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts any Datagram and queues it for writing to InfluxDB
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagInfluxDb::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    m_incommingDatagrams.push_back(datagram);
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagInfluxDb::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
