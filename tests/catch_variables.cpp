// Copyright (c) 2006-2024  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/advgetopt
// contact@m2osw.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

// advgetopt
//
#include    <advgetopt/variables.h>

#include    <advgetopt/exception.h>


// self
//
#include    "catch_main.h"


// C++
//
#include    <fstream>
#include    <iomanip>


// last include
//
#include    <snapdev/poison.h>






CATCH_TEST_CASE("variables", "[variables][valid]")
{
    CATCH_START_SECTION("variables: check the variables class")
    {
        advgetopt::variables vars;

        // not set yet
        CATCH_REQUIRE(vars.get_variable("first-variable") == std::string());
        CATCH_REQUIRE(vars.get_variables().empty());

        CATCH_REQUIRE_FALSE(vars.has_variable("first_variable"));
        vars.set_variable("first_variable", "it works");
        CATCH_REQUIRE(vars.get_variables().size() == 1);
        CATCH_REQUIRE(vars.has_variable("first_variable"));
        CATCH_REQUIRE(vars.has_variable("first-variable"));
        CATCH_REQUIRE(vars.get_variable("first_variable") == "it works");
        CATCH_REQUIRE(vars.get_variable("first-variable") == "it works");

        CATCH_REQUIRE_FALSE(vars.has_variable("second::variable"));
        vars.set_variable("second::variable", "double colon");
        CATCH_REQUIRE(vars.get_variables().size() == 2);
        CATCH_REQUIRE(vars.has_variable("second.variable"));
        CATCH_REQUIRE(vars.has_variable("second..variable"));
        CATCH_REQUIRE(vars.has_variable("second...variable"));
        CATCH_REQUIRE(vars.has_variable("second....variable"));
        CATCH_REQUIRE(vars.has_variable("second:variable"));
        CATCH_REQUIRE(vars.has_variable("second::variable"));
        CATCH_REQUIRE(vars.has_variable("second:::variable"));
        CATCH_REQUIRE(vars.has_variable("second::::variable"));
        CATCH_REQUIRE(vars.get_variable("second.variable") == "double colon");
        CATCH_REQUIRE(vars.get_variable("second..variable") == "double colon");
        CATCH_REQUIRE(vars.get_variable("second...variable") == "double colon");
        CATCH_REQUIRE(vars.get_variable("second....variable") == "double colon");
        CATCH_REQUIRE(vars.get_variable("second:variable") == "double colon");
        CATCH_REQUIRE(vars.get_variable("second::variable") == "double colon");
        CATCH_REQUIRE(vars.get_variable("second:::variable") == "double colon");
        CATCH_REQUIRE(vars.get_variable("second::::variable") == "double colon");

        CATCH_REQUIRE_FALSE(vars.has_variable("third::::variable"));
        vars.set_variable("third::::variable", "scope operator");
        CATCH_REQUIRE(vars.get_variables().size() == 3);
        CATCH_REQUIRE(vars.has_variable("third::variable"));
        CATCH_REQUIRE(vars.get_variable("third::variable") == "scope operator");

        // change value
        vars.set_variable("first_variable", "replaced value");
        CATCH_REQUIRE(vars.get_variable("first_variable") == "replaced value");
        CATCH_REQUIRE(vars.get_variables().size() == 3);

        // attempt changing value when already set
        vars.set_variable("first_variable", "ignored value", advgetopt::assignment_t::ASSIGNMENT_OPTIONAL);
        CATCH_REQUIRE(vars.get_variable("first_variable") == "replaced value");
        CATCH_REQUIRE(vars.get_variables().size() == 3);

        advgetopt::variables::variable_t list(vars.get_variables());
        for(auto l : list)
        {
            if(l.first == "first-variable")
            {
                CATCH_REQUIRE(l.second == "replaced value");
            }
            else if(l.first == "second::variable")
            {
                CATCH_REQUIRE(l.second == "double colon");
            }
            else
            {
                // if not 1st or 2nd, it has to be 3rd
                //
                CATCH_REQUIRE(l.first == "third::variable");
                CATCH_REQUIRE(l.second == "scope operator");
            }
        }

        // valid
        //
        std::string processed(vars.process_value("First Var = [${first-variable}]"));
        CATCH_REQUIRE(processed == "First Var = [replaced value]");

        // missing '}'
        //
        processed = vars.process_value("First Var = [${first-variable]");
        CATCH_REQUIRE(processed == "First Var = [${first-variable]");

        vars.set_variable("loopA", "ref ${loopB}", advgetopt::assignment_t::ASSIGNMENT_OPTIONAL);
        CATCH_REQUIRE(vars.get_variable("loopA") == "ref ${loopB}");
        CATCH_REQUIRE(vars.get_variables().size() == 4);

        vars.set_variable("loopB", "ref ${loopA}", advgetopt::assignment_t::ASSIGNMENT_OPTIONAL);
        CATCH_REQUIRE(vars.get_variable("loopB") == "ref ${loopA}");
        CATCH_REQUIRE(vars.get_variables().size() == 5);

        processed = vars.process_value("Looping like crazy: ${loopA}");
        CATCH_REQUIRE(processed == "Looping like crazy: ref ref <variable \"loopA\" loops>");

        processed = vars.process_value("Looping like crazy: ${loopB}");
        CATCH_REQUIRE(processed == "Looping like crazy: ref ref <variable \"loopB\" loops>");

        vars.set_variable("cummulative", "start", advgetopt::assignment_t::ASSIGNMENT_NEW);
        CATCH_REQUIRE(vars.get_variable("cummulative") == "start");
        CATCH_REQUIRE(vars.get_variables().size() == 6);
        vars.set_variable("cummulative", "-middle-", advgetopt::assignment_t::ASSIGNMENT_APPEND);
        CATCH_REQUIRE(vars.get_variable("cummulative") == "start-middle-");
        CATCH_REQUIRE(vars.get_variables().size() == 6);
        vars.set_variable("cummulative", "end", advgetopt::assignment_t::ASSIGNMENT_APPEND);
        CATCH_REQUIRE(vars.get_variable("cummulative") == "start-middle-end");
        CATCH_REQUIRE(vars.get_variables().size() == 6);

        vars.set_variable("additive", "beg", advgetopt::assignment_t::ASSIGNMENT_APPEND);
        CATCH_REQUIRE(vars.get_variable("additive") == "beg");
        CATCH_REQUIRE(vars.get_variables().size() == 7);
        vars.set_variable("additive", ":mid", advgetopt::assignment_t::ASSIGNMENT_APPEND);
        CATCH_REQUIRE(vars.get_variable("additive") == "beg:mid");
        CATCH_REQUIRE(vars.get_variables().size() == 7);
        vars.set_variable("additive", ":end", advgetopt::assignment_t::ASSIGNMENT_APPEND);
        CATCH_REQUIRE(vars.get_variable("additive") == "beg:mid:end");
        CATCH_REQUIRE(vars.get_variables().size() == 7);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_variable_name", "[variables][invalid]")
{
    CATCH_START_SECTION("invalid_variable_name: parsing an empty section name throws")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::variables::canonicalize_variable_name(":bad_start")
                , advgetopt::getopt_invalid
                , Catch::Matchers::ExceptionMessage(
                      "getopt_exception: found an empty section name in \":bad_start\"."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_variable_name: parsing first section name that start with a digit fails")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::variables::canonicalize_variable_name("3::bad_start")
                , advgetopt::getopt_invalid
                , Catch::Matchers::ExceptionMessage(
                      "getopt_exception: a variable name or section name in \"3::bad_start\" starts with a digit, which is not allowed."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_variable_name: parsing second section name that start with a digit fails")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::variables::canonicalize_variable_name("good::3::bad_section")
                , advgetopt::getopt_invalid
                , Catch::Matchers::ExceptionMessage(
                      "getopt_exception: a variable name or section name in \"good::3::bad_section\" starts with a digit, which is not allowed."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_variable_name: parsing variable name that start with a digit fails")
        CATCH_REQUIRE_THROWS_MATCHES(
                  advgetopt::variables::canonicalize_variable_name("good::and_bad::9times")
                , advgetopt::getopt_invalid
                , Catch::Matchers::ExceptionMessage(
                      "getopt_exception: a variable name or section name in \"good::and_bad::9times\" starts with a digit, which is not allowed."));
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_variable", "[variables][invalid]")
{
    CATCH_START_SECTION("invalid_variable: NEW assignment fails if variable exists")
    {
        advgetopt::variables vars;

        vars.set_variable("unique", "works", advgetopt::assignment_t::ASSIGNMENT_NEW);
        CATCH_REQUIRE(vars.get_variable("unique") == "works");
        CATCH_REQUIRE(vars.get_variables().size() == 1);

        CATCH_REQUIRE_THROWS_MATCHES(
                  vars.set_variable("unique", "fail", advgetopt::assignment_t::ASSIGNMENT_NEW)
                , advgetopt::getopt_defined_twice
                , Catch::Matchers::ExceptionMessage(
                      "getopt_exception: variable \"unique\" is already defined."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
