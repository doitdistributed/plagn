/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagAmqp.cpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Implements the PlagAmqp class
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

// amqpcpp includes — must come before PlagInterface.hpp because amqpcpp defines
// namespace AMQP which conflicts with the enum value PlagType::AMQPBroker.
// Including only in the .cpp keeps the collision out of the header.
#include <amqpcpp.h>

// own includes
#include "DatagramMcp.hpp"

// self include
#include "PlagAmqp.hpp"

using namespace std;

// Custom boost::asio connection handler because vcpkg disables linux_tcp (TcpConnection) on macOS
class AsioConnectionHandler : public ::AMQP::ConnectionHandler
{
private:
    boost::asio::io_context & m_ioc;
    boost::asio::ip::tcp::socket m_socket;
    ::AMQP::Connection * m_connection = nullptr;
    char m_buffer[8192];

public:
    AsioConnectionHandler(boost::asio::io_context & ioc) 
        : m_ioc(ioc), m_socket(ioc) {}
        
    void setConnection(::AMQP::Connection * conn) { m_connection = conn; }
    
    boost::asio::ip::tcp::socket & socket() { return m_socket; }
    
    void doRead()
    {
        m_socket.async_read_some(boost::asio::buffer(m_buffer),
            [this](boost::system::error_code ec, size_t len)
            {
                if (!ec && m_connection)
                {
                    m_connection->parse(m_buffer, len);
                    doRead();
                }
            });
    }

    virtual void onData(::AMQP::Connection * connection, const char * data, size_t size) override
    {
        boost::system::error_code ec;
        boost::asio::write(m_socket, boost::asio::buffer(data, size), ec);
        if (ec)
        {
            cerr << "PlagAmqp: Write error: " << ec.message() << endl;
        }
    }

    virtual void onError(::AMQP::Connection * connection, const char * message) override
    {
        cerr << "PlagAmqp: connection error: " << message << endl;
        boost::system::error_code ec;
        m_socket.close(ec);
    }

    virtual void onClosed(::AMQP::Connection * connection) override
    {
        boost::system::error_code ec;
        m_socket.close(ec);
    }
};

namespace
{
    // Typed accessor helpers to cast the opaque void* members back to their real types
    AsioConnectionHandler * asHandler(shared_ptr<void> & p)
    {
        return static_cast<AsioConnectionHandler *>(p.get());
    }
    ::AMQP::Connection * asConnection(shared_ptr<void> & p)
    {
        return static_cast<::AMQP::Connection *>(p.get());
    }
    ::AMQP::Channel * asChannel(shared_ptr<void> & p)
    {
        return static_cast<::AMQP::Channel *>(p.get());
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagAmqp object and assigns default values
 * 
 */
PlagAmqp::PlagAmqp(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::AMQPBroker)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagAmqp object; stops the io_context thread
 * 
 */
PlagAmqp::~PlagAmqp()
{
    if (!m_stopToken) stopWork();
    m_ioContext.stop();
    if (m_ioThread && m_ioThread->joinable())
    {
        m_ioThread->join();
    }
    // destroy channel and connection before handler
    m_channel.reset();
    m_connection.reset();
    m_handler.reset();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads configuration: brokerUrl, exchangeName, queueName, routingKey
 * 
 */
void PlagAmqp::readConfig() try
{
    m_brokerUrl = getOptionalParameter<string>("brokerUrl", string("amqp://guest:guest@localhost/"));
    m_exchangeName = getOptionalParameter<string>("exchangeName", string(""));
    m_queueName = getParameter<string>("queueName");
    m_routingKey = getOptionalParameter<string>("routingKey", m_queueName);
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagAmqp::readConfig()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagAmqp::init() creates the AMQP connection, channel, declares queue, and starts consumer
 * 
 */
void PlagAmqp::init() try
{
    ::AMQP::Address address(m_brokerUrl);

    // create typed handler and connect sync
    auto * handler = new AsioConnectionHandler(m_ioContext);
    m_handler = shared_ptr<void>(handler, [](void * p) { delete static_cast<AsioConnectionHandler *>(p); });

    boost::asio::ip::tcp::resolver resolver(m_ioContext);
    uint16_t port = address.port() == 0 ? 5672 : address.port();
    auto results = resolver.resolve(address.hostname(), std::to_string(port));
    boost::asio::connect(handler->socket(), results.begin(), results.end());

    // create connection and channel
    auto * connection = new ::AMQP::Connection(handler, address.login(), address.vhost());
    handler->setConnection(connection);
    m_connection = shared_ptr<void>(connection, [](void * p) { delete static_cast<::AMQP::Connection *>(p); });
    
    m_channel = shared_ptr<void>(new ::AMQP::Channel(connection),
                                 [](void * p) { delete static_cast<::AMQP::Channel *>(p); });

    string queueName = m_queueName;
    string exchangeName = m_exchangeName;
    string routingKey = m_routingKey;
    string plagName = getName();

    // declare queue and optionally bind to exchange
    asChannel(m_channel)->declareQueue(queueName)
        .onSuccess([this, exchangeName, queueName, routingKey](
                    const string & name, uint32_t /*msgCount*/, uint32_t /*consumerCount*/)
        {
            if (!exchangeName.empty())
            {
                asChannel(m_channel)->bindQueue(exchangeName, name, routingKey);
            }
        });

    // consume — messages arrive via callback, distributed as DatagramMcp
    asChannel(m_channel)->consume(queueName)
        .onReceived([this, plagName](const ::AMQP::Message & message,
                                     uint64_t deliveryTag, bool /*redelivered*/)
        {
            string body(message.body(), message.bodySize());
            shared_ptr<DatagramMcp> datagram(new DatagramMcp(plagName, body));
            appendToDistribution(datagram);
            asChannel(m_channel)->ack(deliveryTag);
        });

    // start the async read loop
    handler->doRead();

    // run boost::asio io_context on a background thread
    m_ioThread = make_shared<thread>(&PlagAmqp::runIoLoop, this);

    cout << "PlagAmqp: connected to " << m_brokerUrl
         << " queue='" << m_queueName << "'" << endl;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagAmqp::init()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief runIoLoop runs the boost::asio io_context — keeps AMQP connection alive
 * 
 */
void PlagAmqp::runIoLoop()
{
    try
    {
        m_ioContext.run();
    }
    catch (exception & e)
    {
        cerr << "PlagAmqp io_context error: " << e.what() << endl;
    }
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagAmqp::loopWork publishes any queued datagrams to the AMQP exchange
 * 
 */
bool PlagAmqp::loopWork() try
{
    if (m_incommingDatagrams.begin() == m_incommingDatagrams.end())
    {
        return false;
    }

    shared_ptr<DatagramMcp> castPtr = dynamic_pointer_cast<DatagramMcp>(m_incommingDatagrams.front());
    m_incommingDatagrams.pop_front();

    if (castPtr != nullptr && asChannel(m_channel))
    {
        lock_guard<mutex> lock(m_publishMutex);
        const string & json = castPtr->getJson();
        ::AMQP::Envelope envelope(json.c_str(), json.size());
        envelope.setContentType("application/json");
        asChannel(m_channel)->publish(m_exchangeName, m_routingKey, envelope);
        return true;
    }
    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagAmqp::loopWork()";
    throw runtime_error(errorMsg);
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram accepts DatagramMcp datagrams and queues them for AMQP publish
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagAmqp::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    const shared_ptr<DatagramMcp> castPtr = dynamic_pointer_cast<DatagramMcp>(datagram);
    if (castPtr != nullptr)
    {
        m_incommingDatagrams.push_back(datagram);
    }
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagAmqp::placeDatagram()";
    throw runtime_error(errorMsg);
}
