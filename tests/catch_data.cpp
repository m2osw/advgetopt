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


// advgetopt lib
//
#include    <advgetopt/exception.h>
#include    <advgetopt/version.h>


// snapdev lib
//
#include    <snapdev/safe_setenv.h>


// booost lib
//
#include    <boost/preprocessor/stringize.hpp>


// C++ lib
//
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>





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
        CATCH_REQUIRE(opt["user-name"] == "alexis");
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
        CATCH_REQUIRE(opt["user-name"] == "alexis");
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
        CATCH_REQUIRE(opt["size"] == "9821");
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

        // to reach the constant version `opt` has to be constant
        std::string const array_syntax_invalid_parameter(static_cast<advgetopt::getopt const &>(opt)["invalid-parameter"]);
        CATCH_REQUIRE(array_syntax_invalid_parameter == std::string());

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
        CATCH_REQUIRE(opt["size"] == "9821");
        CATCH_REQUIRE(opt.get_long("size") == 9821);
        CATCH_REQUIRE(opt.get_long("size", 0) == 9821);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size").empty());
        CATCH_REQUIRE(opt.size("size") == 1);

        // to access the constant reference we need a constant `opt`...
        std::string const array_syntax1(static_cast<advgetopt::getopt const &>(opt)["size"]);
        CATCH_REQUIRE(array_syntax1 == "9821");
        bool const array_syntax2(static_cast<advgetopt::getopt const &>(opt)["size"] == std::string("9821"));
        CATCH_REQUIRE(array_syntax2);

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
        CATCH_REQUIRE(opt["invalid-parameter"] == std::string());
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

        // with a constant opt, the array syntax returns the default string
        CATCH_REQUIRE(static_cast<advgetopt::getopt const &>(opt)["size"] == "839");

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_version", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --version system flag")
    {
        long const major_version(rand());
        long const minor_version(rand());
        long const patch_version(rand());
        long const build_version(rand());
        std::string const version(std::to_string(major_version)
                                + "."
                                + std::to_string(minor_version)
                                + "."
                                + std::to_string(patch_version)
                                + "."
                                + std::to_string(build_version));

        long const default_value(rand());
        std::string const default_val(std::to_string(default_value));
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue(default_val.c_str())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_version = version.c_str();

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--version",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == default_val);
        CATCH_REQUIRE(opt.get_string("size", 0) == default_val);
        CATCH_REQUIRE(opt["size"] == default_val);
        CATCH_REQUIRE(opt.get_long("size") == default_value);
        CATCH_REQUIRE(opt.get_long("size", 0) == default_value);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == default_val);
        CATCH_REQUIRE(opt.size("size") == 0);

        // version parameter
        CATCH_REQUIRE(opt.get_option("version") != nullptr);
        CATCH_REQUIRE(opt.get_option('V') == opt.get_option("version"));
        CATCH_REQUIRE(opt.is_defined("version"));
        CATCH_REQUIRE(opt.get_string("version") == "");
        CATCH_REQUIRE(opt.get_string("version", 0) == "");
        CATCH_REQUIRE(opt["version"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("version"));
        CATCH_REQUIRE(opt.get_default("version").empty());
        CATCH_REQUIRE(opt.size("version") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_VERSION);
        CATCH_REQUIRE(ss.str() == version + '\n');
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --version system flag, without a --version on the command line")
    {
        long const major_version(rand());
        long const minor_version(rand());
        long const patch_version(rand());
        long const build_version(rand());
        std::string const version(std::to_string(major_version)
                                + "."
                                + std::to_string(minor_version)
                                + "."
                                + std::to_string(patch_version)
                                + "."
                                + std::to_string(build_version));

        long const default_value(rand());
        std::string const default_val(std::to_string(default_value));
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue(default_val.c_str())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_version = version.c_str();

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == default_val);
        CATCH_REQUIRE(opt.size("size") == 1);

        // version parameter
        CATCH_REQUIRE(opt.get_option("version") != nullptr);
        CATCH_REQUIRE(opt.get_option('V') == opt.get_option("version"));
        CATCH_REQUIRE_FALSE(opt.is_defined("version"));
        CATCH_REQUIRE_FALSE(opt.has_default("version"));
        CATCH_REQUIRE(opt.get_default("version").empty());
        CATCH_REQUIRE(opt.size("version") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_has_sanitizer", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --has-sanitizer system flag")
    {
        long const default_value(rand());
        std::string const default_val(std::to_string(default_value));
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue(default_val.c_str())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_version = "2.0.24.0";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--has-sanitizer",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == default_val);
        CATCH_REQUIRE(opt.get_string("size", 0) == default_val);
        CATCH_REQUIRE(opt["size"] == default_val);
        CATCH_REQUIRE(opt.get_long("size") == default_value);
        CATCH_REQUIRE(opt.get_long("size", 0) == default_value);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == default_val);
        CATCH_REQUIRE(opt.size("size") == 0);

        // has-sanitizer parameter
        CATCH_REQUIRE(opt.get_option("has-sanitizer") != nullptr);
        CATCH_REQUIRE(opt.is_defined("has-sanitizer"));
        CATCH_REQUIRE(opt.get_string("has-sanitizer") == "");
        CATCH_REQUIRE(opt.get_string("has-sanitizer", 0) == "");
        CATCH_REQUIRE(opt["has-sanitizer"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("has-sanitizer"));
        CATCH_REQUIRE(opt.get_default("has-sanitizer").empty());
        CATCH_REQUIRE(opt.size("has-sanitizer") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_HELP);
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
        // when running coverage or in the Sanitize version
        //
        std::string const expected(
#ifdef __SANITIZE_ADDRESS__
                "The address sanitizer is compiled in.\n"
#endif
#ifdef __SANITIZE_THREAD__
                "The thread sanitizer is compiled in.\n"
#endif
            );
        CATCH_REQUIRE(ss.str() == expected);
#else
        // when running in Debug or Release
        //
        CATCH_REQUIRE(ss.str() == "The address and thread sanitizers are not compiled in.\n");
#endif
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --has-sanitizer system flag, without a --has-sanitizer on the command line")
    {
        long const default_value(rand());
        std::string const default_val(std::to_string(default_value));
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue(default_val.c_str())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_version = "2.0.24.1";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == default_val);
        CATCH_REQUIRE(opt.size("size") == 1);

        // has-sanitizer parameter
        CATCH_REQUIRE(opt.get_option("has-sanitizer") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("has-sanitizer"));
        CATCH_REQUIRE_FALSE(opt.has_default("has-sanitizer"));
        CATCH_REQUIRE(opt.get_default("has-sanitizer").empty());
        CATCH_REQUIRE(opt.size("has-sanitizer") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_compiler_version", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --compiler-version system flag")
    {
        long const default_value(rand());
        std::string const default_val(std::to_string(default_value));
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue(default_val.c_str())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_version = "2.0.24.0";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--compiler-version",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == default_val);
        CATCH_REQUIRE(opt.get_string("size", 0) == default_val);
        CATCH_REQUIRE(opt["size"] == default_val);
        CATCH_REQUIRE(opt.get_long("size") == default_value);
        CATCH_REQUIRE(opt.get_long("size", 0) == default_value);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == default_val);
        CATCH_REQUIRE(opt.size("size") == 0);

        // compiler-version parameter
        CATCH_REQUIRE(opt.get_option("compiler-version") != nullptr);
        CATCH_REQUIRE(opt.is_defined("compiler-version"));
        CATCH_REQUIRE(opt.get_string("compiler-version") == "");
        CATCH_REQUIRE(opt.get_string("compiler-version", 0) == "");
        CATCH_REQUIRE(opt["compiler-version"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("compiler-version"));
        CATCH_REQUIRE(opt.get_default("compiler-version").empty());
        CATCH_REQUIRE(opt.size("compiler-version") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_HELP);
        CATCH_REQUIRE(ss.str() == LIBADVGETOPT_COMPILER_VERSION "\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --compiler-version system flag, without a --compiler-version on the command line")
    {
        long const default_value(rand());
        std::string const default_val(std::to_string(default_value));
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue(default_val.c_str())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_version = "2.0.24.1";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == default_val);
        CATCH_REQUIRE(opt.size("size") == 1);

        // compiler-version parameter
        CATCH_REQUIRE(opt.get_option("compiler-version") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("compiler-version"));
        CATCH_REQUIRE_FALSE(opt.has_default("compiler-version"));
        CATCH_REQUIRE(opt.get_default("compiler-version").empty());
        CATCH_REQUIRE(opt.size("compiler-version") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_help", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --help system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("33")
            ),
            advgetopt::define_option(
                  advgetopt::Name("obscure")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("obscure command, hidden by default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("secret")
                , advgetopt::ShortName('S')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("even more secret command, hidden by default.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--help",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // check that the result is valid

        // check the list of options
        advgetopt::option_info::map_by_name_t const & list_of_options(opt.get_options());
        CATCH_REQUIRE(list_of_options.size() == 3 + 11 + 1);

        // user options
        CATCH_REQUIRE(list_of_options.find("size") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("obscure") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("secret") != list_of_options.end());

        // system options
        CATCH_REQUIRE(list_of_options.find("help") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("long-help") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("version") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("copyright") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("license") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("build-date") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("environment-variable-name") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("configuration-filenames") != list_of_options.end());
        CATCH_REQUIRE(list_of_options.find("path-to-option-definitions") != list_of_options.end());

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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "33");
        CATCH_REQUIRE(opt.get_string("size", 0) == "33");
        CATCH_REQUIRE(opt["size"] == "33");
        CATCH_REQUIRE(opt.get_long("size") == 33);
        CATCH_REQUIRE(opt.get_long("size", 0) == 33);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "33");
        CATCH_REQUIRE(opt.size("size") == 0);

        // help parameter
        CATCH_REQUIRE(opt.get_option("help") != nullptr);
        CATCH_REQUIRE(opt.get_option('h') == opt.get_option("help"));
        CATCH_REQUIRE(opt.is_defined("help"));
        CATCH_REQUIRE(opt.get_string("help") == "");
        CATCH_REQUIRE(opt.get_string("help", 0) == "");
        CATCH_REQUIRE(opt["help"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("help"));
        CATCH_REQUIRE(opt.get_default("help").empty());
        CATCH_REQUIRE(opt.size("help") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_HELP);
        CATCH_REQUIRE_LONG_STRING(ss.str(),
advgetopt::getopt::breakup_line(
              "Usage: test system commands"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--build-date"
            , "print out the time and date when arguments was built and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--compiler-version"
            , "print the version of the compiler used to compile the advgetopt library."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--configuration-filenames"
            , "print out the list of configuration files checked out by this"
              " tool."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--copyright or -C"
            , "print out the copyright of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--environment-variable-name"
            , "print out the name of the environment variable supported by"
              " arguments (if any.)"
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--has-sanitizer"
            , "print whether the advgetopt was compiled with the sanitizer extension."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--help or -h"
            , "print out this help screen and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--license or -L"
            , "print out the license of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--path-to-option-definitions"
            , "print out the path to the option definitions."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--show-option-sources"
            , "parse all the options and then print out the source of each"
              " value and each override."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--size or -s <arg> (default is \"33\")"
            , "define the size."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--version or -V"
            , "print out the version of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Copyright matters\n"
  "\n"
                    );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --long-help system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("33")
            ),
            advgetopt::define_option(
                  advgetopt::Name("obscure")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("obscure command, hidden by default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("secret")
                , advgetopt::ShortName('S')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("even more secret command, hidden by default.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--long-help",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        CATCH_REQUIRE(opt.get_group_name() == std::string());

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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "33");
        CATCH_REQUIRE(opt.get_string("size", 0) == "33");
        CATCH_REQUIRE(opt["size"] == "33");
        CATCH_REQUIRE(opt.get_long("size") == 33);
        CATCH_REQUIRE(opt.get_long("size", 0) == 33);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "33");
        CATCH_REQUIRE(opt.size("size") == 0);

        // help parameter
        CATCH_REQUIRE(opt.get_option("long-help") != nullptr);
        CATCH_REQUIRE(opt.get_option('?') == opt.get_option("long-help"));
        CATCH_REQUIRE(opt.is_defined("long-help"));
        CATCH_REQUIRE(opt.get_string("long-help") == "");
        CATCH_REQUIRE(opt.get_string("long-help", 0) == "");
        CATCH_REQUIRE(opt["long-help"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("long-help"));
        CATCH_REQUIRE(opt.get_default("long-help").empty());
        CATCH_REQUIRE(opt.size("long-help") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_HELP);
        CATCH_REQUIRE_LONG_STRING(ss.str(),
advgetopt::getopt::breakup_line(
              "Usage: test system commands"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--build-date"
            , "print out the time and date when arguments was built and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--compiler-version"
            , "print the version of the compiler used to compile the advgetopt library."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--configuration-filenames"
            , "print out the list of configuration files checked out by this"
              " tool."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--copyright or -C"
            , "print out the copyright of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--environment-variable-name"
            , "print out the name of the environment variable supported by"
              " arguments (if any.)"
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--has-sanitizer"
            , "print whether the advgetopt was compiled with the sanitizer extension."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--help or -h"
            , "print out this help screen and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--license or -L"
            , "print out the license of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--obscure or -o <arg>"
            , "obscure command, hidden by default."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--path-to-option-definitions"
            , "print out the path to the option definitions."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--secret or -S <arg>"
            , "even more secret command, hidden by default."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--show-option-sources"
            , "parse all the options and then print out the source of each"
              " value and each override."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--size or -s <arg> (default is \"33\")"
            , "define the size."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--version or -V"
            , "print out the version of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Copyright matters\n"
  "\n"
                    );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --help system flag, without a --help on the command line")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("33")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "33");
        CATCH_REQUIRE(opt.size("size") == 1);

        // help parameter
        CATCH_REQUIRE(opt.get_option("help") != nullptr);
        CATCH_REQUIRE(opt.get_option('h') == opt.get_option("help"));
        CATCH_REQUIRE_FALSE(opt.is_defined("help"));
        CATCH_REQUIRE_FALSE(opt.has_default("help"));
        CATCH_REQUIRE(opt.get_default("help").empty());
        CATCH_REQUIRE(opt.size("help") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --commmands-help system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("33")
            ),
            advgetopt::define_option(
                  advgetopt::Name("obscure")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
                , advgetopt::Help("obscure command, hidden by default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("secret")
                , advgetopt::ShortName('S')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
                , advgetopt::Help("even more secret command, hidden by default.")
            ),
            advgetopt::end_options()
        };

        advgetopt::group_description const groups[] =
        {
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
                , advgetopt::GroupName("commands")
                , advgetopt::GroupDescription("Commands:")
            ),
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
                , advgetopt::GroupName("option")
                , advgetopt::GroupDescription("Options:")
            ),
            advgetopt::end_groups()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";
        environment_options.f_groups = groups;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--commands-help",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "33");
        CATCH_REQUIRE(opt.get_string("size", 0) == "33");
        CATCH_REQUIRE(opt["size"] == "33");
        CATCH_REQUIRE(opt.get_long("size") == 33);
        CATCH_REQUIRE(opt.get_long("size", 0) == 33);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "33");
        CATCH_REQUIRE(opt.size("size") == 0);

        // help parameter
        CATCH_REQUIRE(opt.get_option("commands-help") != nullptr);
        CATCH_REQUIRE(opt.is_defined("commands-help"));
        CATCH_REQUIRE(opt.get_string("commands-help") == "");
        CATCH_REQUIRE(opt.get_string("commands-help", 0) == "");
        CATCH_REQUIRE(opt["commands-help"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("commands-help"));
        CATCH_REQUIRE(opt.get_default("commands-help").empty());
        CATCH_REQUIRE(opt.size("commands-help") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_HELP);
        CATCH_REQUIRE_LONG_STRING(ss.str(),
advgetopt::getopt::breakup_line(
              "Usage: test system commands"
            , 0
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Commands:\n"
+ advgetopt::getopt::format_usage_string(
              "--build-date"
            , "print out the time and date when arguments was built and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--commands-help"
            , "show help from the \"commands\" group of options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--compiler-version"
            , "print the version of the compiler used to compile the advgetopt library."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--configuration-filenames"
            , "print out the list of configuration files checked out by this"
              " tool."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--copyright or -C"
            , "print out the copyright of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--environment-variable-name"
            , "print out the name of the environment variable supported by"
              " arguments (if any.)"
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--has-sanitizer"
            , "print whether the advgetopt was compiled with the sanitizer extension."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--help or -h"
            , "print out this help screen and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--license or -L"
            , "print out the license of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--obscure or -o <arg>"
            , "obscure command, hidden by default."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--option-help"
            , "show help from the \"option\" group of options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--path-to-option-definitions"
            , "print out the path to the option definitions."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--show-option-sources"
            , "parse all the options and then print out the source of each"
              " value and each override."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--size or -s <arg> (default is \"33\")"
            , "define the size."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--version or -V"
            , "print out the version of arguments and exit."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Copyright matters\n"
  "\n"
                    );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --options-help system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("33")
            ),
            advgetopt::define_option(
                  advgetopt::Name("obscure")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
                , advgetopt::Help("obscure command, hidden by default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("secret")
                , advgetopt::ShortName('S')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                          , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
                , advgetopt::Help("even more secret command, hidden by default.")
            ),
            advgetopt::end_options()
        };

        advgetopt::group_description const groups[] =
        {
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
                , advgetopt::GroupName("commands")
                , advgetopt::GroupDescription("Commands:")
            ),
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
                , advgetopt::GroupName("options")
                , advgetopt::GroupDescription("Options:")
            ),
            advgetopt::end_groups()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";
        environment_options.f_groups = groups;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--options-help",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "33");
        CATCH_REQUIRE(opt.get_string("size", 0) == "33");
        CATCH_REQUIRE(opt["size"] == "33");
        CATCH_REQUIRE(opt.get_long("size") == 33);
        CATCH_REQUIRE(opt.get_long("size", 0) == 33);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "33");
        CATCH_REQUIRE(opt.size("size") == 0);

        // help parameter
        CATCH_REQUIRE(opt.get_option("options-help") != nullptr);
        CATCH_REQUIRE(opt.is_defined("options-help"));
        CATCH_REQUIRE(opt.get_string("options-help") == "");
        CATCH_REQUIRE(opt.get_string("options-help", 0) == "");
        CATCH_REQUIRE(opt["options-help"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("options-help"));
        CATCH_REQUIRE(opt.get_default("options-help").empty());
        CATCH_REQUIRE(opt.size("options-help") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_HELP);
        CATCH_REQUIRE_LONG_STRING(ss.str(),
"Usage: test system commands\n"
"\n"
"Options:\n"
"   --secret or -S <arg>       even more secret command, hidden by default.\n"
"\n"
"Copyright matters\n"
"\n"
                    );
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_copyright", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --copyright system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("23")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_copyright = "Copyright (c) " BOOST_PP_STRINGIZE(UTC_BUILD_YEAR) "  Made to Order Software Corporation";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--copyright",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "23");
        CATCH_REQUIRE(opt.get_string("size", 0) == "23");
        CATCH_REQUIRE(opt["size"] == "23");
        CATCH_REQUIRE(opt.get_long("size") == 23);
        CATCH_REQUIRE(opt.get_long("size", 0) == 23);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "23");
        CATCH_REQUIRE(opt.size("size") == 0);

        // copyright parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_string("copyright") == "");
        CATCH_REQUIRE(opt.get_string("copyright", 0) == "");
        CATCH_REQUIRE(opt["copyright"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_COPYRIGHT);
        CATCH_REQUIRE(ss.str() == "Copyright (c) " BOOST_PP_STRINGIZE(UTC_BUILD_YEAR) "  Made to Order Software Corporation\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --copyright system flag, without a --copyright on the command line")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("53")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "53");
        CATCH_REQUIRE(opt.size("size") == 1);

        // copyright parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE_FALSE(opt.has_default("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_license", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --license system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("73")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_license = "GPL v2";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--license",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "73");
        CATCH_REQUIRE(opt.get_string("size", 0) == "73");
        CATCH_REQUIRE(opt["size"] == "73");
        CATCH_REQUIRE(opt.get_long("size") == 73);
        CATCH_REQUIRE(opt.get_long("size", 0) == 73);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "73");
        CATCH_REQUIRE(opt.size("size") == 0);

        // license parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') == opt.get_option("license"));
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_string("license") == "");
        CATCH_REQUIRE(opt.get_string("license", 0) == "");
        CATCH_REQUIRE(opt["license"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_LICENSE);
        CATCH_REQUIRE(ss.str() == "GPL v2\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --license system flag, without a --license on the command line")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("103")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "103");
        CATCH_REQUIRE(opt.size("size") == 1);

        // license parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') == opt.get_option("license"));
        CATCH_REQUIRE_FALSE(opt.is_defined("license"));
        CATCH_REQUIRE_FALSE(opt.has_default("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_build_date", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --build-date system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("7301")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--build-date",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "7301");
        CATCH_REQUIRE(opt.get_string("size", 0) == "7301");
        CATCH_REQUIRE(opt["size"] == "7301");
        CATCH_REQUIRE(opt.get_long("size") == 7301);
        CATCH_REQUIRE(opt.get_long("size", 0) == 7301);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "7301");
        CATCH_REQUIRE(opt.size("size") == 0);

        // build-date parameter
        CATCH_REQUIRE(opt.get_option("build-date") != nullptr);
        CATCH_REQUIRE(opt.is_defined("build-date"));
        CATCH_REQUIRE(opt.get_string("build-date") == "");
        CATCH_REQUIRE(opt.get_string("build-date", 0) == "");
        CATCH_REQUIRE(opt["build-date"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("build-date"));
        CATCH_REQUIRE(opt.get_default("build-date").empty());
        CATCH_REQUIRE(opt.size("build-date") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_BUILD_DATE);
        CATCH_REQUIRE(ss.str() == "Built on "
                                + std::string(environment_options.f_build_date)
                                + " at "
                                + environment_options.f_build_time
                                + "\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --build-date system flag, without a --build-date on the command line")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("103")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "103");
        CATCH_REQUIRE(opt.size("size") == 1);

        // build-date parameter
        CATCH_REQUIRE(opt.get_option("build-date") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("build-date"));
        CATCH_REQUIRE_FALSE(opt.has_default("build-date"));
        CATCH_REQUIRE(opt.get_default("build-date").empty());
        CATCH_REQUIRE(opt.size("build-date") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_environment_variable_name", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --environment-variable-name system flag")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("7301")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_environment_variable_name = "ADVGETOPT_OPTIONS";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--environment-variable-name",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "7301");
        CATCH_REQUIRE(opt.get_string("size", 0) == "7301");
        CATCH_REQUIRE(opt["size"] == "7301");
        CATCH_REQUIRE(opt.get_long("size") == 7301);
        CATCH_REQUIRE(opt.get_long("size", 0) == 7301);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "7301");
        CATCH_REQUIRE(opt.size("size") == 0);

        // environment-variable-name parameter
        CATCH_REQUIRE(opt.get_option("environment-variable-name") != nullptr);
        CATCH_REQUIRE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE(opt.get_string("environment-variable-name") == "");
        CATCH_REQUIRE(opt.get_string("environment-variable-name", 0) == "");
        CATCH_REQUIRE(opt["environment-variable-name"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_ENVIRONMENT_VARIABLE_NAME);
        CATCH_REQUIRE(ss.str() == "ADVGETOPT_OPTIONS\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --environment-variable-name system flag with nullptr")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("7301")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_environment_variable_name = nullptr;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--environment-variable-name",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "7301");
        CATCH_REQUIRE(opt.get_string("size", 0) == "7301");
        CATCH_REQUIRE(opt["size"] == "7301");
        CATCH_REQUIRE(opt.get_long("size") == 7301);
        CATCH_REQUIRE(opt.get_long("size", 0) == 7301);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "7301");
        CATCH_REQUIRE(opt.size("size") == 0);

        // environment-variable-name parameter
        CATCH_REQUIRE(opt.get_option("environment-variable-name") != nullptr);
        CATCH_REQUIRE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE(opt.get_string("environment-variable-name") == "");
        CATCH_REQUIRE(opt.get_string("environment-variable-name", 0) == "");
        CATCH_REQUIRE(opt["environment-variable-name"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_ENVIRONMENT_VARIABLE_NAME);
        CATCH_REQUIRE(ss.str() == "unittest does not support an environment variable.\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --environment-variable-name system flag with \"\"")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("7301")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_environment_variable_name = "";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--environment-variable-name",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "7301");
        CATCH_REQUIRE(opt.get_string("size", 0) == "7301");
        CATCH_REQUIRE(opt["size"] == "7301");
        CATCH_REQUIRE(opt.get_long("size") == 7301);
        CATCH_REQUIRE(opt.get_long("size", 0) == 7301);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "7301");
        CATCH_REQUIRE(opt.size("size") == 0);

        // environment-variable-name parameter
        CATCH_REQUIRE(opt.get_option("environment-variable-name") != nullptr);
        CATCH_REQUIRE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE(opt.get_string("environment-variable-name") == "");
        CATCH_REQUIRE(opt.get_string("environment-variable-name", 0) == "");
        CATCH_REQUIRE(opt["environment-variable-name"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_ENVIRONMENT_VARIABLE_NAME);
        CATCH_REQUIRE(ss.str() == "unittest does not support an environment variable.\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --environment-variable-name system flag, without a --environment-variable-name on the command line")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("103")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "103");
        CATCH_REQUIRE(opt.size("size") == 1);

        // environment-variable-name parameter
        CATCH_REQUIRE(opt.get_option("environment-variable-name") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE_FALSE(opt.has_default("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_configuration_filenames", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --configuration-filenames system flag")
    {
        snapdev::safe_setenv env("HOME", "/home/advgetopt");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("3101")
            ),
            advgetopt::end_options()
        };

        char const * confs[] =
        {
            ".config/file.mdi",
            "/etc/snapwebsites/server.conf",
            "~/.config/advgetopt/snap.conf",
            nullptr
        };
        char const * dirs[] =
        {
            ".config",
            "/etc/secret",
            "~/.config/snapwebsites",
            nullptr
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_files = confs;
        environment_options.f_configuration_filename = "snapdb.conf";
        environment_options.f_configuration_directories = dirs;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--configuration-filenames",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "3101");
        CATCH_REQUIRE(opt.get_string("size", 0) == "3101");
        CATCH_REQUIRE(opt["size"] == "3101");
        CATCH_REQUIRE(opt.get_long("size") == 3101);
        CATCH_REQUIRE(opt.get_long("size", 0) == 3101);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "3101");
        CATCH_REQUIRE(opt.size("size") == 0);

        // configuration-filenames parameter
        CATCH_REQUIRE(opt.get_option("configuration-filenames") != nullptr);
        CATCH_REQUIRE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE(opt.get_string("configuration-filenames") == "");
        CATCH_REQUIRE(opt.get_string("configuration-filenames", 0) == "");
        CATCH_REQUIRE(opt["configuration-filenames"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_CONFIGURATION_FILENAMES);
        CATCH_REQUIRE(getenv("HOME") != nullptr);
        std::string const home(getenv("HOME"));
        CATCH_REQUIRE(ss.str() ==
"Configuration filenames:\n"
" . .config/file.mdi\n"
" . .config/unittest.d/50-file.mdi\n"
" . /etc/snapwebsites/server.conf\n"
" . /etc/snapwebsites/unittest.d/50-server.conf\n"
" . " + home + "/.config/advgetopt/snap.conf\n"
" . .config/snapdb.conf\n"
" . .config/unittest.d/50-snapdb.conf\n"
" . /etc/secret/snapdb.conf\n"
" . /etc/secret/unittest.d/50-snapdb.conf\n"
" . " + home + "/.config/snapwebsites/snapdb.conf\n"
);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --configuration-filenames system flag with --config-dir too")
    {
        snapdev::safe_setenv env("HOME", "/home/advgetopt");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("3101")
            ),
            advgetopt::end_options()
        };

        char const * confs[] =
        {
            ".config/file.mdi",
            "/etc/snapwebsites/server.conf",
            "~/.config/advgetopt/snap.conf",
            nullptr
        };
        char const * dirs[] =
        {
            ".config",
            "/etc/secret",
            "~/.config/snapwebsites",
            nullptr
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_files = confs;
        environment_options.f_configuration_filename = "snapdb.conf";
        environment_options.f_configuration_directories = dirs;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--config-dir",
            "/var/lib/advgetopt",
            "--configuration-filenames",
            "--config-dir",
            "/opt/config",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "3101");
        CATCH_REQUIRE(opt.get_string("size", 0) == "3101");
        CATCH_REQUIRE(opt["size"] == "3101");
        CATCH_REQUIRE(opt.get_long("size") == 3101);
        CATCH_REQUIRE(opt.get_long("size", 0) == 3101);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "3101");
        CATCH_REQUIRE(opt.size("size") == 0);

        // configuration-filenames parameter
        CATCH_REQUIRE(opt.get_option("configuration-filenames") != nullptr);
        CATCH_REQUIRE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE(opt.get_string("configuration-filenames") == "");
        CATCH_REQUIRE(opt.get_string("configuration-filenames", 0) == "");
        CATCH_REQUIRE(opt["configuration-filenames"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == (advgetopt::SYSTEM_OPTION_CONFIGURATION_FILENAMES
                               | advgetopt::SYSTEM_OPTION_CONFIG_DIR));
        CATCH_REQUIRE(getenv("HOME") != nullptr);
        std::string const home(getenv("HOME"));
        CATCH_REQUIRE(ss.str() ==
"Configuration filenames:\n"
" . .config/file.mdi\n"
" . .config/unittest.d/50-file.mdi\n"
" . /etc/snapwebsites/server.conf\n"
" . /etc/snapwebsites/unittest.d/50-server.conf\n"
" . " + home + "/.config/advgetopt/snap.conf\n"
" . /var/lib/advgetopt/snapdb.conf\n"
" . /var/lib/advgetopt/unittest.d/50-snapdb.conf\n"
" . /opt/config/snapdb.conf\n"
" . /opt/config/unittest.d/50-snapdb.conf\n"
" . .config/snapdb.conf\n"
" . .config/unittest.d/50-snapdb.conf\n"
" . /etc/secret/snapdb.conf\n"
" . /etc/secret/unittest.d/50-snapdb.conf\n"
" . " + home + "/.config/snapwebsites/snapdb.conf\n"
);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --configuration-filenames system flag without any configuration files")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("3101")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--configuration-filenames",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "3101");
        CATCH_REQUIRE(opt.get_string("size", 0) == "3101");
        CATCH_REQUIRE(opt["size"] == "3101");
        CATCH_REQUIRE(opt.get_long("size") == 3101);
        CATCH_REQUIRE(opt.get_long("size", 0) == 3101);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "3101");
        CATCH_REQUIRE(opt.size("size") == 0);

        // configuration-filenames parameter
        CATCH_REQUIRE(opt.get_option("configuration-filenames") != nullptr);
        CATCH_REQUIRE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE(opt.get_string("configuration-filenames") == "");
        CATCH_REQUIRE(opt.get_string("configuration-filenames", 0) == "");
        CATCH_REQUIRE(opt["configuration-filenames"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_CONFIGURATION_FILENAMES);
        CATCH_REQUIRE(ss.str() == "unittest does not support configuration files.\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --configuration-filenames system flag, without a --configuration-filenames on the command line")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("193")
            ),
            advgetopt::end_options()
        };

        char const * confs[] =
        {
            ".config/file.mdi",
            "/etc/snapwebsites/server.conf",
            "~/.config/advgetopt/snap.conf",
            nullptr
        };
        char const * dirs[] =
        {
            ".config",
            "/etc/secret",
            "~/.config/snapwebsites",
            nullptr
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";
        environment_options.f_configuration_files = confs;
        environment_options.f_configuration_filename = "snapdb.conf";
        environment_options.f_configuration_directories = dirs;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1221",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1221");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1221");
        CATCH_REQUIRE(opt["size"] == "1221");
        CATCH_REQUIRE(opt.get_long("size") == 1221);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1221);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "193");
        CATCH_REQUIRE(opt.size("size") == 1);

        // configuration-filenames parameter
        CATCH_REQUIRE(opt.get_option("configuration-filenames") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE_FALSE(opt.has_default("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_flags_path_to_option_definitions", "[arguments][valid][getopt][system_flags]")
{
    CATCH_START_SECTION("Check with the --path-to-option-definitions system flag (Default)")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("7301")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--path-to-option-definitions",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "7301");
        CATCH_REQUIRE(opt.get_string("size", 0) == "7301");
        CATCH_REQUIRE(opt["size"] == "7301");
        CATCH_REQUIRE(opt.get_long("size") == 7301);
        CATCH_REQUIRE(opt.get_long("size", 0) == 7301);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "7301");
        CATCH_REQUIRE(opt.size("size") == 0);

        // path-to-option-definitions parameter
        CATCH_REQUIRE(opt.get_option("path-to-option-definitions") != nullptr);
        CATCH_REQUIRE(opt.is_defined("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_string("path-to-option-definitions") == "");
        CATCH_REQUIRE(opt.get_string("path-to-option-definitions", 0) == "");
        CATCH_REQUIRE(opt["path-to-option-definitions"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_default("path-to-option-definitions").empty());
        CATCH_REQUIRE(opt.size("path-to-option-definitions") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_PATH_TO_OPTION_DEFINITIONS);
        CATCH_REQUIRE(ss.str() == "/usr/share/advgetopt/options/\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --path-to-option-definitions system flag (Specified)")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("7301")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_options_files_directory = "/opt/advgetopt/configs";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--path-to-option-definitions",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "7301");
        CATCH_REQUIRE(opt.get_string("size", 0) == "7301");
        CATCH_REQUIRE(opt["size"] == "7301");
        CATCH_REQUIRE(opt.get_long("size") == 7301);
        CATCH_REQUIRE(opt.get_long("size", 0) == 7301);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "7301");
        CATCH_REQUIRE(opt.size("size") == 0);

        // path-to-option-definitions parameter
        CATCH_REQUIRE(opt.get_option("path-to-option-definitions") != nullptr);
        CATCH_REQUIRE(opt.is_defined("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_string("path-to-option-definitions") == "");
        CATCH_REQUIRE(opt.get_string("path-to-option-definitions", 0) == "");
        CATCH_REQUIRE(opt["path-to-option-definitions"] == "");
        CATCH_REQUIRE_FALSE(opt.has_default("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_default("path-to-option-definitions").empty());
        CATCH_REQUIRE(opt.size("path-to-option-definitions") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_PATH_TO_OPTION_DEFINITIONS);
        CATCH_REQUIRE(ss.str() == "/opt/advgetopt/configs/\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with the --path-to-option-definitions system flag, without a --path-to-option-definitions on the command line")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("303")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test system commands";
        environment_options.f_help_footer = "Copyright matters";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "1919",
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

        // valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') == opt.get_option("size"));
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "1919");
        CATCH_REQUIRE(opt.get_string("size", 0) == "1919");
        CATCH_REQUIRE(opt["size"] == "1919");
        CATCH_REQUIRE(opt.get_long("size") == 1919);
        CATCH_REQUIRE(opt.get_long("size", 0) == 1919);
        CATCH_REQUIRE(opt.has_default("size"));
        CATCH_REQUIRE(opt.get_default("size") == "303");
        CATCH_REQUIRE(opt.size("size") == 1);

        // environment-variable-name parameter
        CATCH_REQUIRE(opt.get_option("path-to-option-definitions") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("path-to-option-definitions"));
        CATCH_REQUIRE_FALSE(opt.has_default("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_default("path-to-option-definitions").empty());
        CATCH_REQUIRE(opt.size("path-to-option-definitions") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == advgetopt::SYSTEM_OPTION_NONE);
        CATCH_REQUIRE(ss.str().empty());
    CATCH_END_SECTION()
}








CATCH_TEST_CASE("invalid_option_name", "[arguments][invalid][getopt]")
{
    CATCH_START_SECTION("Verify that asking for the string of a non-existant option fails")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test get_string() functions";

        advgetopt::getopt opt(environment_options);

        char const * cargv[] =
        {
            "tests/options-parser",
            "--license",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        opt.finish_parsing(argc, argv);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("non-existant")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("non-existant", 0)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("non-existant", 1)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: there is no --non-existant option defined."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that asking for the long of a non-existant option fails")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test get_string() functions";

        advgetopt::getopt opt(environment_options);

        char const * cargv[] =
        {
            "tests/options-parser",
            "--license",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        opt.finish_parsing(argc, argv);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("non-existant")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("non-existant", 0)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: there is no --non-existant option defined."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("non-existant", 1)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: there is no --non-existant option defined."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that asking for a default with an empty string fails")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: test get_default() functions";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.has_default("")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.has_default(std::string())
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_default("")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_default(std::string())
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("[] operators want a valid name")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test get_default() functions";

        advgetopt::getopt opt(environment_options);

        char const * cargv[] =
        {
            "tests/options-parser",
            "--license",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        opt.finish_parsing(argc, argv);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt[""]
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt[std::string()]
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt["g"]
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be one letter if it does not exist in operator []."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt[std::string("g")]
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be one letter if it does not exist in operator []."));

        advgetopt::getopt const & const_opt(opt);

        CATCH_REQUIRE_THROWS_MATCHES(
                  const_opt[""]
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  const_opt[std::string()]
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argument name cannot be empty."));
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("missing_default_value", "[arguments][invalid][getopt]")
{
    CATCH_START_SECTION("Verify a string value without arguments and no default")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            , advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
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
        CATCH_REQUIRE(static_cast<advgetopt::getopt const &>(opt)["size"].empty());

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size")
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size", 0)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size", 1)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no default."));

        // these do not create an entry (even though it looks like it,
        // i.e. it would for an std::map)
        //
        CATCH_REQUIRE(opt["size"].empty());
        CATCH_REQUIRE(opt["size"].length() == 0);
        CATCH_REQUIRE(opt["size"].size() == 0);

        CATCH_REQUIRE(opt.size("size") == 0);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size", 0)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size", 1)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no default."));

        // now this one does create a value
        //
        opt["size"] = "45.3";

        CATCH_REQUIRE(opt.get_string("size") == "45.3");
        CATCH_REQUIRE(opt.get_string("size", 0) == "45.3");

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_string("size", 1)
                , advgetopt::getopt_undefined
                , Catch::Matchers::ExceptionMessage(
                              "getopt_exception: option_info::get_value(): no value at index 1 (idx >= 1) for --size so you can't get this value."));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify an integer (long) value without arguments and no default")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            , advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
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
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 0)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 1)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no or an empty default."));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify an integer (long) value without arguments and an empty string as default")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            , advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
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
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 0)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no or an empty default."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 1)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: the --size option was not defined on the command line and it has no or an empty default."));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
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
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: invalid default number \"undefined\" for option --size"));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 0)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: invalid default number \"undefined\" for option --size"));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.get_long("size", 1)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: invalid default number \"undefined\" for option --size"));

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
        CATCH_REQUIRE(opt["size"] == "312");
        CATCH_REQUIRE(opt.get_long("size") == 312);
        CATCH_REQUIRE(opt.get_long("size", 0) == 312);
        CATCH_REQUIRE(opt.get_default("size") == "-300");
        CATCH_REQUIRE(opt.size("size") == 1);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: 312 is out of bounds (-100..100 inclusive) in parameter --size.");
        CATCH_REQUIRE(opt.get_long("size", 0, -100, 100) == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("check_sanitizer", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Check that the sanitizer is detected")
    {
        // when compiling the tests with coverage we turn on the sanitizer
        // so here we should get output that show the sanitizer as being
        // turned on; unfortunately, we can't test all cases in our current
        // situation
        //
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
        // when running coverage or in the Sanitize version
        //
        std::string const expected(
#ifdef __SANITIZE_ADDRESS__
                    "The address sanitizer is compiled in.\n"
#endif
#ifdef __SANITIZE_THREAD__
                    "The thread sanitizer is compiled in.\n"
#endif
            );
        CATCH_REQUIRE(advgetopt::getopt::sanitizer_details() == expected);
#else
        // when running in Debug or Release
        //
        CATCH_REQUIRE(advgetopt::getopt::sanitizer_details() == "The address and thread sanitizers are not compiled in.\n");
#endif
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
