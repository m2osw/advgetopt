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

// advgetopt
//
#include    <advgetopt/flags.h>
#include    <advgetopt/exception.h>


// self
//
#include    "catch_main.h"


// snapdev
//
#include    <snapdev/safe_setenv.h>


// C++
//
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>







CATCH_TEST_CASE("option_string", "[getopt][string]")
{
    CATCH_START_SECTION("empty string returns an empty empty")
    {
        CATCH_REQUIRE(advgetopt::escape_shell_argument(std::string()) == std::string("\"\""));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("empty string returns empty")
    {
        std::string const g_simple_characters("_/.-+=0123456789ABCEFGHIJKLMNOPQRSTUVWXYZabcefghijklmnopqrstuvwxyz");
        for(std::size_t i(0); i < g_simple_characters.length(); ++i)
        {
            std::string t;
            t += g_simple_characters[i];
            CATCH_REQUIRE(advgetopt::escape_shell_argument(t) == t);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("string in single quotes")
    {
        CATCH_REQUIRE(advgetopt::escape_shell_argument("'between quotes'") == std::string("''\\''between quotes'\\'''"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("string with apostrophe")
    {
        CATCH_REQUIRE(advgetopt::escape_shell_argument("c'est un test") == std::string("'c'\\''est un test'"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("string with special characters")
    {
        CATCH_REQUIRE(advgetopt::escape_shell_argument("space colon: and semi-colon;") == std::string("'space colon: and semi-colon;'"));
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("options_to_string", "[arguments][valid][getopt]")
{
    CATCH_START_SECTION("Transform command line options back to a shell compatible command.")
    {
        // create a getopt object with options
        //
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::EnvironmentVariableName("VERBOSE")
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("coordinates")
                , advgetopt::ShortName('C')
                , advgetopt::Flags(advgetopt::command_flags<
                          advgetopt::GETOPT_FLAG_REQUIRED
                        , advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::EnvironmentVariableName("COORDINATES")
                , advgetopt::Help("define the angle.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("angle")
                , advgetopt::ShortName('a')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::DefaultValue("90")
                , advgetopt::EnvironmentVariableName("ANGLE")
                , advgetopt::Help("define the angle.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("weight")
                , advgetopt::ShortName('w')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::DefaultValue("455")
                , advgetopt::EnvironmentVariableName("WEIGHT")
                , advgetopt::Help("define the weight.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::EnvironmentVariableName("FILES")
                , advgetopt::Help("list of filenames.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: testing system arguments.";
        environment_options.f_options = options;
        environment_options.f_version = "3.1.5";
        environment_options.f_license = "MIT";
        environment_options.f_copyright = "Copyright (c) 2022  Made to Order Software Corp. -- All Rights Reserved";
        environment_options.f_build_date = "Jun  4 2019";
        environment_options.f_build_time = "23:02:36";

        CATCH_WHEN("Mix environment variable and command line options")
        {
            snapdev::safe_setenv env_size("WEIGHT", "303.183");

            char const * cargv[] =
            {
                "tests/system-arguments",
                "-C",
                "33",
                "52",
                "109",
                "17",
                "37",
                "--verbose",
                "file1",
                "more2",
                "info3",
                "-a",
                "90",
                nullptr
            };
            int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
            char ** argv = const_cast<char **>(cargv);

            // the command line has priority, but the MULTIPLE creates a
            // problem here...
            //
            advgetopt::getopt::pointer_t opt(std::make_shared<advgetopt::getopt>(environment_options, argc, argv));
            CATCH_REQUIRE(opt != nullptr);

            CATCH_REQUIRE(opt->is_defined("verbose"));

            CATCH_REQUIRE(opt->is_defined("coordinates"));
            CATCH_REQUIRE(opt->get_string("coordinates") == "33");
            CATCH_REQUIRE(opt->get_string("coordinates", 1) == "52");
            CATCH_REQUIRE(opt->get_string("coordinates", 2) == "109");
            CATCH_REQUIRE(opt->get_string("coordinates", 3) == "17");
            CATCH_REQUIRE(opt->get_string("coordinates", 4) == "37");

            CATCH_REQUIRE(opt->is_defined("weight"));
            CATCH_REQUIRE(opt->get_string("weight") == "303.183");

            CATCH_REQUIRE(opt->is_defined("angle"));
            CATCH_REQUIRE(opt->get_string("angle") == "90");

            CATCH_REQUIRE(opt->is_defined("--"));
            CATCH_REQUIRE(opt->get_string("--", 0) == "file1");
            CATCH_REQUIRE(opt->get_string("--", 1) == "more2");
            CATCH_REQUIRE(opt->get_string("--", 2) == "info3");

            CATCH_REQUIRE(opt->options_to_string() == "--coordinates 33 52 109 17 37 --verbose --weight 303.183 -- file1 more2 info3");
            CATCH_REQUIRE(opt->options_to_string(true) == "tests/system-arguments --coordinates 33 52 109 17 37 --verbose --weight 303.183 -- file1 more2 info3");
            CATCH_REQUIRE(opt->options_to_string(false, true) == "--angle 90 --coordinates 33 52 109 17 37 --verbose --weight 303.183 -- file1 more2 info3");
            CATCH_REQUIRE(opt->options_to_string(true, true) == "tests/system-arguments --angle 90 --coordinates 33 52 109 17 37 --verbose --weight 303.183 -- file1 more2 info3");
        }
    }
    CATCH_END_SECTION()
}




// vim: ts=4 sw=4 et
