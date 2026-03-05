/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagWebSocket.cpp
 * @author plagn AI Assitant
 * @contributors:
 * @brief Implements the PlagWebSocket class
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

// own includes

// self include
#include "PlagWebSocket.hpp"

using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagWebSocket object and assigns default values
 * 
 */
PlagWebSocket::PlagWebSocket(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::WebSocket)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagWebSocket object
 * 
 */
PlagWebSocket::~PlagWebSocket()
{
    if (!m_stopToken) stopWork();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads many optional parameters from config and assigns them to member values
 * 
 */
void PlagWebSocket::readConfig() try
{

}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagWebSocket::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagWebSocket::init() configures the interface
 * 
 */
void PlagWebSocket::init() try
{

}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagWebSocket::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagWebSocket::loopWork regularly reads on the socket, if data popped in or sends data
 * 
 */
bool PlagWebSocket::loopWork() try
{
    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagWebSocket::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram is a function to place a Datagram here.
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagWebSocket::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagWebSocket::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
