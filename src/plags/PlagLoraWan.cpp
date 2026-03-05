/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagLoraWan.cpp
 * @author Gerrit Erichsen (saxomophon@gmx.de)
 * @contributors:
 * @brief Implements the PlagLoraWan class
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
#include "DatagramMcp.hpp"

// self include
#include "PlagLoraWan.hpp"

using namespace std;
using boost::asio::ip::tcp;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagLoraWan object and assigns default values
 * 
 */
PlagLoraWan::PlagLoraWan(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::LoRaWAN)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagLoraWan object; stops the webhook server thread
 * 
 */
PlagLoraWan::~PlagLoraWan()
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
 * @brief reads configuration: port and optional secret header value for validation
 * 
 */
void PlagLoraWan::readConfig() try
{
    m_port = getOptionalParameter<uint16_t>("port", 8880);
    m_secret = getOptionalParameter<string>("secret", string(""));
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagLoraWan::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagLoraWan::init() starts the HTTP webhook server on a background thread
 * 
 */
void PlagLoraWan::init() try
{
    m_serverThread = make_shared<thread>(&PlagLoraWan::runWebhookServer, this);
    cout << "PlagLoraWan: webhook listener started on port " << m_port << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagLoraWan::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief runWebhookServer is the main loop for the background HTTP server thread
 * 
 */
void PlagLoraWan::runWebhookServer()
{
    try
    {
        tcp::acceptor acceptor(m_ioContext, tcp::endpoint(tcp::v4(), m_port));
        while (!m_ioContext.stopped())
        {
            tcp::socket socket(m_ioContext);
            acceptor.accept(socket);
            handleWebhook(std::move(socket));
        }
    }
    catch (exception & e)
    {
        cerr << "PlagLoraWan webhook server error: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief handleWebhook reads an HTTP POST body and wraps it in a DatagramMcp for distribution
 *
 * @details The full LoRaWAN JSON payload from TTN/Chirpstack is preserved as-is in the
 * DatagramMcp json field. Downstream Kables can further parse it.
 *
 * @param socket the accepted client socket
 */
void PlagLoraWan::handleWebhook(tcp::socket socket)
{
    try
    {
        boost::asio::streambuf requestBuf;
        boost::asio::read_until(socket, requestBuf, "\r\n\r\n");
        istream requestStream(&requestBuf);

        // read headers to find Content-Length
        string line;
        size_t contentLength = 0;
        while (getline(requestStream, line) && line != "\r")
        {
            if (line.find("Content-Length:") != string::npos ||
                line.find("content-length:") != string::npos)
            {
                size_t colonPos = line.find(':');
                contentLength = stoul(line.substr(colonPos + 1));
            }
        }

        // read the JSON body
        string body(contentLength, '\0');
        if (contentLength > 0)
        {
            boost::asio::read(socket, boost::asio::buffer(&body[0], contentLength));
        }

        // ack with HTTP 200 OK
        string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        boost::asio::write(socket, boost::asio::buffer(response));

        // wrap the full LoRaWAN JSON into a DatagramMcp and distribute
        shared_ptr<DatagramMcp> datagram(new DatagramMcp(getName(), body));
        appendToDistribution(datagram);
    }
    catch (exception & e)
    {
        cerr << "PlagLoraWan: error handling webhook: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagLoraWan::loopWork has no primary-thread work; the webhook server runs on its own thread
 * 
 */
bool PlagLoraWan::loopWork() try
{
    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagLoraWan::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram has no effect for this source-only Plag
 *
 * @param datagram A Datagram (ignored, as this Plag only produces data)
 */
void PlagLoraWan::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    // PlagLoraWan is a source-only Plag; it does not consume incoming datagrams
    (void)datagram;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagLoraWan::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
