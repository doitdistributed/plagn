/**
 *-------------------------------------------------------------------------------------------------
 * @file DatagramMcp.hpp
 * @author plagn AI Assitant
 * @contributors:
 * @brief Defines the DatagramMcp class
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
#ifndef DATAGRAM_MCP_HPP
#define DATAGRAM_MCP_HPP

#include "Datagram.hpp"

class DatagramMcp : public Datagram
{
public:
    DatagramMcp(const std::string& sender, const std::string& json);

    std::string getJson() const;

private:
    std::string m_json;
};

#endif // DATAGRAM_MCP_HPP
