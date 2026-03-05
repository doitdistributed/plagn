/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagPrometheus.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagPrometheus class
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
#include "PlagPrometheus.hpp"

using namespace std;
using boost::asio::ip::tcp;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagPrometheus object and assigns default values
 * 
 */
PlagPrometheus::PlagPrometheus(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::Prometheus)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagPrometheus object, stops the HTTP metrics server and thread
 * 
 */
PlagPrometheus::~PlagPrometheus()
{
    if (!m_stopToken) stopWork();
    m_ioContext.stop();
    if (m_serverThread && m_serverThread->joinable())
    {
        m_serverThread->join();
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: port and optional metric prefix
 * 
 */
void PlagPrometheus::readConfig() try
{
    m_port = getOptionalParameter<uint16_t>("port", 9090);
    m_metricPrefix = getOptionalParameter<string>("metricPrefix", string("plagn_"));
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagPrometheus::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagPrometheus::init() starts the background HTTP metrics server thread
 * 
 */
void PlagPrometheus::init() try
{
    m_serverThread = make_shared<thread>(&PlagPrometheus::serveMetrics, this);
    cout << "PlagPrometheus: /metrics available on port " << m_port << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagPrometheus::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief serveMetrics runs the HTTP acceptor loop in the background thread
 * 
 */
void PlagPrometheus::serveMetrics()
{
    try
    {
        tcp::acceptor acceptor(m_ioContext,
                               tcp::endpoint(tcp::v4(), m_port));
        while (!m_ioContext.stopped())
        {
            tcp::socket socket(m_ioContext);
            acceptor.accept(socket);
            handleRequest(std::move(socket));
        }
    }
    catch (exception & e)
    {
        cerr << "PlagPrometheus metrics server error: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief handleRequest serves one HTTP GET request by returning all metrics
 *
 * @param socket the accepted client socket
 */
void PlagPrometheus::handleRequest(tcp::socket socket)
{
    try
    {
        boost::asio::streambuf requestBuf;
        boost::asio::read_until(socket, requestBuf, "\r\n\r\n");

        string payload = buildMetricsPayload();
        string response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain; version=0.0.4\r\n"
                          "Content-Length: " + to_string(payload.size()) + "\r\n"
                          "Connection: close\r\n"
                          "\r\n" + payload;

        boost::asio::write(socket, boost::asio::buffer(response));
    }
    catch (exception & e)
    {
        cerr << "PlagPrometheus: error handling request: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief buildMetricsPayload formats all stored metrics in Prometheus text exposition format
 *
 * @return std::string the complete /metrics text payload
 */
string PlagPrometheus::buildMetricsPayload() const
{
    lock_guard<mutex> lock(m_metricsMutex);
    ostringstream oss;
    for (const auto & kv : m_metrics)
    {
        oss << "# HELP " << kv.first << " Metric exported by plagn\n";
        oss << "# TYPE " << kv.first << " gauge\n";
        oss << kv.first << " " << kv.second << "\n";
    }
    return oss.str();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagPrometheus::loopWork has nothing to do externally — the metrics server runs on its
 * own thread. This function handles potential incoming placeholder work.
 * 
 */
bool PlagPrometheus::loopWork() try
{
    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagPrometheus::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram parses key/value pairs from incoming datagrams and updates the metrics store
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagPrometheus::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    // extract numeric float values via the standard Datagram::getData interface
    // keys are expected to be provided by the Kable's translation and available
    // as doubles in the DataType variant. Non-numeric keys are silently skipped.
    // For raw payloads (DatagramUdp), we attempt to parse "key=value" pairs.
    shared_ptr<DatagramUdp> udpPtr = dynamic_pointer_cast<DatagramUdp>(datagram);
    if (udpPtr != nullptr)
    {
        istringstream stream(udpPtr->getPayload());
        string pair;
        lock_guard<mutex> lock(m_metricsMutex);
        while (getline(stream, pair, ','))
        {
            size_t sep = pair.find('=');
            if (sep != string::npos)
            {
                string key = m_metricPrefix + pair.substr(0, sep);
                double val = stod(pair.substr(sep + 1));
                m_metrics[key] = val;
            }
        }
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagPrometheus::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
