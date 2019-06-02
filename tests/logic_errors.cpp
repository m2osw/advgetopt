/*
 * Files:
 *    tests/logic_errors.cpp
 *
 * License:
 *    Copyright (c) 2019  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/
 *    contact@m2osw.com
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *    Alexis Wilke   alexis@m2osw.com
 */

// self
//
#include "main.h"

// advgetopt lib
//
#include <advgetopt/advgetopt.h>
#include <advgetopt/exception.h>
#include <advgetopt/log.h>

// C++ lib
//
#include <cstring>
#include <cmath>
#include <sstream>
#include <fstream>

// C lib
//
#include <time.h>


namespace
{



advgetopt::options_environment *    g_options_environment = nullptr; // to test pasing a reference to a null pointer





} // no name namespace


// TODO: get rid of this file once we replicated all of these (two test) in other locations

CATCH_TEST_CASE("getop_logic_errors", "[invalid]")
{
    CATCH_SECTION("initialiazing getopt with null pointers")
    {
        // argv can't be a nullptr
        //
        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(*g_options_environment, 0, nullptr), advgetopt::getopt_exception_logic);

        // g_options_environment can't be a reference to a null pointer
        // (I know that is not very likely but if you allocate the
        // object and end up with a null pointer at the time of the call
        // it can happen.)
        //
        char * argv[] = { const_cast<char *>("program.exe"), nullptr };
        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(*g_options_environment, 0, argv), advgetopt::getopt_exception_logic);
    }

    CATCH_SECTION("initialiazing getopt with null pointers")
    {
        advgetopt::options_environment options_environment;
        char * argv[] = { const_cast<char *>("program.exe"), nullptr };
        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_environment, 1, argv), advgetopt::getopt_exception_logic);
    }
}





// vim: ts=4 sw=4 et
