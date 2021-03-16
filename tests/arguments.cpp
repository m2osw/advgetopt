/*
 * License:
 *    Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved
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
#include <snapdev/ostream_to_buf.h>
#include <snapdev/safe_setenv.h>

// C++ lib
//
#include <fstream>
#include <sstream>












CATCH_TEST_CASE("flag_argument", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Verify a simple --verbose argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --verbose option";
        environment_options.f_environment_variable_name = "ADVGETOPT_UNDEFINED_VARIABLE_TO_TEST_THE_NULL";

        CATCH_WHEN("using long form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--verbose",
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
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-v",
                nullptr
            };
            int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
            char ** argv = const_cast<char **>(cargv);

            advgetopt::getopt opt(environment_options, argc, argv);

            // check that the result is valid

            // an invalid parameter, MUST NOT EXIST
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE(opt.get_option('Z', true) == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // no default
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.get_option('v', true) == opt.get_option('v'));
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple --verbose argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --verbose option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("using long form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

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
            CATCH_REQUIRE(opt.get_option('Z', true) == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // no default
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.get_option('v', true) == opt.get_option('v'));
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-v");

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
            CATCH_REQUIRE(opt.get_option('Z', true) == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // no default
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.get_option('v', true) == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple alias argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbosity")
                , advgetopt::ShortName('n')
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Alias("verbose")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --verbosity option";

        CATCH_WHEN("using long form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--verbosity",
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
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("verbosity") != nullptr);
            CATCH_REQUIRE(opt.get_option("verbosity") == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.is_defined("verbosity"));
            CATCH_REQUIRE(opt.get_default("verbosity").empty());
            CATCH_REQUIRE(opt.size("verbosity") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-n",
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
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("verbosity") != nullptr);
            CATCH_REQUIRE(opt.get_option("verbosity") == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.is_defined("verbosity"));
            CATCH_REQUIRE(opt.get_option('n') == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.get_option('n', true) == opt.get_option("verbosity", true));
            CATCH_REQUIRE(opt.get_option('n', true) != opt.get_option("verbose"));
            CATCH_REQUIRE(opt.get_default("verbosity").empty());
            CATCH_REQUIRE(opt.size("verbosity") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple alias argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbosity")
                , advgetopt::ShortName('n')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Alias("verbose")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --verbosity option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("using long form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbosity");

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
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("verbosity") != nullptr);
            CATCH_REQUIRE(opt.get_option("verbosity") == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.is_defined("verbosity"));
            CATCH_REQUIRE(opt.get_default("verbosity").empty());
            CATCH_REQUIRE(opt.size("verbosity") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-n");

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
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("verbosity") != nullptr);
            CATCH_REQUIRE(opt.get_option("verbosity") == opt.get_option("verbose"));
            CATCH_REQUIRE(opt.is_defined("verbosity"));
            CATCH_REQUIRE(opt.get_default("verbosity").empty());
            CATCH_REQUIRE(opt.size("verbosity") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("require_argument", "[arguments][valid][required][getopt]")
{
    CATCH_START_SECTION("Verify a simple --out <filename> argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test simple --out <filename> option";

        CATCH_WHEN("using long form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the license system parameter
            CATCH_REQUIRE(opt.get_option("license") != nullptr);
            CATCH_REQUIRE(opt.get_option('L') != nullptr);
            CATCH_REQUIRE(opt.is_defined("license"));
            CATCH_REQUIRE(opt.get_default("license").empty());
            CATCH_REQUIRE(opt.size("license") == 1);

            // the copyright system parameter
            CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
            CATCH_REQUIRE(opt.get_option('C') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
            CATCH_REQUIRE(opt.get_default("copyright").empty());
            CATCH_REQUIRE(opt.size("copyright") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form with an equal sign")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out=my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-o",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple alias of --out <filename> argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("results")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("out")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --results <filename> option";

        CATCH_WHEN("using long form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--results",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("results") != nullptr);
            CATCH_REQUIRE(opt.get_option("results") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.get_string("results") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("results").empty());
            CATCH_REQUIRE(opt.size("results") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form with an equal sign")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--results=my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("results") != nullptr);
            CATCH_REQUIRE(opt.get_option("results") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.get_string("results") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("results").empty());
            CATCH_REQUIRE(opt.size("results") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-r",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("results") != nullptr);
            CATCH_REQUIRE(opt.get_option("results") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.get_string("results") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("results").empty());
            CATCH_REQUIRE(opt.size("results") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple --out <filename> argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out_dir")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out_dir <filename> option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("using long form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--out_dir my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out-dir") != nullptr);
            CATCH_REQUIRE(opt.get_option("out_dir") != nullptr);
            CATCH_REQUIRE(opt.get_option("out_dir") == opt.get_option("out-dir"));
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out-dir"));
            CATCH_REQUIRE(opt.get_string("out-dir") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out-dir", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out-dir").empty());
            CATCH_REQUIRE(opt.size("out-dir") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form with an equal sign")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--out_dir=my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out_dir") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out_dir"));
            CATCH_REQUIRE(opt.get_string("out_dir") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out_dir", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out_dir").empty());
            CATCH_REQUIRE(opt.size("out_dir") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-o my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out_dir") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out-dir"));
            CATCH_REQUIRE(opt.get_string("out_dir") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out-dir", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out_dir").empty());
            CATCH_REQUIRE(opt.size("out-dir") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple alias of --out <filename> argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("results")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("o")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple alias of --out <filename> option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("using long form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--results my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("results") != nullptr);
            CATCH_REQUIRE(opt.get_option("results") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.get_string("results") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("results").empty());
            CATCH_REQUIRE(opt.size("results") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-r my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("results") != nullptr);
            CATCH_REQUIRE(opt.get_option("results") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.get_string("results") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("results").empty());
            CATCH_REQUIRE(opt.size("results") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("require_arguments", "[arguments][valid][required][multiple][getopt]")
{
    CATCH_START_SECTION("Verify a simple --out <filename> ... argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out <filename> ... option";

        CATCH_WHEN("using long form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-o",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple alias of --out <filename> ... argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("results")
                , advgetopt::ShortName('r')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Alias("out")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple alias of --out <filename> ... option";

        CATCH_WHEN("using long form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--results",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("results") != nullptr);
            CATCH_REQUIRE(opt.get_option("results") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.get_string("results") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("results", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("results").empty());
            CATCH_REQUIRE(opt.size("results") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-r",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // the alias parameter
            CATCH_REQUIRE(opt.get_option("results") != nullptr);
            CATCH_REQUIRE(opt.get_option("results") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("results"));
            CATCH_REQUIRE(opt.get_string("results") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("results", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("results", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("results").empty());
            CATCH_REQUIRE(opt.size("results") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple --out <filename> ... argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out <filename> ... option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("using long form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--out \"my filename.out\" another.out last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-o 'my filename.out' another.out last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("optional_arguments", "[arguments][valid][multiple][getopt]")
{
    CATCH_START_SECTION("Verify a simple --out [<filename> ...] argument")
        char const * const separators[] =
        {
            ",",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
                , advgetopt::Separators(separators)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out <filename> ... option";

        CATCH_WHEN("using long form and no arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out").empty());
            CATCH_REQUIRE(opt.get_string("out", 0).empty());
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form and one empty argument and an equal sign")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out=",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out").empty());
            CATCH_REQUIRE(opt.get_string("out", 0).empty());
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form and one argument")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form and one argument and an equal sign")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out=my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form and three arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form and three arguments and an equal sign")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--out=my-filename.out,another.out,last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form and no arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-o",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out").empty());
            CATCH_REQUIRE(opt.get_string("out", 0).empty());
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form and one argument")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-o",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form and three arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "-o",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple --out [<filename> ...] argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out <filename> ... option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("using long form and no arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out").empty());
            CATCH_REQUIRE(opt.get_string("out", 0).empty());
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form and one argument")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--out my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using long form and three arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--out my-filename.out another.out last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form and no arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-o");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out").empty());
            CATCH_REQUIRE(opt.get_string("out", 0).empty());
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form and one argument")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-o my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using short form and three arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-o my-filename.out another.out last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("config_dir_argument", "[arguments][valid][getopt][config]")
{
    CATCH_START_SECTION("Verify that we do get the --config-dir option when we have a standalone configuration filename")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = "snapwatchdog.conf";
        environment_options.f_help_header = "Usage: test --config-dir";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--out",
            "my-filename.out",
            "--license",
            "--config-dir",
            "/opt/m2osw/config",
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

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("out") != nullptr);
        CATCH_REQUIRE(opt.get_option('o') == opt.get_option("out"));
        CATCH_REQUIRE(opt.is_defined("out"));
        CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
        CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
        CATCH_REQUIRE(opt.get_default("out").empty());
        CATCH_REQUIRE(opt.size("out") == 1);

        // the license system parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // the copyright system parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // the config-dir system parameter
        CATCH_REQUIRE(opt.get_option("config-dir") != nullptr);
        CATCH_REQUIRE(opt.is_defined("config-dir"));
        CATCH_REQUIRE(opt.get_default("config-dir").empty());
        CATCH_REQUIRE(opt.size("config-dir") == 1);
        CATCH_REQUIRE(opt.get_string("config-dir") == "/opt/m2osw/config");

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that we do not get the --config-dir option when the standalone configuration filename is nullptr")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = nullptr;
        environment_options.f_help_header = "Usage: test --config-dir";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--out",
            "my-filename.out",
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

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("out") != nullptr);
        CATCH_REQUIRE(opt.get_option('o') == opt.get_option("out"));
        CATCH_REQUIRE(opt.is_defined("out"));
        CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
        CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
        CATCH_REQUIRE(opt.get_default("out").empty());
        CATCH_REQUIRE(opt.size("out") == 1);

        // the license system parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // the copyright system parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // no config-dir system parameter when the configuration filename is missing
        CATCH_REQUIRE(opt.get_option("config-dir") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("config-dir"));
        CATCH_REQUIRE(opt.get_default("config-dir").empty());
        CATCH_REQUIRE(opt.size("config-dir") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that we do not get the --config-dir option when the standalone configuration filename is \"\"")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = "";
        environment_options.f_help_header = "Usage: test --config-dir";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--out",
            "my-filename.out",
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

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("out") != nullptr);
        CATCH_REQUIRE(opt.get_option('o') == opt.get_option("out"));
        CATCH_REQUIRE(opt.is_defined("out"));
        CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
        CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
        CATCH_REQUIRE(opt.get_default("out").empty());
        CATCH_REQUIRE(opt.size("out") == 1);

        // the license system parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // the copyright system parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // no config-dir system parameter when the configuration filename is missing
        CATCH_REQUIRE(opt.get_option("config-dir") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("config-dir"));
        CATCH_REQUIRE(opt.get_default("config-dir").empty());
        CATCH_REQUIRE(opt.size("config-dir") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Test adding '-c' to '--config-dir'")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = "snapwatchdog.conf";
        environment_options.f_help_header = "Usage: test --config-dir";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--out",
            "my-filename.out",
            "--license",
            "-c",
            "/opt/m2osw/config",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);
        opt.parse_program_name(argv);
        opt.link_aliases();

        advgetopt::option_info::pointer_t config_dir(opt.get_option("config-dir"));
        CATCH_REQUIRE(config_dir != nullptr);
        opt.set_short_name("config-dir", U'c');

        opt.parse_arguments(argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("out") != nullptr);
        CATCH_REQUIRE(opt.get_option('o') == opt.get_option("out"));
        CATCH_REQUIRE(opt.is_defined("out"));
        CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
        CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
        CATCH_REQUIRE(opt.get_default("out").empty());
        CATCH_REQUIRE(opt.size("out") == 1);

        // the license system parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // the copyright system parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // the config-dir system parameter
        CATCH_REQUIRE(opt.get_option("config-dir") != nullptr);
        CATCH_REQUIRE(opt.is_defined("config-dir"));
        CATCH_REQUIRE(opt.get_default("config-dir").empty());
        CATCH_REQUIRE(opt.size("config-dir") == 1);
        CATCH_REQUIRE(opt.get_string("config-dir") == "/opt/m2osw/config");

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Test our own parsing with '--config-dir' and f_configuration_filename set to nullptr")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = nullptr;
        environment_options.f_help_header = "Usage: test --config-dir";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--out",
            "my-filename.out",
            "--license",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);
        opt.parse_program_name(argv);
        opt.link_aliases();

        CATCH_REQUIRE(opt.get_option("config-dir") == nullptr);

        opt.parse_arguments(argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("out") != nullptr);
        CATCH_REQUIRE(opt.get_option('o') == opt.get_option("out"));
        CATCH_REQUIRE(opt.is_defined("out"));
        CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
        CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
        CATCH_REQUIRE(opt.get_default("out").empty());
        CATCH_REQUIRE(opt.size("out") == 1);

        // the license system parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // the copyright system parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // the config-dir system parameter
        CATCH_REQUIRE(opt.get_option("config-dir") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("config-dir"));
        CATCH_REQUIRE(opt.get_default("config-dir").empty());
        CATCH_REQUIRE(opt.size("config-dir") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Test our own parsing with '--config-dir' and f_configuration_filename set to \"\"")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_configuration_filename = "";
        environment_options.f_help_header = "Usage: test --config-dir";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--out",
            "my-filename.out",
            "--license",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);
        opt.parse_program_name(argv);
        opt.link_aliases();

        CATCH_REQUIRE(opt.get_option("config-dir") == nullptr);

        opt.parse_arguments(argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("out") != nullptr);
        CATCH_REQUIRE(opt.get_option('o') == opt.get_option("out"));
        CATCH_REQUIRE(opt.is_defined("out"));
        CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
        CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
        CATCH_REQUIRE(opt.get_default("out").empty());
        CATCH_REQUIRE(opt.size("out") == 1);

        // the license system parameter
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // the copyright system parameter
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == opt.get_option("copyright"));
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // the config-dir system parameter
        CATCH_REQUIRE(opt.get_option("config-dir") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("config-dir"));
        CATCH_REQUIRE(opt.get_default("config-dir").empty());
        CATCH_REQUIRE(opt.size("config-dir") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("default_argument", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Verify a simple [<filename>] argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple [<filename>] option";

        CATCH_WHEN("no arguments")
        {
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 0);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("one argument")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("three arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "last.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "last.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using the '--' separator and no arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 0);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using the '--' separator and one argument")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--",
                "--my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "--my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "--my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "--my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "--my-filename.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using the '--' separator and three arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--",
                "my-filename.out",
                "-another.out",
                "--last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "--last.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "--last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "--last.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "--last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple [<filename>] argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE
                                                      , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out <filename> ... option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("no arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 0);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("one argument")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("three arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "my-filename.out another.out last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "last.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "last.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("four arguments with a '-'")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "my-filename.out another.out - last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "last.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "last.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using the '--' separator and no arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 0);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using the '--' separator and one argument")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-- my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using the '--' separator and three arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-- my-filename.out -another.out --last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "--last.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "--last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "--last.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "--last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("using the '--' separator and four arguments with a '-'")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-- my-filename.out -another.out - --last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "--last.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "--last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter is the default
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.get_option("--") == opt.get_option("out"));
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "--last.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "--last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that we can have a non-require argument with an invalid default")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("before")
                , advgetopt::ShortName('b')
                , advgetopt::Flags(advgetopt::command_flags<>())
                , advgetopt::Help("appears before.")
                , advgetopt::DefaultValue("early")
            ),
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::command_flags<>())
                , advgetopt::Help("output filename.")
                , advgetopt::DefaultValue("default-name")
                , advgetopt::Validator("/[a-z]+/")
            ),
            advgetopt::define_option(
                  advgetopt::Name("after")
                , advgetopt::ShortName('a')
                , advgetopt::Flags(advgetopt::command_flags<>())
                , advgetopt::Help("appears after.")
                , advgetopt::DefaultValue("late")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out <filename> ... option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("no parameters to arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--before",
                "--out",
                "--after",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("before") != nullptr);
            CATCH_REQUIRE(opt.get_option('b') != nullptr);
            CATCH_REQUIRE(opt.is_defined("before"));
            CATCH_REQUIRE(opt.get_string("before").empty());
            CATCH_REQUIRE(opt.get_string("before", 0).empty());
            CATCH_REQUIRE(opt.get_default("before") == "early");
            CATCH_REQUIRE(opt.size("before") == 1);

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out").empty());
            CATCH_REQUIRE(opt.get_string("out", 0).empty());
            CATCH_REQUIRE(opt.get_default("out") == "default-name");
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("after") != nullptr);
            CATCH_REQUIRE(opt.get_option('a') != nullptr);
            CATCH_REQUIRE(opt.is_defined("after"));
            CATCH_REQUIRE(opt.get_string("after").empty());
            CATCH_REQUIRE(opt.get_string("after", 0).empty());
            CATCH_REQUIRE(opt.get_default("after") == "late");
            CATCH_REQUIRE(opt.size("after") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("parameters to some arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "--before",
                "avant",
                "--out",
                "--after",
                "apres",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("before") != nullptr);
            CATCH_REQUIRE(opt.get_option('b') != nullptr);
            CATCH_REQUIRE(opt.is_defined("before"));
            CATCH_REQUIRE(opt.get_string("before") == "avant");
            CATCH_REQUIRE(opt.get_string("before", 0) == "avant");
            CATCH_REQUIRE(opt.get_default("before") == "early");
            CATCH_REQUIRE(opt.size("before") == 1);

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out").empty());
            CATCH_REQUIRE(opt.get_string("out", 0).empty());
            CATCH_REQUIRE(opt.get_default("out") == "default-name");
            CATCH_REQUIRE(opt.size("out") == 1);

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("after") != nullptr);
            CATCH_REQUIRE(opt.get_option('a') != nullptr);
            CATCH_REQUIRE(opt.is_defined("after"));
            CATCH_REQUIRE(opt.get_string("after") == "apres");
            CATCH_REQUIRE(opt.get_string("after", 0) == "apres");
            CATCH_REQUIRE(opt.get_default("after") == "late");
            CATCH_REQUIRE(opt.size("after") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("default_arguments", "[arguments][valid][multiple][getopt]")
{
    CATCH_START_SECTION("Verify a simple [<filename> ...] argument")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_DEFAULT_OPTION
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple [<filename>] option";

        CATCH_WHEN("no arguments")
        {
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("one argument")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("three arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("four arguments with a '-'")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
                "another.out",
                "-",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "-");
            CATCH_REQUIRE(opt.get_string("out", 3) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 4);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple [<filename> ...] argument with the default name (a.k.a. \"--\")")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_DEFAULT_OPTION
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple [<filename>] option";

        CATCH_WHEN("no arguments")
        {
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("one argument")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("three arguments")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
                "another.out",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("--", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("four arguments with a '-'")
        {
            char const * cargv[] =
            {
                "/usr/bin/arguments",
                "my-filename.out",
                "another.out",
                "-",
                "last.out",
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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("--", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("--", 2) == "-");
            CATCH_REQUIRE(opt.get_string("--", 3) == "last.out");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 4);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a simple [<filename> ...] argument in a variable")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("out")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE
                                                      , advgetopt::GETOPT_FLAG_DEFAULT_OPTION
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("output filename.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test simple --out <filename> ... option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        CATCH_WHEN("no arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 0);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("one argument")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "my-filename.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("three arguments")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "my-filename.out another.out last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 3);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }

        CATCH_WHEN("four arguments with a '-'")
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "my-filename.out another.out - last.out");

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

            // the valid parameter
            CATCH_REQUIRE(opt.get_option("out") != nullptr);
            CATCH_REQUIRE(opt.get_option('o') != nullptr);
            CATCH_REQUIRE(opt.is_defined("out"));
            CATCH_REQUIRE(opt.get_string("out") == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 0) == "my-filename.out");
            CATCH_REQUIRE(opt.get_string("out", 1) == "another.out");
            CATCH_REQUIRE(opt.get_string("out", 2) == "-");
            CATCH_REQUIRE(opt.get_string("out", 3) == "last.out");
            CATCH_REQUIRE(opt.get_default("out").empty());
            CATCH_REQUIRE(opt.size("out") == 4);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("manual_arguments", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Verify a few arguments added manually")

        // create a getopt object
        //
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_help_header = "Usage: test simple --verbose option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        advgetopt::getopt opt(environment_options);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") == nullptr);
            CATCH_REQUIRE(opt.get_option('v') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") == nullptr);
            CATCH_REQUIRE(opt.get_option('h') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name().empty());
            CATCH_REQUIRE(opt.get_program_fullname().empty());

        // setup the program name early
        //
        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "--help",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        opt.parse_program_name(argv);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") == nullptr);
            CATCH_REQUIRE(opt.get_option('v') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") == nullptr);
            CATCH_REQUIRE(opt.get_option('h') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // verify nullptr has no effect
        //
        opt.parse_options_info(nullptr);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") == nullptr);
            CATCH_REQUIRE(opt.get_option('v') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") == nullptr);
            CATCH_REQUIRE(opt.get_option('h') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // add --verbose
        //
        advgetopt::option const verbose_option[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };
        opt.parse_options_info(verbose_option);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") == nullptr);
            CATCH_REQUIRE(opt.get_option('h') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // add --help
        //
        advgetopt::option const help_option[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("help")
                , advgetopt::ShortName('h')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print this help screen.")
            ),
            advgetopt::end_options()
        };
        opt.parse_options_info(help_option);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // add -- (default option)
        //
        advgetopt::option const default_option[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("filenames to handle by this command.")
            ),
            advgetopt::end_options()
        };
        opt.parse_options_info(default_option);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // first parse the environment variable
        //
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "file.exe file.so file.dll file.lib file.a");
        opt.parse_environment_variable();

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 0) == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 1) == "file.so");
            CATCH_REQUIRE(opt.get_string("--", 2) == "file.dll");
            CATCH_REQUIRE(opt.get_string("--", 3) == "file.lib");
            CATCH_REQUIRE(opt.get_string("--", 4) == "file.a");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 5);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // now parse our command line arguments
        //
        opt.parse_arguments(argc, argv);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 1);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 0) == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 1) == "file.so");
            CATCH_REQUIRE(opt.get_string("--", 2) == "file.dll");
            CATCH_REQUIRE(opt.get_string("--", 3) == "file.lib");
            CATCH_REQUIRE(opt.get_string("--", 4) == "file.a");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 5);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify a few arguments added manually and system options")
        // create a getopt object with options
        //
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::DefaultValue("123")
                , advgetopt::Help("define a size.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("filenames to handle by this command.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test simple --verbose option";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        advgetopt::getopt opt(environment_options);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // size
            //
            CATCH_REQUIRE(opt.get_option("size") != nullptr);
            CATCH_REQUIRE(opt.get_option('s') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("size"));
            CATCH_REQUIRE(opt.get_default("size") == "123");
            CATCH_REQUIRE(opt.size("size") == 0);

            // pos
            //
            CATCH_REQUIRE(opt.get_option("pos") == nullptr);
            CATCH_REQUIRE(opt.get_option('p') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("pos"));
            CATCH_REQUIRE(opt.get_default("pos").empty());
            CATCH_REQUIRE(opt.size("pos") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name().empty());
            CATCH_REQUIRE(opt.get_program_fullname().empty());

        // setup the program name early
        //
        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "40132",
            "--pos",
            "43,91",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        opt.parse_program_name(argv);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // size
            //
            CATCH_REQUIRE(opt.get_option("size") != nullptr);
            CATCH_REQUIRE(opt.get_option('s') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("size"));
            CATCH_REQUIRE(opt.get_default("size") == "123");
            CATCH_REQUIRE(opt.size("size") == 0);

            // pos
            //
            CATCH_REQUIRE(opt.get_option("pos") == nullptr);
            CATCH_REQUIRE(opt.get_option('p') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("pos"));
            CATCH_REQUIRE(opt.get_default("pos").empty());
            CATCH_REQUIRE(opt.size("pos") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // add --pos
        //
        advgetopt::option const help_option[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("pos")
                , advgetopt::ShortName('p')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("specify position.")
                , advgetopt::DefaultValue("0,0")
            ),
            advgetopt::end_options()
        };
        opt.parse_options_info(help_option);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // size
            //
            CATCH_REQUIRE(opt.get_option("size") != nullptr);
            CATCH_REQUIRE(opt.get_option('s') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("size"));
            CATCH_REQUIRE(opt.get_default("size") == "123");
            CATCH_REQUIRE(opt.size("size") == 0);

            // pos
            //
            CATCH_REQUIRE(opt.get_option("pos") != nullptr);
            CATCH_REQUIRE(opt.get_option('p') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("pos"));
            CATCH_REQUIRE(opt.get_default("pos") == "0,0");
            CATCH_REQUIRE(opt.size("pos") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 0);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // first parse the environment variable
        //
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "file.exe file.so file.dll file.lib file.a");
        opt.parse_environment_variable();

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // size
            //
            CATCH_REQUIRE(opt.get_option("size") != nullptr);
            CATCH_REQUIRE(opt.get_option('s') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("size"));
            CATCH_REQUIRE(opt.get_default("size") == "123");
            CATCH_REQUIRE(opt.size("size") == 0);

            // pos
            //
            CATCH_REQUIRE(opt.get_option("pos") != nullptr);
            CATCH_REQUIRE(opt.get_option('p') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("pos"));
            CATCH_REQUIRE(opt.get_default("pos") == "0,0");
            CATCH_REQUIRE(opt.size("pos") == 0);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 0) == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 1) == "file.so");
            CATCH_REQUIRE(opt.get_string("--", 2) == "file.dll");
            CATCH_REQUIRE(opt.get_string("--", 3) == "file.lib");
            CATCH_REQUIRE(opt.get_string("--", 4) == "file.a");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 5);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // now parse our command line arguments
        //
        opt.parse_arguments(argc, argv);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 0);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 0);

            // size
            //
            CATCH_REQUIRE(opt.get_option("size") != nullptr);
            CATCH_REQUIRE(opt.get_option('s') != nullptr);
            CATCH_REQUIRE(opt.is_defined("size"));
            CATCH_REQUIRE(opt.get_string("size") == "40132");
            CATCH_REQUIRE(opt.get_string("size", 0) == "40132");
            CATCH_REQUIRE(opt.get_long("size") == 40132);
            CATCH_REQUIRE(opt.get_long("size", 0) == 40132);
            CATCH_REQUIRE(opt.get_default("size") == "123");
            CATCH_REQUIRE(opt.size("size") == 1);

            // pos
            //
            CATCH_REQUIRE(opt.get_option("pos") != nullptr);
            CATCH_REQUIRE(opt.get_option('p') != nullptr);
            CATCH_REQUIRE(opt.is_defined("pos"));
            CATCH_REQUIRE(opt.get_string("pos") == "43,91");
            CATCH_REQUIRE(opt.get_string("pos", 0) == "43,91");
            CATCH_REQUIRE(opt.get_default("pos") == "0,0");
            CATCH_REQUIRE(opt.size("pos") == 1);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 0) == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 1) == "file.so");
            CATCH_REQUIRE(opt.get_string("--", 2) == "file.dll");
            CATCH_REQUIRE(opt.get_string("--", 3) == "file.lib");
            CATCH_REQUIRE(opt.get_string("--", 4) == "file.a");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 5);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

        // second set of arguments
        //
        char const * cargv2[] =
        {
            "/usr/bin/arguments",
            "--help",
            "--verbose",
            nullptr
        };
        int const argc2(sizeof(cargv2) / sizeof(cargv2[0]) - 1);
        char ** argv2 = const_cast<char **>(cargv2);

        opt.parse_arguments(argc2, argv2);

            // an invalid parameter, MUST NEVER EXIST
            //
            CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
            CATCH_REQUIRE(opt.get_option('Z') == nullptr);
            CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
            CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
            CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

            // verbose
            //
            CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
            CATCH_REQUIRE(opt.get_option('v') != nullptr);
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_string("verbose").empty());
            CATCH_REQUIRE(opt.get_string("verbose", 0).empty());
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // help
            //
            CATCH_REQUIRE(opt.get_option("help") != nullptr);
            CATCH_REQUIRE(opt.get_option('h') != nullptr);
            CATCH_REQUIRE(opt.is_defined("help"));
            CATCH_REQUIRE(opt.get_string("help").empty());
            CATCH_REQUIRE(opt.get_string("help", 0).empty());
            CATCH_REQUIRE(opt.get_default("help").empty());
            CATCH_REQUIRE(opt.size("help") == 1);

            // size
            //
            CATCH_REQUIRE(opt.get_option("size") != nullptr);
            CATCH_REQUIRE(opt.get_option('s') != nullptr);
            CATCH_REQUIRE(opt.is_defined("size"));
            CATCH_REQUIRE(opt.get_string("size") == "40132");
            CATCH_REQUIRE(opt.get_string("size", 0) == "40132");
            CATCH_REQUIRE(opt.get_long("size") == 40132);
            CATCH_REQUIRE(opt.get_long("size", 0) == 40132);
            CATCH_REQUIRE(opt.get_default("size") == "123");
            CATCH_REQUIRE(opt.size("size") == 1);

            // pos
            //
            CATCH_REQUIRE(opt.get_option("pos") != nullptr);
            CATCH_REQUIRE(opt.get_option('p') != nullptr);
            CATCH_REQUIRE(opt.is_defined("pos"));
            CATCH_REQUIRE(opt.get_string("pos") == "43,91");
            CATCH_REQUIRE(opt.get_string("pos", 0) == "43,91");
            CATCH_REQUIRE(opt.get_default("pos") == "0,0");
            CATCH_REQUIRE(opt.size("pos") == 1);

            // default
            //
            CATCH_REQUIRE(opt.get_option("--") != nullptr);
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 0) == "file.exe");
            CATCH_REQUIRE(opt.get_string("--", 1) == "file.so");
            CATCH_REQUIRE(opt.get_string("--", 2) == "file.dll");
            CATCH_REQUIRE(opt.get_string("--", 3) == "file.lib");
            CATCH_REQUIRE(opt.get_string("--", 4) == "file.a");
            CATCH_REQUIRE(opt.get_default("--").empty());
            CATCH_REQUIRE(opt.size("--") == 5);

            // other parameters
            //
            CATCH_REQUIRE(opt.get_program_name() == "arguments");
            CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");

    CATCH_END_SECTION()

    CATCH_START_SECTION("Verify that not calling link_aliases() causes problems")
        // create a getopt object, we will part the options "manually" later
        //
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_help_header = "Usage: test --licence as equivalent to --license";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;

        advgetopt::getopt opt(environment_options);

        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("licence")    // French spelling
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Alias("license")   // forgot system flags?
            ),
            advgetopt::end_options()
        };

        opt.parse_options_info(options);

        // "forgot" to call the opt.link_aliases() here

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "--licence",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.parse_arguments(argc, argv)
                , advgetopt::getopt_undefined
                , Catch::Matchers::ExceptionMessage(
                              "getopt_exception: getopt::get_alias_destination(): alias is missing. Did you call link_aliases()?"));
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("auto_process_system_arguments", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Test auto-processing of system arguments (with many CATCH_WHEN)")
    {
        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS;
        options.f_help_header = "Usage: testing system arguments.";
        options.f_version = "2.0.1";
        options.f_license = "MIT";
        options.f_Copyright (c) 2019  Made to Order Software Corp.  All Rights Reserved";
        options.f_build_date = "Jun  4 2019";
        options.f_build_time = "23:02:36";

        CATCH_WHEN("Testing --version")
        {
            char const * cargv[] =
            {
                "tests/system-arguments",
                "--version",
                nullptr
            };
            int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
            char ** argv = const_cast<char **>(cargv);

            // do our own try/catch because we want to test the message
            // and exit code of the exception and it's easier to do it
            // this way
            //
            snap::ostream_to_buf<char> out(std::cout);
            try
            {
                advgetopt::getopt::pointer_t opt(std::make_shared<advgetopt::getopt>(options, argc, argv));

                // if it returns opt cannot be nullptr
                //
                CATCH_REQUIRE(opt == nullptr);
            }
            catch(advgetopt::getopt_exit const & e)
            {
                // this is the expected route
                //
                CATCH_REQUIRE(e.what() == std::string("getopt_exception: system command processed."));
                CATCH_REQUIRE(e.code() == 0);
                CATCH_REQUIRE(out.str() == "2.0.1\n");
            }
            catch(std::exception const & e)
            {
                // no other exception is considered valid here
                //
                CATCH_REQUIRE(e.what() == std::string("no other exception was expected..."));
            }
        }

        CATCH_WHEN("Testing --copyright")
        {
            char const * cargv[] =
            {
                "tests/system-arguments",
                "--copyright",
                nullptr
            };
            int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
            char ** argv = const_cast<char **>(cargv);

            // do our own try/catch because we want to test the message
            // and exit code of the exception and it's easier to do it
            // this way
            //
            snap::ostream_to_buf<char> out(std::cout);
            try
            {
                advgetopt::getopt::pointer_t opt(std::make_shared<advgetopt::getopt>(options, argc, argv));

                // if it returns opt cannot be nullptr
                //
                CATCH_REQUIRE(opt == nullptr);
            }
            catch(advgetopt::getopt_exit const & e)
            {
                // this is the expected route
                //
                CATCH_REQUIRE(e.what() == std::string("getopt_exception: system command processed."));
                CATCH_REQUIRE(e.code() == 0);
                CATCH_REQUIRE(out.str() == "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved\n");
            }
            catch(std::exception const & e)
            {
                // no other exception is considered valid here
                //
                CATCH_REQUIRE(e.what() == std::string("no other exception was expected..."));
            }
        }

        CATCH_WHEN("Testing --build-date")
        {
            char const * cargv[] =
            {
                "tests/system-arguments",
                "--build-date",
                nullptr
            };
            int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
            char ** argv = const_cast<char **>(cargv);

            // do our own try/catch because we want to test the message
            // and exit code of the exception and it's easier to do it
            // this way
            //
            snap::ostream_to_buf<char> out(std::cout);
            try
            {
                advgetopt::getopt::pointer_t opt(std::make_shared<advgetopt::getopt>(options, argc, argv));

                // if it returns opt cannot be nullptr
                //
                CATCH_REQUIRE(opt == nullptr);
            }
            catch(advgetopt::getopt_exit const & e)
            {
                // this is the expected route
                //
                CATCH_REQUIRE(e.what() == std::string("getopt_exception: system command processed."));
                CATCH_REQUIRE(e.code() == 0);
                CATCH_REQUIRE(out.str() == "Built on Jun  4 2019 at 23:02:36\n");
            }
            catch(std::exception const & e)
            {
                // no other exception is considered valid here
                //
                CATCH_REQUIRE(e.what() == std::string("no other exception was expected..."));
            }
        }

        CATCH_WHEN("Testing -L (For --license)")
        {
            char const * cargv[] =
            {
                "tests/system-arguments",
                "-L",
                nullptr
            };
            int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
            char ** argv = const_cast<char **>(cargv);

            // do our own try/catch because we want to test the message
            // and exit code of the exception and it's easier to do it
            // this way
            //
            snap::ostream_to_buf<char> out(std::cout);
            try
            {
                advgetopt::getopt::pointer_t opt(std::make_shared<advgetopt::getopt>(options, argc, argv));

                // if it returns opt cannot be nullptr
                //
                CATCH_REQUIRE(opt == nullptr);
            }
            catch(advgetopt::getopt_exit const & e)
            {
                // this is the expected route
                //
                CATCH_REQUIRE(e.what() == std::string("getopt_exception: system command processed."));
                CATCH_REQUIRE(e.code() == 0);
                CATCH_REQUIRE(out.str() == "MIT\n");
            }
            catch(std::exception const & e)
            {
                // no other exception is considered valid here
                //
                CATCH_REQUIRE(e.what() == std::string("no other exception was expected..."));
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_getopt_pointers", "[invalid][getopt][arguments]")
{
    CATCH_START_SECTION("Create getopt with argv set to nullptr.")
        advgetopt::options_environment const environment_opt;

        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<advgetopt::getopt>(environment_opt, 3, nullptr)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: argv pointer cannot be nullptr"));
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_getopt_missing_options", "[invalid][getopt][arguments]")
{
    CATCH_START_SECTION("Create getopt with no options.")
        advgetopt::options_environment options_empty;
        options_empty.f_project_name = "unittest";
        options_empty.f_options = nullptr;
        options_empty.f_help_header = "Usage: try this one and we get a throw (empty list)";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--no-options",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<advgetopt::getopt>(options_empty, argc, argv)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: an empty list of options is not legal, you must"
                    " defined at least one (i.e. --version, --help...)"));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Create getopt with an empty list of options.")
        const advgetopt::option options_empty_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_empty;
        options_empty.f_project_name = "unittest";
        options_empty.f_options = options_empty_list;
        options_empty.f_help_header = "Usage: try this one and we get a throw (empty list)";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--no-options",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<advgetopt::getopt>(options_empty, argc, argv)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                    "getopt_logic_error: an empty list of options is not legal, you must"
                    " defined at least one (i.e. --version, --help...)"));
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_getopt_missing_required_option", "[invalid][getopt][arguments][alias]")
{
    CATCH_START_SECTION("Specify the option without a corresponding parameter.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::Help("The size of the line.")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::DefaultValue("100")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use --size without a value.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "--size",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --size expects an argument.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // the invalid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size") == "100");
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Specify the option with an equal sign but without a corresponding parameter.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::Help("The size of the line.")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::DefaultValue("100")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use --size without a value.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "--size=",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --size must be given a value.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // the invalid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size") == "100");
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Specify the option without a corresponding parameter followed by a long argument.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::Help("The size of the line.")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::DefaultValue("100")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use --size without a value.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "--verbose",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --size expects an argument.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // the invalid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size") == "100");
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Specify the option without a corresponding parameter followed by a short argument.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::Help("The size of the line.")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::DefaultValue("100")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use --size without a value.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size",
            "-v",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --size expects an argument.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // the invalid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size") == "100");
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_default_options", "[invalid][getopt][arguments]")
{
    CATCH_START_SECTION("Using a standalone 'file.txt' when no default option is allowed.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '--' when no default option was defined.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "file.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; we do not know what to do of \"file.txt\"; standalone parameters are not accepted by this program.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a standalone 'file.txt' on the command line when only allowed in a variable.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("default option.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '--' on the command line when only allowed in the variable.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "allowed.jpg");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "file.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: default options are not supported on the command line.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // default on command line passed through
        CATCH_REQUIRE(opt.get_option("--") != nullptr);
        CATCH_REQUIRE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_string("--") == "allowed.jpg");
        CATCH_REQUIRE(opt.get_string("--", 0) == "allowed.jpg");
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 1);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a standalone 'file.txt' in a variable when only allowed on the command line.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("default option.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '--' on the command line when only allowed in the variable.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "file.txt");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "allowed.jpg",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: default options are not supported in the environment variable.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // default on command line passed through
        CATCH_REQUIRE(opt.get_option("--") != nullptr);
        CATCH_REQUIRE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_string("--") == "allowed.jpg");
        CATCH_REQUIRE(opt.get_string("--", 0) == "allowed.jpg");
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 1);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a '--' when no default option is allowed.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '--' when no default option was defined.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "--",
            "file.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; thus -- is not accepted by this program.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a '--' when on the command line when only accepted in variables.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("default option.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '--' when no default option was defined.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-- allowed.txt");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "--",
            "file.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -- is not supported in the environment variable.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") != nullptr);
        CATCH_REQUIRE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_string("--") == "allowed.txt");
        CATCH_REQUIRE(opt.get_string("--", 0) == "allowed.txt");
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 1);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a '--' in the environment variable when only accepted on the command line.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("default option.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '--' when no default option was defined.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", " -- file.txt ");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "--",
            "allowed.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -- is not supported in the environment variable.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") != nullptr);
        CATCH_REQUIRE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_string("--") == "allowed.txt");
        CATCH_REQUIRE(opt.get_string("--", 0) == "allowed.txt");
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 1);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a '-' when no default option is allowed.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '-' when no default option was defined.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "-",
            "file.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; thus - is not accepted by this program.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a '-' when on the command line when only accepted in variables.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("default option.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '-' when no default option was defined.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "- allowed.txt");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "-",
            "file.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option - is not supported in the environment variable.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") != nullptr);
        CATCH_REQUIRE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_string("--") == "allowed.txt");
        CATCH_REQUIRE(opt.get_string("--", 0) == "allowed.txt");
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 1);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Using a '-' in the environment variable when only accepted on the command line.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("default option.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use '-' when no default option was defined.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", " -  file.txt ");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--verbose",
            "-",
            "allowed.txt",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option - is not supported in the environment variable.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // no default
        CATCH_REQUIRE(opt.get_option("--") != nullptr);
        CATCH_REQUIRE(opt.is_defined("--"));
        CATCH_REQUIRE(opt.get_string("--") == "allowed.txt");
        CATCH_REQUIRE(opt.get_string("--", 0) == "allowed.txt");
        CATCH_REQUIRE(opt.get_default("--").empty());
        CATCH_REQUIRE(opt.size("--") == 1);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_options", "[invalid][getopt][arguments]")
{
    CATCH_START_SECTION("Long option with an equal sign but not name.")
        const advgetopt::option options[] =
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
        environment_options.f_help_header = "Usage: use a long name option without the long name.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--=591",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: name missing in \"--=591\".");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size").empty());
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Unknown long option.")
        const advgetopt::option options[] =
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
        environment_options.f_help_header = "Usage: use an unknown long option.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--unknown=568",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --unknown is not supported.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size").empty());
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Long option not available in environment variable.")
        const advgetopt::option options[] =
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
        environment_options.f_help_header = "Usage: use an unknown long option.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--size 4551");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size=568",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --size is not supported in the environment variable.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "568");
        CATCH_REQUIRE(opt.get_string("size", 0) == "568");
        CATCH_REQUIRE(opt.get_long("size") == 568);
        CATCH_REQUIRE(opt.get_long("size", 0) == 568);
        CATCH_REQUIRE(opt.get_default("size").empty());
        CATCH_REQUIRE(opt.size("size") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Long option not available on command line.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("331")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use an unknown long option.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--size 4551");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "--size=568",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --size is not supported on the command line.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "4551");
        CATCH_REQUIRE(opt.get_string("size", 0) == "4551");
        CATCH_REQUIRE(opt.get_long("size") == 4551);
        CATCH_REQUIRE(opt.get_long("size", 0) == 4551);
        CATCH_REQUIRE(opt.get_default("size") == "331");
        CATCH_REQUIRE(opt.size("size") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Unknown short option.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("tell you about our work here.")
            ),
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
        environment_options.f_help_header = "Usage: use an unknown long option.";

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "-vq 568",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -q is not supported.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // verbose
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_default("size").empty());
        CATCH_REQUIRE(opt.size("size") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Short option not available in environment variable.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("tell you about our work here.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use an unknown long option.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-s 4551");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "-sv",
            "568",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -s is not supported in the environment variable.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; we do not know what to do of \"4551\"; standalone parameters are not accepted by this program.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "568");
        CATCH_REQUIRE(opt.get_string("size", 0) == "568");
        CATCH_REQUIRE(opt.get_long("size") == 568);
        CATCH_REQUIRE(opt.get_long("size", 0) == 568);
        CATCH_REQUIRE(opt.get_default("size").empty());
        CATCH_REQUIRE(opt.size("size") == 1);

        // verbose
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Long option not available on command line.")
        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("define the size.")
                , advgetopt::DefaultValue("331")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("tell you about our work here.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: use an unknown long option.";
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-s 4551");

        char const * cargv[] =
        {
            "/usr/bin/arguments",
            "-sv 568",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -s is not supported on the command line.");
        //SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; we do not know what to do of \"4551\"; standalone parameters are not accepted by this program.");
        advgetopt::getopt opt(environment_options, argc, argv);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE(opt.get_option('Z') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));
        CATCH_REQUIRE(opt.get_default("invalid-parameter").empty());
        CATCH_REQUIRE(opt.size("invalid-parameter") == 0);

        // the valid parameter
        CATCH_REQUIRE(opt.get_option("size") != nullptr);
        CATCH_REQUIRE(opt.get_option('s') != nullptr);
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "4551");
        CATCH_REQUIRE(opt.get_string("size", 0) == "4551");
        CATCH_REQUIRE(opt.get_long("size") == 4551);
        CATCH_REQUIRE(opt.get_long("size", 0) == 4551);
        CATCH_REQUIRE(opt.get_default("size") == "331");
        CATCH_REQUIRE(opt.size("size") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "arguments");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/arguments");
    CATCH_END_SECTION()
}


#if 0
// TODO: break this out in sections and even multiple tests
//       the old version would check a command set of options which is
//       not required (doable/useful) with version 2.x
//
//       also this test used to work but now that I heavily changed
//       the configuration file loading process (specifically, added
//       the cache), it fails since this test assumed that you can
//       load a new copy of the same file over and over again

CATCH_TEST_CASE("invalid_parameters", "[invalid][getopt][arguments]")
{

    CATCH_START_SECTION("Check all possible invalid argument")

        // default arguments
        //
        char const * cargv[] =
        {
            "tests/unittests/invalid_parameters",
            "--ignore-parameters",
            nullptr
        };
        const int argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char **argv = const_cast<char **>(cargv);

        // no options available
        //
        const advgetopt::option options_empty_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_empty;
        options_empty.f_project_name = "unittest";
        options_empty.f_options = options_empty_list;
        options_empty.f_help_header = "Usage: try this one and we get a throw (empty list)";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_empty, argc, argv), advgetopt::getopt_logic_error);
        }

        // option without a name and "wrong" type
        //
        const advgetopt::option options_no_name_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_REQUIRED,
                nullptr,
                "we can have a default though",
                nullptr,
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_no_name;
        options_no_name.f_project_name = "unittest";
        options_no_name.f_options = options_no_name_list;
        options_no_name.f_help_header = "Usage: try this one and we get a throw (no name)";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_no_name, argc, argv), advgetopt::getopt_logic_error);
        }

        // long options must be 2+ characters
        //
        const advgetopt::option options_2chars_minimum_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_MULTIPLE,
                "", // cannot be empty string (use nullptr instead)
                nullptr,
                "long option must be 2 characters long at least",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_2chars_minimum;
        options_2chars_minimum.f_project_name = "unittest";
        options_2chars_minimum.f_options = options_2chars_minimum_list;
        options_2chars_minimum.f_help_header = "Usage: try this one and we get a throw (2 chars minimum)";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_2chars_minimum, argc, argv), advgetopt::getopt_logic_error);
        }

        // long options must be 2+ characters
        //
        const advgetopt::option options_2chars_minimum2_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_MULTIPLE,
                "f", // cannot be 1 character
                nullptr,
                "long option must be 2 characters long at least",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_2chars_minimum2;
        options_2chars_minimum2.f_project_name = "unittest";
        options_2chars_minimum2.f_options = options_2chars_minimum2_list;
        options_2chars_minimum2.f_help_header = "Usage: try this one and we get a throw (2 chars minimum 2nd)";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_2chars_minimum2, argc, argv), advgetopt::getopt_logic_error);
        }

        // same long option defined twice
        //
        const advgetopt::option options_defined_twice_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_REQUIRED,
                "filename",
                nullptr,
                "options must be unique",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_REQUIRED,
                "filename", // copy/paste problem maybe?
                nullptr,
                "options must be unique",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_defined_twice;
        options_defined_twice.f_project_name = "unittest";
        options_defined_twice.f_options = options_defined_twice_list;
        options_defined_twice.f_help_header = "Usage: try this one and we get a throw (long defined twice)";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_defined_twice, argc, argv), advgetopt::getopt_logic_error);
        }

        // same short option defined twice
        //
        const advgetopt::option options_short_defined_twice_list[] =
        {
            {
                'f',
                advgetopt::GETOPT_FLAG_REQUIRED,
                "from",
                nullptr,
                "options must be unique",
                nullptr
            },
            {
                'f',
                advgetopt::GETOPT_FLAG_REQUIRED,
                "to",
                nullptr,
                "options must be unique",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_short_defined_twice;
        options_short_defined_twice.f_project_name = "unittest";
        options_short_defined_twice.f_options = options_short_defined_twice_list;
        options_short_defined_twice.f_help_header = "Usage: try this one and we get a throw (short defined twice)";
        options_short_defined_twice.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_short_defined_twice, argc, argv), advgetopt::getopt_logic_error);
        }

        // 2 default_multiple_argument's in the same list is invalid
        //
        const advgetopt::option options_two_default_multiple_arguments_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "filenames",
                nullptr,
                "other parameters are viewed as filenames",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "more",
                nullptr,
                "yet other parameters are viewed as \"more\" data--here it breaks, one default max.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_two_default_multiple_arguments;
        options_two_default_multiple_arguments.f_project_name = "unittest";
        options_two_default_multiple_arguments.f_options = options_two_default_multiple_arguments_list;
        options_two_default_multiple_arguments.f_help_header = "Usage: try this one and we get a throw (two defaults by flag, multiple args)";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_two_default_multiple_arguments, argc, argv), advgetopt::getopt_logic_error);
        }

        // 2 default_argument's in the same list is invalid
        //
        const advgetopt::option options_two_default_arguments_list[] =
        {
            {
                '\0',
                0,
                "--",
                nullptr,
                "one other parameter is viewed as a filename",
                nullptr
            },
            {
                '\0',
                0,
                "--",
                nullptr,
                "yet other parameter viewed as \"more\" data--here it breaks, one default max.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_two_default_arguments;
        options_two_default_arguments.f_project_name = "unittest";
        options_two_default_arguments.f_options = options_two_default_arguments_list;
        options_two_default_arguments.f_help_header = "Usage: try this one and we get a throw (two default args by name)";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_two_default_arguments, argc, argv), advgetopt::getopt_logic_error);
        }

        // mix of default arguments in the same list is invalid
        //
        const advgetopt::option options_mix_of_default_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_MULTIPLE,
                "--",
                nullptr,
                "other parameters are viewed as filenames",
                nullptr
            },
            {
                '\0',
                0,
                "--",
                nullptr,
                "yet other parameter viewed as \"more\" data--here it breaks, one default max.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_mix_of_default;
        options_mix_of_default.f_project_name = "unittest";
        options_mix_of_default.f_options = options_mix_of_default_list;
        options_mix_of_default.f_help_header = "Usage: try this one and we get a throw (mix flags of defaults by name)";
        options_mix_of_default.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options_mix_of_default, argc, argv), advgetopt::getopt_logic_error);
        }

        // try the - and -- without a default in the arguments
        //
        const advgetopt::option options_no_defaults_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                "verbose",
                nullptr,
                "just a flag to test.",
                0
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_no_defaults;
        options_no_defaults.f_project_name = "unittest";
        options_no_defaults.f_options = options_no_defaults_list;
        options_no_defaults.f_help_header = "Usage: try this one and we get a throw (no defaults)";

        {
            // a filename by itself is a problem when there is no default
            //
            char const * sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                "this",
                "filename",
                nullptr
            };
            const int sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
            char ** sub_argv(const_cast<char **>(sub_cargv));

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; we do not know what to do of \"this\"; standalone parameters are not accepted by this program.");
            advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
        {
            // a '-' by itself is a problem when there is no default because it
            // is expected to represent a filename (stdin)
            //
            char const * sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                "-",
                nullptr
            };
            const int sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
            char **sub_argv(const_cast<char **>(sub_cargv));

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; thus - is not accepted by this program.");
            advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
        {
            // the -- by itself would be fine, but since it represents a
            // transition from arguments to only filenames (or whatever the
            // program expects as default options) it generates an error if
            // no default options are accepted
            //
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                "--", // already just by itself it causes problems
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; thus -- is not accepted by this program.");
            advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
        {
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                "--",
                "66",
                "--filenames",
                "extra",
                "--file",
                "names",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: no default options defined; thus -- is not accepted by this program.");
            advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
        {
            // check that -v, that does not exist, generates a usage error
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "-v",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -v is not supported.");
            advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        // check -- when default does not allowed environment variables
        //
        const advgetopt::option options_no_defaults_in_envvar_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
                "verbose",
                nullptr,
                "just a flag to test.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
                "--",
                nullptr,
                "default multiple filenames",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_no_defaults_in_envvar;
        options_no_defaults_in_envvar.f_project_name = "unittest";
        options_no_defaults_in_envvar.f_options = options_no_defaults_in_envvar_list;
        options_no_defaults_in_envvar.f_help_header = "Usage: try this one and we get a throw (no defaults in envvar)";
        options_no_defaults_in_envvar.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose - no default here");
            char const * sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                "-",
                "here",
                "it",
                "works",
                nullptr
            };
            int const sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option - is not supported in the environment variable.");
            advgetopt::getopt opt(options_no_defaults_in_envvar, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose no default here");
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                "-",
                "here",
                "it",
                "works",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: default options are not supported in the environment variable.");
            advgetopt::getopt opt(options_no_defaults_in_envvar, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
        {
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose -- foo bar blah");
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                "here",
                "it",
                "works",
                "--",
                "66",
                "--filenames",
                "extra",
                "--file",
                "names",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -- is not supported in the environment variable.");
            advgetopt::getopt opt(options_no_defaults_in_envvar, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        // unnknown long options
        //
        const advgetopt::option valid_options_unknown_command_line_option_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
                "command",
                nullptr,
                "there is a --command, but the user tries --verbose!",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment valid_options_unknown_command_line_option;
        valid_options_unknown_command_line_option.f_project_name = "unittest";
        valid_options_unknown_command_line_option.f_options = valid_options_unknown_command_line_option_list;
        valid_options_unknown_command_line_option.f_help_header = "Usage: try this one and we get a throw (unknown command line option)";

        {
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
                "--verbose",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --verbose is not supported.");
            advgetopt::getopt opt(valid_options_unknown_command_line_option, sub_argc, sub_argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        // illegal short or long option in variable
        //
        const advgetopt::option options_illegal_in_variable_list[] =
        {
            {
                'v',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                "verbose",
                nullptr,
                "just a flag to test.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options_illegal_in_variable;
        options_illegal_in_variable.f_project_name = "unittest";
        options_illegal_in_variable.f_options = options_illegal_in_variable_list;
        options_illegal_in_variable.f_help_header = "Usage: try this one and we get a throw (illegal in variable)";
        options_illegal_in_variable.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            // long
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --verbose is not supported in the environment variable.");
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(options_illegal_in_variable, argc, argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
        {
            // short
            snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "-v");

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option -v is not supported in the environment variable.");
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(options_illegal_in_variable, argc, argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }

        // configuration file options must have a long name
        //
        const advgetopt::option configuration_long_name_missing_list[] =
        {
            {
                'c',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE,
                nullptr,
                nullptr,
                "a valid option",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment configuration_long_name_missing;
        configuration_long_name_missing.f_project_name = "unittest";
        configuration_long_name_missing.f_options = configuration_long_name_missing_list;
        configuration_long_name_missing.f_help_header = "Usage: try this one and we get a throw (long name missing)";
        configuration_long_name_missing.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(configuration_long_name_missing, argc, argv), advgetopt::getopt_logic_error);
        }

        // create invalid configuration files
        //
        const advgetopt::option valid_options_list[] =
        {
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE,
                "valid",
                nullptr,
                "a valid option",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE,
                "command",
                nullptr,
                "a valid command, but not a valid configuration option",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "filename",
                nullptr,
                "other parameters are viewed as filenames",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/.config";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }

        std::string const config_filename(tmpdir + "/advgetopt.config");
        char const * invalid_confs[] =
        {
            config_filename.c_str(),
            nullptr
        };

        advgetopt::options_environment valid_options;
        valid_options.f_project_name = "unittest";
        valid_options.f_options = valid_options_list;
        valid_options.f_help_header = "Usage: try this one and we get a throw (valid options!)";
        valid_options.f_configuration_files = invalid_confs;

        {
            // '=' operator missing
            {
                std::ofstream config_file;
                config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "valid param\n"
                    "# no spaces acceptable in param names\n"
                ;
            }
            {
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option name from \"valid param\" on line 2 in configuration file \""
                                        + config_filename
                                        + "\" cannot include a space, missing assignment operator?");
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
                advgetopt::getopt opt(valid_options, argc, argv);
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }
        {
            // same effect with a few extra spaces
            {
                std::ofstream config_file;
                config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    " valid param \n"
                    "# no spaces acceptable in param names\n"
                ;
            }
            {
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option name from \" valid param \" on line 2 in configuration file \""
                                        + config_filename
                                        + "\" cannot include a space, missing assignment operator?");
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
                advgetopt::getopt opt(valid_options, argc, argv);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }
        {
            // param name missing
            {
                std::ofstream config_file;
                config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    " = valid param\n"
                    "# no spaces acceptable in param names\n"
                ;
            }
            {
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: no option name in \" = valid param\" on line 2 from configuration file \""
                                        + config_filename
                                        + "\", missing name before = sign?");
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
                advgetopt::getopt opt(valid_options, argc, argv);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }
        {
            // param name starts with a dash or more
            {
                std::ofstream config_file;
                config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "--valid=param\n"
                    "# no spaces acceptable in param names\n"
                ;
            }
            {
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option names in configuration files cannot start with a dash or an underscore in \"--valid=param\" on line 2 from configuration file \""
                                        + config_filename
                                        + "\".");
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
                advgetopt::getopt opt(valid_options, argc, argv);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }
        {
            // unknown param name
            {
                std::ofstream config_file;
                config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "invalid=param\n"
                    "# no spaces acceptable in param names\n"
                ;
            }
            {
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: unknown option \"invalid\" found in configuration file \""
                                        + config_filename
                                        + "\".");
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
                advgetopt::getopt opt(valid_options, argc, argv);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }
        {
            // known command, not valid in configuration file
            {
                std::ofstream config_file;
                config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "command=value\n"
                    "# no spaces acceptable in param names\n"
                ;
            }
            {
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option \"command\" is not supported in configuration files (found in \""
                                        + config_filename
                                        + "\").");
                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --ignore-parameters is not supported.");
                advgetopt::getopt opt(valid_options, argc, argv);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }

#if 0
    // in version 2.x we do not have this case anymore

        // one of the options has an invalid mode; explicit option
        {
            const advgetopt::option options_list[] =
            {
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
                    "mode",
                    nullptr,
                    "an argument with an invalid mode to see that we get an exception",
                    static_cast<advgetopt::getopt::argument_mode_t>(static_cast<int>(advgetopt::getopt::argument_mode_t::end_of_options) + 1)
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_END,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                }
            };

            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_options = options_list;
            options.f_help_header = "Usage: one of the options has an invalid mode";

            {
                const char *cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt",
                    "--mode",
                    "test",
                    nullptr
                };
                const int argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char **argv2 = const_cast<char **>(cargv2);

                // here we hit the one in add_options() (plural)
                // the one in add_option() is not reachable because it is called only
                // when a default option is defined and that means the mode is
                // correct
                CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(argc2, argv2, options, confs, nullptr); }, advgetopt::getopt_invalid );
            }
            {
                const char *cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt",
                    nullptr
                };
                const int argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char **argv2 = const_cast<char **>(cargv2);

                // this one checks that the --mode flag does indeed generate a
                // throw when not used on the command line but then gets shown
                // in the usage() function
                advgetopt::getopt opt(argc2, argv2, options, confs, nullptr);
                for(int i(static_cast<int>(advgetopt::getopt::status_t::no_error)); i <= static_cast<int>(advgetopt::getopt::status_t::fatal); ++i)
                {
                    CATCH_REQUIRE_THROWS_AS( opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_invalid);
                }
            }
        }
#endif

        // a valid initialization, but not so valid calls afterward
        {
            advgetopt::option const options_list[] =
            {
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                    "validate",
                    nullptr,
                    "this is used to validate different things.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "long",
                    nullptr,
                    "used to validate that invalid numbers generate an error.",
                    nullptr
                },
                {
                    'o',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "out-of-bounds",
                    nullptr,
                    "valid values from 1 to 9.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-specified-and-no-default",
                    nullptr,
                    "test long without having used the option and no default.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-specified-with-invalid-default",
                    "123abc",
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-specified-string-without-default",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "string",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                    "filename",
                    nullptr,
                    "other parameters are viewed as filenames",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_END,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                }
            };
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt",
                "--validate",
                "--long",
                "123abc",
                "--out-of-bounds",
                "123",
                "--string",
                "string value",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_options = options_list;
            options.f_help_header = "Usage: one of the options has an invalid mode";
            options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

            advgetopt::getopt opt(options, argc2, argv2);

            // cannot get the default without a valid name!
            CATCH_REQUIRE_THROWS_AS( opt.get_default(""), advgetopt::getopt_logic_error);

            // cannot get a long named "blah"
            CATCH_REQUIRE_THROWS_AS( opt.get_long("blah"), advgetopt::getopt_logic_error);
            // existing "long", but only 1 entry
            CATCH_REQUIRE_THROWS_AS( opt.get_long("long", 100), advgetopt::getopt_undefined);
            long l(-1);
            CATCH_REQUIRE_THROWS_AS( l = opt.get_long("not-specified-and-no-default", 0), advgetopt::getopt_logic_error);
            CATCH_REQUIRE(l == -1);
            CATCH_REQUIRE_THROWS_AS( l = opt.get_long("not-specified-with-invalid-default", 0), advgetopt::getopt_logic_error);
            CATCH_REQUIRE(l == -1);
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number (123abc) in parameter --long.");
            l = opt.get_long("long");
            CATCH_REQUIRE(l == -1);
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: 123 is out of bounds (1..9 inclusive) in parameter --out-of-bounds.");
            l = opt.get_long("out-of-bounds", 0, 1, 9);
            CATCH_REQUIRE(l == -1);
            std::string s;
            CATCH_REQUIRE_THROWS_AS( s = opt.get_string("not-specified-string-without-default", 0), advgetopt::getopt_logic_error);
            CATCH_REQUIRE(s.empty());
            CATCH_REQUIRE_THROWS_AS( s = opt.get_string("string", 100), advgetopt::getopt_undefined);
            CATCH_REQUIRE(s.empty());

            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

            // reuse all those invalid options with the reset() function
            // and expect the same result
            // (the constructor is expected to call reset() the exact same way)
            //  -- this changed in version 2.x; we can't reset the option definitions
            //
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_empty_list, confs, nullptr), advgetopt::getopt_invalid);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_no_name_list, confs, nullptr), advgetopt::getopt_invalid);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_2chars_minimum, confs, nullptr), advgetopt::getopt_invalid);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_2chars_minimum2, confs, nullptr), advgetopt::getopt_invalid);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_defined_twice, confs, "ADVGETOPT_TEST_OPTIONS"), advgetopt::getopt_invalid);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_short_defined_twice, confs, nullptr), advgetopt::getopt_invalid);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_two_default_multiple_arguments, confs, nullptr), advgetopt::getopt_invalid_default);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_two_default_arguments, confs, "ADVGETOPT_TEST_OPTIONS"), advgetopt::getopt_invalid_default);
            //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_mix_of_default, confs, nullptr), advgetopt::getopt_invalid_default);
        }

        // valid initialization + usage calls
        {
            const advgetopt::option options_list[] =
            {
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                    "validate",
                    nullptr,
                    "this is used to validate different things.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "long",
                    nullptr,
                    "used to validate that invalid numbers generate an error.",
                    nullptr
                },
                {
                    'o',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "out-of-bounds",
                    nullptr,
                    "valid values from 1 to 9.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-specified-and-no-default",
                    nullptr,
                    "test long without having used the option and no default.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE,
                    "not-specified-with-invalid-default",
                    "123abc",
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-specified-string-without-default",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "string",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    'u',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE,
                    "unique",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    'q',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
                    "quiet",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                    "filename",
                    nullptr,
                    "other parameters are viewed as filenames.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_END,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                }
            };
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt",
                "--validate",
                "--long",
                "123abc",
                "--out-of-bounds",
                "123",
                "--string",
                "string value",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_options = options_list;
            options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls)";
            options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

            // this initialization works as expected
            advgetopt::getopt opt(options, argc2, argv2);

            // all of the following have the exiting exception
            CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) ==
    "Usage: try this one and we get a throw (valid options + usage calls)\n"
    "   --long <arg>               used to validate that invalid numbers generate an\n"
    "                              error.\n"
    "   --not-specified-and-no-default <arg>\n"
    "                              test long without having used the option and no\n"
    "                              default.\n"
    "   --not-specified-string-without-default <arg>\n"
    "                              test long with an invalid default value.\n"
    "   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
    "                              test long with an invalid default value.\n"
    "   --out-of-bounds or -o <arg>\n"
    "                              valid values from 1 to 9.\n"
    "   --quiet or -q {<arg>}      test long with an invalid default value.\n"
    "   --string <arg>             test long with an invalid default value.\n"
    "   --unique or -u [<arg>]     test long with an invalid default value.\n"
    "   --validate                 this is used to validate different things.\n"
    "   [default arguments]        other parameters are viewed as filenames.\n"
                            );
        }

        // valid initialization + usage calls with a few different options
        {
            const advgetopt::option options_list[] =
            {
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                    "validate",
                    nullptr,
                    "this is used to validate different things.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "long",
                    nullptr,
                    "used to validate that invalid numbers generate an error.",
                    nullptr
                },
                {
                    'o',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "out-of-bounds",
                    nullptr,
                    "valid values from 1 to 9.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-specified-and-no-default",
                    nullptr,
                    "test long without having used the option and no default.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE,
                    "not-specified-with-invalid-default",
                    "123abc",
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-specified-string-without-default",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "string",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    'u',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE,
                    "unique",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    'q',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
                    "quiet",
                    nullptr,
                    "test long with an invalid default value.",
                    nullptr
                },
                {
                    'l',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                    "not-in-v2-though",
                    nullptr,
                    "long with just a letter.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE,
                    "--",
                    nullptr,
                    "other parameters are viewed as filenames; and we need at least one option with a very long help to check that it wraps perfectly (we'd really need to get the output of the command and check that against what is expected because at this time the test is rather blind in that respect! FIXED IN v2!)",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_END,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                }
            };

            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_options = options_list;
            options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls bis)";
            options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

            {
                // make sure that --long (REQUIRED) fails if the
                // long value is not specified
                //
                char const * cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt",
                    "--validate",
                    "--long",
                    "--out-of-bounds",
                    "123",
                    "--string",
                    "string value",
                    nullptr
                };
                int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char ** argv2 = const_cast<char **>(cargv2);

                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --long expects an argument.");
                advgetopt::getopt opt(options, argc2, argv2);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
            {
                // again with the lone -l (no long name)
                //
                char const * cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt",
                    "--validate",
                    "-l",
                    "--out-of-bounds",
                    "123",
                    "--string",
                    "string value",
                    nullptr
                };
                int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char ** argv2 = const_cast<char **>(cargv2);

                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --not-in-v2-though expects an argument.");
                advgetopt::getopt opt(options, argc2, argv2);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
            {
                char const * cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt",
                    "--validate",
                    "--long",
                    "123abc",
                    "--out-of-bounds",
                    "123",
                    "--string",
                    "string value",
                    nullptr
                };
                int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char ** argv2 = const_cast<char **>(cargv2);

                // this initialization works as expected
                advgetopt::getopt opt(options, argc2, argv2);

                // all of the following have the exiting exception
    //            for(int i(static_cast<int>(advgetopt::getopt::status_t::no_error)); i <= static_cast<int>(advgetopt::getopt::status_t::fatal); ++i)
    //            {
    //                CATCH_REQUIRE_THROWS_AS( opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exit);
    //            }
                CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) ==
    "Usage: try this one and we get a throw (valid options + usage calls bis)\n"
    "   --long <arg>               used to validate that invalid numbers generate an\n"
    "                              error.\n"
    "   --not-in-v2-though or -l <arg>\n"
    "                              long with just a letter.\n"
    "   --not-specified-and-no-default <arg>\n"
    "                              test long without having used the option and no\n"
    "                              default.\n"
    "   --not-specified-string-without-default <arg>\n"
    "                              test long with an invalid default value.\n"
    "   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
    "                              test long with an invalid default value.\n"
    "   --out-of-bounds or -o <arg>\n"
    "                              valid values from 1 to 9.\n"
    "   --quiet or -q {<arg>}      test long with an invalid default value.\n"
    "   --string <arg>             test long with an invalid default value.\n"
    "   --unique or -u [<arg>]     test long with an invalid default value.\n"
    "   --validate                 this is used to validate different things.\n"
    "   [default argument]         other parameters are viewed as filenames; and we\n"
    "                              need at least one option with a very long help to\n"
    "                              check that it wraps perfectly (we'd really need to\n"
    "                              get the output of the command and check that\n"
    "                              against what is expected because at this time the\n"
    "                              test is rather blind in that respect! FIXED IN\n"
    "                              v2!)\n"
                            );
            }
        }

        // strange entry without a name
        {
            advgetopt::option const options_list[] =
            {
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR | advgetopt::GETOPT_FLAG_FLAG,
                    nullptr,
                    nullptr,
                    "this entry has help, but no valid name...",
                    nullptr
                },
                {
                    'v',
                    advgetopt::GETOPT_FLAG_FLAG,
                    "verbose",
                    nullptr,
                    "show more stuff when found on the command line.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_END,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                }
            };

            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_options = options_list;
            options.f_help_header = "Usage: try this one and we get a throw (strange empty entry!)";
            options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

            {
                char const * cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                    nullptr
                };
                int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char ** argv2 = const_cast<char **>(cargv2);

                // this initialization works as expected
                //
                CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options, argc2, argv2), advgetopt::getopt_logic_error );

    //            // all of the following have the exiting exception
    //            for(int i(static_cast<int>(advgetopt::getopt::status_t::no_error)); i <= static_cast<int>(advgetopt::getopt::status_t::fatal); ++i)
    //            {
    //                CATCH_REQUIRE_THROWS_AS( opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_invalid);
    //            }
    //std::cout << "test usage output here? " << opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) << "\n";
            }
        }

        // required multiple without arguments
        {
            advgetopt::option const options_list[] =
            {
                {
                    'f',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                    "filenames",
                    nullptr,
                    "test a required multiple without any arguments and fail.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_END,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                }
            };

            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_options = options_list;
            options.f_help_header = "Usage: try this one and we get a throw (required multiple without args)";

            {
                // first with -f
                char const * cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                    "-f",
                    nullptr
                };
                int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char ** argv2 = const_cast<char **>(cargv2);

                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --filenames expects an argument.");
                advgetopt::getopt opt(options, argc2, argv2);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
            {
                // second with --filenames
                const char *cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                    "--filenames",
                    nullptr
                };
                int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char ** argv2 = const_cast<char **>(cargv2);

                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --filenames expects an argument.");
                advgetopt::getopt opt(options, argc2, argv2);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }

        // required multiple without arguments, short name only
        {
            advgetopt::option const options_list[] =
            {
                {
                    'f',
                    advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE,
                    "filenames",
                    nullptr,
                    "test a required multiple without any arguments and fail.",
                    nullptr
                },
                {
                    '\0',
                    advgetopt::GETOPT_FLAG_END,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                }
            };

            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_options = options_list;
            options.f_help_header = "Usage: try this one and we get a throw (required multiple without args + short name)";

            {
                // -f only in this case
                char const * cargv2[] =
                {
                    "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                    "-f",
                    nullptr
                };
                int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
                char ** argv2 = const_cast<char **>(cargv2);

                SNAP_CATCH2_NAMESPACE::push_expected_log("error: option --filenames expects an argument.");
                advgetopt::getopt opt(options, argc2, argv2);
                SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
            }
        }
    CATCH_END_SECTION()
}
#endif


// vim: ts=4 sw=4 et
