/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagAmqp.hpp
 * @author plagn AI Assitant
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

// own includes
#include "Plag.hpp"

/**
 *-------------------------------------------------------------------------------------------------
 * @brief The PlagAmqp class is a Plag to interact via AMQP
 * 
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

private:
    // config parameters
    
    // worker members
};

#endif // PLAGAMQP_HPP
