// Copyright (c) 2006-2022  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// advgetopt
//
#include    <advgetopt/exception.h>
#include    <advgetopt/variables.h>


// C++
//
#include    <fstream>
#include    <iomanip>


// last include
//
#include    <snapdev/poison.h>






CATCH_TEST_CASE("variables", "[variables][valid]")
{
    CATCH_START_SECTION("Check the variables class")
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

        vars.set_variable("loopA", "ref ${loopB}");
        vars.set_variable("loopB", "ref ${loopA}");

        processed = vars.process_value("Looping like crazy: ${loopA}");
        CATCH_REQUIRE(processed == "Looping like crazy: ref ref <variable \"loopA\" loops>");

        processed = vars.process_value("Looping like crazy: ${loopB}");
        CATCH_REQUIRE(processed == "Looping like crazy: ref ref <variable \"loopB\" loops>");
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
