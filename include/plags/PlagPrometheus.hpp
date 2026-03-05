/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagPrometheus.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagPrometheus class
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

#ifndef PLAGPROMETHEUS_HPP
#define PLAGPROMETHEUS_HPP

// std includes
#include <map>
#include <mutex>
#include <string>
#include <thread>

// boost includes
#include <boost/asio.hpp>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagPrometheus class is a Plag that exposes a /metrics HTTP endpoint for Prometheus
 *
 * @details Keeps an in-memory store of metric name/value pairs. When a datagram arrives with
 * key-value data, it updates the store. A lightweight HTTP server runs on a background thread
 * serving GET /metrics in the standard Prometheus text exposition format.
 */
class PlagPrometheus : public Plag
{
public:
    PlagPrometheus(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagPrometheus();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    void serveMetrics();
    std::string buildMetricsPayload() const;
    void handleRequest(boost::asio::ip::tcp::socket socket);

private:
    // config parameters
    uint16_t m_port;            //!< port to expose /metrics on (default: 9090)
    std::string m_metricPrefix; //!< optional prefix for all metric names

    // worker members
    std::map<std::string, double> m_metrics;    //!< current metric values
    mutable std::mutex m_metricsMutex;          //!< guards m_metrics for thread-safe access
    boost::asio::io_context m_ioContext;        //!< io_context for the metrics HTTP server
    std::shared_ptr<std::thread> m_serverThread;//!< thread running the acceptor loop
};

#endif // PLAGPROMETHEUS_HPP
