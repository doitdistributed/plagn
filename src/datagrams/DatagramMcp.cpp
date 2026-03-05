/**
 *-------------------------------------------------------------------------------------------------
 * @file DatagramMcp.cpp
 * @author plagn AI Assitant
 * @contributors:
 * @brief Implements the DatagramMcp class
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

#include "DatagramMcp.hpp"

DatagramMcp::DatagramMcp(const std::string& sender, const std::string& json) :
    Datagram(sender),
    m_json(json)
{
}

std::string DatagramMcp::getJson() const
{
    return m_json;
}
