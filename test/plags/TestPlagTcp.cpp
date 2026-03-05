/**
 *-------------------------------------------------------------------------------------------------
 * @file TestPlagTcp.cpp
 * @author plagn AI Assitant
 * @contributors:
 * @brief Tests for the PlagTcp class
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

#include <boost/test/unit_test.hpp>
#include <boost/property_tree/ptree.hpp>
#include "PlagTcp.hpp"

BOOST_AUTO_TEST_SUITE(PlagTcpTestSuite)

BOOST_AUTO_TEST_CASE(test_initialization)
{
    boost::property_tree::ptree pt;
    pt.put("plag1.remoteIp", "127.0.0.1");
    pt.put("plag1.port", "8080");
    PlagTcp plag(pt, "plag1", 1);
    BOOST_CHECK_EQUAL(plag.getName(), "plag1");
}

BOOST_AUTO_TEST_SUITE_END()
