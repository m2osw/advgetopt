/*
 * Files:
 *    tests/valid_config_files.cpp
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
//#include "advgetopt/advgetopt.h"
#include "advgetopt/exception.h"
//#include "advgetopt/log.h"

// C++ lib
//
//#include <cstring>
//#include <cmath>
//#include <sstream>
#include <fstream>

//// C lib
////
//#include <time.h>







CATCH_TEST_CASE("valid_config_files", "config,getopt")
{
    // default arguments
    const char *cargv[] =
    {
        "tests/unittests/AdvGetOptUnitTests::valid_config_files",
        "--valid-parameter",
        nullptr
    };
    const int argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
    char **argv = const_cast<char **>(cargv);

    //std::vector<std::string> empty_confs;

    std::string tmpdir(unittest::g_tmp_dir);
    tmpdir += "/.config";
    std::stringstream ss;
    ss << "mkdir -p " << tmpdir;
    if(system(ss.str().c_str()) != 0)
    {
        std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
        exit(1);
    }
    std::string const config_filename(tmpdir + "/advgetopt.config");

    //std::vector<std::string> confs;
    //confs.push_back(config_filename);
    char const * confs[] =
    {
        config_filename.c_str(),
        nullptr
    };

    char const * space_separators[] =
    {
        " ",
        "\t",
        nullptr
    };

    // some command line options to test against
    const advgetopt::option valid_options_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: test valid options",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE,
            "valid-parameter",
            nullptr,
            "a valid option",
            nullptr
        },
        {
            'v',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "a verbose like option, select it or not",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "not-specified",
            nullptr,
            "a verbose like option, but never specified anywhere",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "number",
            "111",
            "expect a valid number",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "string",
            "the default string",
            "expect a valid string",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_MULTIPLE,
            "filenames",
            "a.out",
            "expect multiple strings",
            space_separators
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

    advgetopt::options_environment valid_options_no_confs;
    valid_options_no_confs.f_project_name = "unittest";
    valid_options_no_confs.f_options = valid_options_list;
    valid_options_no_confs.f_help_header = "Usage: test valid options";
    valid_options_no_confs.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    advgetopt::options_environment valid_options;
    valid_options.f_project_name = "unittest";
    valid_options.f_options = valid_options_list;
    valid_options.f_help_header = "Usage: test valid options";
    valid_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
    valid_options.f_configuration_files = confs;

    // test that a configuration files gets loaded as expected
    {
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number = 5\n"
                "string=     strange\n"
                "verbose\n"
                "filenames\t= foo bar blah\n"
            ;
        }

        advgetopt::getopt opt(valid_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 5);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "strange");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 3);

        // as we're at it, make sure that indices out of bounds generate an exception
        for(int i(-100); i <= 100; ++i)
        {
            if(i != 0 && i != 1 && i != 2)
            {
                CATCH_REQUIRE_THROWS_AS( opt.get_string("filenames", i), advgetopt::getopt_exception_undefined);
            }
        }

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // make sure that command line options have priority or are cumulative
    {
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number = 5\n"
                "string=     strange\n"
                "verbose\n"
                "filenames\t= foo bar blah\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files",
            "--valid-parameter",
            "--number",
            "66",
            "--filenames",
            "extra",
            "file",
            "names",
            nullptr
        };
        int const sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 66);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "strange");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "file");
        CATCH_REQUIRE(opt.get_string("filenames", 5) == "names");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 6);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // repeat with ADVGETOPT_TEST_OPTIONS instead of a configuration file
    {
        // here we have verbose twice which should hit the no_argument case
        // in the add_option() function
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS= --verbose --number\t15\t--filenames foo bar blah --string weird -v");
        advgetopt::getopt opt(valid_options_no_confs, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 15);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "weird");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 3);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // test that the environment variable has priority over a configuration file
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=--number 501 --filenames more files"));

        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number=99\n"
                "string      =     strange\n"
                "verbose\n"
                "filenames =\tfoo\tbar \t blah\n"
            ;
        }
        advgetopt::getopt opt(valid_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 501);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "strange");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "more");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "files");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 5);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // test order: conf files, environment var, command line
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=--number 501 --filenames more files"));
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number=99\n"
                "string      =     strange\n"
                "verbose\n"
                "filenames =\tfoo\tbar \t blah\n"
            ;
        }

        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files",
            "--valid-parameter",
            "--string",
            "hard work",
            "--filenames",
            "extra",
            "file",
            "names",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 501);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "hard work");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "more");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "files");
        CATCH_REQUIRE(opt.get_string("filenames", 5) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames", 6) == "file");
        CATCH_REQUIRE(opt.get_string("filenames", 7) == "names");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 8);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // test again, just in case: conf files, environment var, command line
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=--number 709 --filenames more files --string \"hard work in env\""));
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number=99\n"
                "string      =     strange\n"
                "verbose\n"
                "filenames =\tfoo\tbar \t blah\n"
            ;
        }

        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files",
            "--valid-parameter",
            "--filenames",
            "extra",
            "file",
            "names",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 709);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "hard work in env");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "more");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "files");
        CATCH_REQUIRE(opt.get_string("filenames", 5) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames", 6) == "file");
        CATCH_REQUIRE(opt.get_string("filenames", 7) == "names");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 8);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }
}



// vim: ts=4 sw=4 et
