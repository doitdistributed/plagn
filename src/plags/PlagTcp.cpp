/**
 *-------------------------------------------------------------------------------------------------
 * @file PlagTcp.cpp
 * @author plagn AI Assitant
 * @contributors:
 * @brief Implements the PlagTcp class
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
#include "PlagTcp.hpp"

using namespace std;

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Construct a new PlagTcp object and assigns default values
 * 
 */
PlagTcp::PlagTcp(const boost::property_tree::ptree & propTree,
                   const std::string & name, const uint64_t & id) :
    Plag(propTree, name, id, PlagType::TCP)
{
    readConfig();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief Destroy the PlagTcp object
 * 
 */
PlagTcp::~PlagTcp()
{
    if (!m_stopToken) stopWork();
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief reads many optional parameters from config and assigns them to member values
 * 
 */
void PlagTcp::readConfig() try
{

}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagTcp::readConfig()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagTcp::init() configures the interface
 * 
 */
void PlagTcp::init() try
{

}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagTcp::init()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief PlagTcp::loopWork regularly reads on the socket, if data popped in or sends data
 * 
 */
bool PlagTcp::loopWork() try
{
    return false;
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened during PlagTcp::loopWork()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}

/**
 *-------------------------------------------------------------------------------------------------
 * @brief placeDatagram is a function to place a Datagram here.
 *
 * @param datagram A Datagram containing data for this Plag to interprete
 */
void PlagTcp::placeDatagram(const shared_ptr<Datagram> datagram) try
{
    
}
catch (exception & e)
{
    string errorMsg = e.what();
    errorMsg += "\nSomething happened in PlagTcp::placeDatagram()";
    runtime_error eEdited(errorMsg);
    throw eEdited;
}
