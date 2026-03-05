/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagAmqp.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagAmqp class
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

#ifndef PLAGAMQP_HPP
#define PLAGAMQP_HPP

// std includes
#include <memory>
#include <mutex>
#include <string>
#include <thread>

// boost includes
#include <boost/asio.hpp>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagAmqp class is a Plag to interact via AMQP 0-9-1 (e.g. RabbitMQ)
 *
 * @details Connects to an AMQP broker using amqpcpp over boost::asio. Declares a queue, binds
 * it to an exchange, and consumes messages. Incoming messages are distributed as DatagramMcp
 * (JSON body). Outgoing DatagramMcp datagrams are published to the configured exchange.
 */
class PlagAmqp : public Plag
{
public:
    PlagAmqp(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagAmqp();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    void runIoLoop();

private:
    // config parameters
    std::string m_brokerUrl;    //!< AMQP broker URL, e.g. amqp://guest:guest@localhost/
    std::string m_exchangeName; //!< exchange to publish to and bind the queue to
    std::string m_queueName;    //!< queue name to declare and consume from
    std::string m_routingKey;   //!< routing key for publish and binding

    // worker members (kept as void* / unique_ptr<void,Deleter> to avoid header pollution
    // from amqpcpp's AMQP namespace clashing with the PlagType::AMQP enum value)
    boost::asio::io_context m_ioContext;          //!< io_context for amqpcpp
    std::shared_ptr<void> m_handler;              //!< AMQP::LibBoostAsioHandler
    std::shared_ptr<void> m_connection;           //!< AMQP::TcpConnection
    std::shared_ptr<void> m_channel;              //!< AMQP::TcpChannel
    std::shared_ptr<std::thread> m_ioThread;      //!< thread running io_context
    std::mutex m_publishMutex;                    //!< guards publish calls
};

#endif // PLAGAMQP_HPP
