/*
 * Files:
 *    tests/data.cpp
 *
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






CATCH_TEST_CASE("string_access", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Verify a string in a long argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("user-name")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("check specified user.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: user name as a string";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--user-name",
            "alexis",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE_FALSE(opt.has_default("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE_FALSE(opt.has_default("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("user-name") != nullptr);
        CATCH_REQUIRE(opt.get_option('u') != nullptr);
        CATCH_REQUIRE(opt.get_string("user-name") == "alexis");
        CATCH_REQUIRE(opt.get_string("user-name", 0) == "alexis");
        CATCH_REQUIRE(opt.is_defined("user-name"));
        CATCH_REQUIRE_FALSE(opt.has_default("user-name"));
        CATCH_REQUIRE(opt.get_default("user-name").empty());
        CATCH_REQUIRE(opt.size("user-name") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a string in a short argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("user-name")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("check specified user.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: user name as a string";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "-u",
            "alexis",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("user-name") != nullptr);
        CATCH_REQUIRE(opt.get_option('u') != nullptr);
        CATCH_REQUIRE(opt.get_string("user-name") == "alexis");
        CATCH_REQUIRE(opt.get_string("user-name", 0) == "alexis");
        CATCH_REQUIRE(opt.is_defined("user-name"));
        CATCH_REQUIRE(opt.get_default("user-name").empty());
        CATCH_REQUIRE(opt.size("user-name") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("long_access", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Verify an integer (long) value in a long argument")
        long const default_value(rand());
        std::string const default_value_str(std::to_string(default_value));
        char const * const default_val(default_value_str.c_str());

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue(default_val)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "9821",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "9821");
        CATCH_REQUIRE(opt.get_string("size", 0) == "9821");
        CATCH_REQUIRE(opt.get_long("size") == 9821);
        CATCH_REQUIRE(opt.get_long("size", 0) == 9821);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == default_value_str);
        CATCH_REQUIRE(opt.size("size") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify an integer (long) value in a short argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "-s",
            "9821",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "9821");
        CATCH_REQUIRE(opt.get_string("size", 0) == "9821");
        CATCH_REQUIRE(opt.get_long("size") == 9821);
        CATCH_REQUIRE(opt.get_long("size", 0) == 9821);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size").empty());
        CATCH_REQUIRE(opt.size("size") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify an integer (long) value in no arguments")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("839")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "839");
        CATCH_REQUIRE(opt.get_string("size", 0) == "839");
        CATCH_REQUIRE(opt.get_long("size") == 839);
        CATCH_REQUIRE(opt.get_long("size", 0) == 839);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "839");
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("invalid_option_name", "[arguments][invalid][getopt]")
{
    CATCH_START_SECTION("Verify that asking for the string of a non-existant option fails")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: test get_string() functions";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("non-existant")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("non-existant", 0)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("non-existant", 1)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "there is no --non-existant option defined."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that asking for the long of a non-existant option fails")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: test get_string() functions";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("non-existant")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("non-existant", 0)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("non-existant", 1)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "there is no --non-existant option defined."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that asking for a default with an empty string fails")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: test get_default() functions";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.has_default("")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.has_default(std::string())
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_default("")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_default(std::string())
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "argument name cannot be empty."));
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("missing_default_value", "[arguments][invalid][getopt]")
{
    CATCH_START_SECTION("Verify a string value without arguments and no default")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_string() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.size("size") == 0);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size", 0)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size", 1)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no default."));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify an integer (long) value without arguments and no default")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE_FALSE(opt.has_default("size"));
        CATCH_REQUIRE(opt.size("size") == 0);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 0)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 1)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no or an empty default."));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify an integer (long) value without arguments and an empty string as default")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.size("size") == 0);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 0)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 1)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "the --size option was not defined on the command line and it has no or an empty default."));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("incompatible_default_value", "[arguments][invalid][getopt]")
{
    CATCH_START_SECTION("Verify an integer (long) value without arguments and a non-numeric default")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("undefined")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "undefined"); // this works, it fails with get_long() though
        CATCH_REQUIRE(opt.size("size") == 0);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size")
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "invalid default number \"undefined\" for option --size"));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 0)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "invalid default number \"undefined\" for option --size"));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 1)
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "invalid default number \"undefined\" for option --size"));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("out_of_range_value", "[arguments][invalid][getopt]")
{
    CATCH_START_SECTION("Verify an integer (long) value without arguments and a non-numeric default")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("-300")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "312",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "312");
        CATCH_REQUIRE(opt.get_string("size", 0) == "312");
        CATCH_REQUIRE(opt.get_long("size") == 312);
        CATCH_REQUIRE(opt.get_long("size", 0) == 312);
        CATCH_REQUIRE(opt.get_default("size") == "-300");
        CATCH_REQUIRE(opt.size("size") == 1);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: 312 is out of bounds (-100..100 inclusive) in parameter --size.");
        CATCH_REQUIRE(opt.get_long("size", 0, -100, 100) == -1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify an integer (long) value without arguments and a non-numeric default")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("-300")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: test get_long() functions";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size") == "-300");
        CATCH_REQUIRE(opt.size("size") == 0);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: -300 is out of bounds (-100..100 inclusive) in parameter --size.");
        CATCH_REQUIRE(opt.get_long("size", 0, -100, 100) == -1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
