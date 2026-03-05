/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagLoraWan.hpp
 * @author Gerrit Erichsen (saxomophon@gmx.de)
 * @contributors:
 * @brief Holds the PlagLoraWan class
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

#ifndef PLAGLORAWAN_HPP
#define PLAGLORAWAN_HPP

// std includes
#include <string>
#include <thread>

// boost includes
#include <boost/asio.hpp>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagLoraWan class is a Plag to receive data from LoRaWAN networks via HTTP webhooks
 *
 * @details Acts as an HTTP webhook receiver compatible with The Things Network (TTN) and
 * Chirpstack. It listens on a configurable port for JSON POST requests and translates the
 * decoded LoRaWAN payload into a Datagram for distribution.
 */
class PlagLoraWan : public Plag
{
public:
    PlagLoraWan(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagLoraWan();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    void runWebhookServer();
    void handleWebhook(boost::asio::ip::tcp::socket socket);

private:
    // config parameters
    uint16_t m_port;            //!< port to listen for incoming LoRaWAN webhooks
    std::string m_secret;       //!< optional shared secret for basic validation

    // worker members
    boost::asio::io_context m_ioContext;        //!< io_context for the webhook HTTP server
    std::shared_ptr<std::thread> m_serverThread;//!< thread running the acceptor loop
};

#endif // PLAGLORAWAN_HPP
