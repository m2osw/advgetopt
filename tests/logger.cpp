/*
 * License:
 *    Copyright (c) 2006-2019  Made to Order Software Corp.  All Rights Reserved
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
#include <advgetopt/exception.h>

// snapdev lib
//
#include <snapdev/safe_setenv.h>

// C++ lib
//
#include <fstream>




CATCH_TEST_CASE("logger", "[logger][valid][log]")
{
    CATCH_START_SECTION("Verify log levels")
        CATCH_REQUIRE(to_string(advgetopt::log_level_t::debug)   == "debug");
        CATCH_REQUIRE(to_string(advgetopt::log_level_t::info)    == "info");
        CATCH_REQUIRE(to_string(advgetopt::log_level_t::warning) == "warning");
        CATCH_REQUIRE(to_string(advgetopt::log_level_t::error)   == "error");
        CATCH_REQUIRE(to_string(advgetopt::log_level_t::fatal)   == "fatal");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify log string")
        SNAP_CATCH2_NAMESPACE::push_expected_log("debug: Test a regular string.");
        advgetopt::log << advgetopt::log_level_t::debug
                       << "Test a regular string."
                       << advgetopt::end;
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        SNAP_CATCH2_NAMESPACE::push_expected_log("info: Test an std::string.");
        std::string const msg("Test an std::string.");
        advgetopt::log << advgetopt::log_level_t::info
                       << msg
                       << advgetopt::end;
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify log integers")
        // gcc sees this one as a char
        {
            std::int8_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log(std::string("warning: Test an int8_t: ") + static_cast<char>(v) + ".");
            advgetopt::log << advgetopt::log_level_t::warning
                           << "Test an int8_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::int16_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: Test an int16_t: " + std::to_string(v) + ".");
            advgetopt::log << advgetopt::log_level_t::error
                           << "Test an int16_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::int32_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("fatal: Test an int32_t: " + std::to_string(v) + ".");
            advgetopt::log << advgetopt::log_level_t::fatal
                           << "Test an int32_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::int64_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("debug: Test an int64_t: " + std::to_string(v) + ".");
            advgetopt::log << advgetopt::log_level_t::debug
                           << "Test an int64_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        // gcc sees this one as a char
        {
            std::uint8_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log(std::string("info: Test an uint8_t: ") + static_cast<char>(v) + ".");
            advgetopt::log << advgetopt::log_level_t::info
                           << "Test an uint8_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::uint16_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("warning: Test an uint16_t: " + std::to_string(static_cast<int>(v)) + ".");
            advgetopt::log << advgetopt::log_level_t::warning
                           << "Test an uint16_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::uint32_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: Test an uint32_t: " + std::to_string(v) + ".");
            advgetopt::log << advgetopt::log_level_t::error
                           << "Test an uint32_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::uint64_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("fatal: Test an uint64_t: " + std::to_string(v) + ".");
            advgetopt::log << advgetopt::log_level_t::fatal
                           << "Test an uint64_t: "
                           << v
                           << "."
                           << advgetopt::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

    CATCH_END_SECTION()
}


CATCH_TEST_CASE("logger_without_callback", "[logger][valid][log]")
{
    CATCH_START_SECTION("Verify log string")
        // cancel the callback for one test
        advgetopt::set_log_callback(nullptr);

        //SNAP_CATCH2_NAMESPACE::push_expected_log("debug: Test without a callback."); -- not going to be registered!
        advgetopt::log << advgetopt::log_level_t::debug
                       << "Test without a callback."
                       << advgetopt::end;

        // restore the callback
        advgetopt::set_log_callback(SNAP_CATCH2_NAMESPACE::log_for_test);
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_logger", "[logger][invalid][log]")
{
    CATCH_START_SECTION("Verify invalid log levels")
        for(int i(0); i < 100; ++i)
        {
            advgetopt::log_level_t level(advgetopt::log_level_t::warning);
            for(;;)
            {
                level = static_cast<advgetopt::log_level_t>(rand());
                if(level != advgetopt::log_level_t::debug
                && level != advgetopt::log_level_t::info
                && level != advgetopt::log_level_t::warning
                && level != advgetopt::log_level_t::error
                && level != advgetopt::log_level_t::fatal)
                {
                    break;
                }
            }

            CATCH_REQUIRE_THROWS_MATCHES(advgetopt::to_string(level)
                    , advgetopt::getopt_exception_invalid
                    , Catch::Matchers::ExceptionMessage(
                              "unknown log level ("
                            + std::to_string(static_cast<int>(level))
                            + ")"));
        }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
