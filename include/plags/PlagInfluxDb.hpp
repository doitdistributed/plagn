/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagInfluxDb.hpp
 * @author Bjoern Boettcher (doitdistributed@parallel-ing.net)
 * @contributors:
 * @brief Holds the PlagInfluxDb class
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

#ifndef PLAGINFLUXDB_HPP
#define PLAGINFLUXDB_HPP

// std includes
#include <string>

// boost includes
#include <boost/asio.hpp>

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagInfluxDb class is a Plag that writes datagrams to InfluxDB via the HTTP Line Protocol
 *
 * @details Translates incoming datagrams into InfluxDB Line Protocol format and POSTs them to
 * a configured InfluxDB v1/v2 endpoint over plain HTTP (no TLS). Intended as a sink Plag.
 */
class PlagInfluxDb : public Plag
{
public:
    PlagInfluxDb(const boost::property_tree::ptree & propTree,
            const std::string & name, const uint64_t & id);
    ~PlagInfluxDb();

    virtual void readConfig();

    virtual void init();

    virtual bool loopWork();

    virtual void placeDatagram(const std::shared_ptr<Datagram> datagram);

private:
    bool postLineProtocol(const std::string & lineData);

private:
    // config parameters
    std::string m_host;         //!< InfluxDB host
    uint16_t m_port;            //!< InfluxDB HTTP port (default: 8086)
    std::string m_database;     //!< InfluxDB v1 database name (or v2 bucket)
    std::string m_measurement;  //!< measurement name prefix
    std::string m_token;        //!< InfluxDB v2 API token (empty = v1 mode)
    std::string m_org;          //!< InfluxDB v2 org (empty = v1 mode)
};

#endif // PLAGINFLUXDB_HPP
