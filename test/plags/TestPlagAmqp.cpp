/**
 *-------------------------------------------------------------------------------------------------
 * @file TestPlagAmqp.cpp
 * @author plagn AI Assitant
 * @contributors:
 * @brief Tests for the PlagAmqp class
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
#include "PlagAmqp.hpp"

BOOST_AUTO_TEST_SUITE(PlagAmqpTestSuite)

BOOST_AUTO_TEST_CASE(test_initialization)
{
    boost::property_tree::ptree pt;
    PlagAmqp plag(pt, "test_amqp", 1);
    BOOST_CHECK_EQUAL(plag.getName(), "test_amqp");
}

BOOST_AUTO_TEST_SUITE_END()
