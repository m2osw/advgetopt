// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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
#include    <advgetopt/exception.h>


// self
//
#include    "catch_main.h"


// cppthread
//
#include    <cppthread/exception.h>


// snapdev
//
#include    <snapdev/safe_setenv.h>


// C++
//
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>




CATCH_TEST_CASE("logger", "[logger][valid][log]")
{
    CATCH_START_SECTION("logger: verify log levels")
    {
        CATCH_REQUIRE(to_string(cppthread::log_level_t::debug)   == "debug");
        CATCH_REQUIRE(to_string(cppthread::log_level_t::info)    == "info");
        CATCH_REQUIRE(to_string(cppthread::log_level_t::warning) == "warning");
        CATCH_REQUIRE(to_string(cppthread::log_level_t::error)   == "error");
        CATCH_REQUIRE(to_string(cppthread::log_level_t::fatal)   == "fatal");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("logger: verify log string")
    {
        SNAP_CATCH2_NAMESPACE::push_expected_log("debug: Test a regular string.");
        cppthread::log << cppthread::log_level_t::debug
                       << "Test a regular string."
                       << cppthread::end;
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        SNAP_CATCH2_NAMESPACE::push_expected_log("info: Test an std::string.");
        std::string const msg("Test an std::string.");
        cppthread::log << cppthread::log_level_t::info
                       << msg
                       << cppthread::end;
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("logger: verify log integers")
    {
        // gcc sees this one as a char
        {
            std::int8_t v(rand());
            std::string msg(std::string("warning: Test an int8_t: "));
            msg += static_cast<char>(v);
            msg += '.';
            SNAP_CATCH2_NAMESPACE::push_expected_log(msg);
            cppthread::log << cppthread::log_level_t::warning
                           << "Test an int8_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::int16_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: Test an int16_t: " + std::to_string(v) + ".");
            cppthread::log << cppthread::log_level_t::error
                           << "Test an int16_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::int32_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("fatal: Test an int32_t: " + std::to_string(v) + ".");
            cppthread::log << cppthread::log_level_t::fatal
                           << "Test an int32_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::int64_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("debug: Test an int64_t: " + std::to_string(v) + ".");
            cppthread::log << cppthread::log_level_t::debug
                           << "Test an int64_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        // gcc sees this one as a char
        {
            std::uint8_t v(rand());
            std::string const msg(std::string("info: Test an uint8_t: ") + static_cast<char>(v) + ".");
            SNAP_CATCH2_NAMESPACE::push_expected_log(msg);
            cppthread::log << cppthread::log_level_t::info
                           << "Test an uint8_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::uint16_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("warning: Test an uint16_t: " + std::to_string(static_cast<int>(v)) + ".");
            cppthread::log << cppthread::log_level_t::warning
                           << "Test an uint16_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::uint32_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: Test an uint32_t: " + std::to_string(v) + ".");
            cppthread::log << cppthread::log_level_t::error
                           << "Test an uint32_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        {
            std::uint64_t v(rand());
            SNAP_CATCH2_NAMESPACE::push_expected_log("fatal: Test an uint64_t: " + std::to_string(v) + ".");
            cppthread::log << cppthread::log_level_t::fatal
                           << "Test an uint64_t: "
                           << v
                           << "."
                           << cppthread::end;
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("logger_without_callback", "[logger][valid][log]")
{
    CATCH_START_SECTION("logger_without_callback: verify log string")
    {
        // cancel the callback for one test
        cppthread::set_log_callback(nullptr);

        //SNAP_CATCH2_NAMESPACE::push_expected_log("debug: Test without a callback."); -- not going to be registered!
        cppthread::log << cppthread::log_level_t::debug
                       << "Test without a callback."
                       << cppthread::end;

        // restore the callback
        cppthread::set_log_callback(SNAP_CATCH2_NAMESPACE::log_for_test);

        // make sure we have at least one assertion otherwise the
        // coverage will fail with an error
        //
        CATCH_REQUIRE(true);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_logger", "[logger][invalid][log]")
{
    CATCH_START_SECTION("invalid_logger: verify invalid log levels")
    {
        for(int i(0); i < 100; ++i)
        {
            cppthread::log_level_t level(cppthread::log_level_t::warning);
            for(;;)
            {
                level = static_cast<cppthread::log_level_t>(rand());
                if(level != cppthread::log_level_t::debug
                && level != cppthread::log_level_t::info
                && level != cppthread::log_level_t::warning
                && level != cppthread::log_level_t::error
                && level != cppthread::log_level_t::fatal)
                {
                    break;
                }
            }

            CATCH_REQUIRE_THROWS_MATCHES(cppthread::to_string(level)
                    , cppthread::invalid_error
                    , Catch::Matchers::ExceptionMessage(
                              "cppthread_exception: unknown log level ("
                            + std::to_string(static_cast<int>(level))
                            + ")"));
        }
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
