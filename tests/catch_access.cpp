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



CATCH_TEST_CASE("program_name", "[program_name][valid][getopt]")
{
    CATCH_START_SECTION("program_name: Verify a nullptr program name in argv[]s")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_help_header = "Usage: verify program name handling";

        char const * cargv[] =
        {
            nullptr,
            "--verbose",
            nullptr
        };
        //int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);

        opt.parse_program_name(argv);

        CATCH_REQUIRE(opt.get_program_name().empty());
        CATCH_REQUIRE(opt.get_program_fullname().empty());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("program_name: verify a program name with no path")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: verify program name handling";

        char const * cargv[] =
        {
            "basename-only.exe",
            "--verbose",
            nullptr
        };
        //int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);

        opt.parse_program_name(argv);

        CATCH_REQUIRE(opt.get_program_name() == "basename-only.exe");
        CATCH_REQUIRE(opt.get_program_fullname() == "basename-only.exe");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("program_name: verify a program name with a relative path")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: verify program name handling";

        char const * cargv[] =
        {
            "project/bin/and-basename.tool",
            "--verbose",
            nullptr
        };
        //int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);

        opt.parse_program_name(argv);

        CATCH_REQUIRE(opt.get_program_name() == "and-basename.tool");
        CATCH_REQUIRE(opt.get_program_fullname() == "project/bin/and-basename.tool");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("program_name: verify a program name with a relative path and backslashes")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: verify program name handling";

        char const * cargv[] =
        {
            "project\\bin\\and-basename.tool",
            "--verbose",
            nullptr
        };
        //int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);

        opt.parse_program_name(argv);

        CATCH_REQUIRE(opt.get_program_name() == "and-basename.tool");
        CATCH_REQUIRE(opt.get_program_fullname() == "project\\bin\\and-basename.tool");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("program_name: verify a program name with a full path")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: verify program name handling";

        char const * cargv[] =
        {
            "/usr/bin/basename",
            "--verbose",
            nullptr
        };
        //int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);

        opt.parse_program_name(argv);

        CATCH_REQUIRE(opt.get_program_name() == "basename");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/basename");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("program_name: verify a program name with a full path and backslashes")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_help_header = "Usage: verify program name handling";

        char const * cargv[] =
        {
            "\\usr\\bin\\basename",
            "--verbose",
            nullptr
        };
        //int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options);

        opt.parse_program_name(argv);

        CATCH_REQUIRE(opt.get_program_name() == "basename");
        CATCH_REQUIRE(opt.get_program_fullname() == "\\usr\\bin\\basename");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("project_name", "[project_name][valid][getopt]")
{
    CATCH_START_SECTION("project_name: verify a nullptr project name")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_help_header = "Usage: verify project name handling";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE(opt.get_project_name().empty());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("project_name: verify an actual project name")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unit-test";
        environment_options.f_help_header = "Usage: verify program name handling";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE(opt.get_project_name() == "unit-test");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_program_name", "[program_name][invalid][getopt]")
{
    CATCH_START_SECTION("invalid_program_name: parsing a nullptr program name throws")
    {
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_help_header = "Usage: verify program name handling";

        advgetopt::getopt opt(environment_options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.parse_program_name(nullptr)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: argv pointer cannot be nullptr"));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
