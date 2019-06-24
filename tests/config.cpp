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
#include <advgetopt/exception.h>

// snapdev lib
//
#include <snapdev/safe_setenv.h>

// C++ lib
//
#include <fstream>








CATCH_TEST_CASE("valid_config_files", "[config][getopt]")
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
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE,
            "valid-parameter",
            nullptr,
            "a valid option",
            nullptr,
            nullptr
        },
        {
            'v',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "a verbose like option, select it or not",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "not-specified",
            nullptr,
            "a verbose like option, but never specified anywhere",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "number",
            "111",
            "expect a valid number",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "string",
            "the default string",
            "expect a valid string",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_MULTIPLE,
            "filenames",
            "a.out",
            "expect multiple strings",
            nullptr,
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
        CATCH_REQUIRE_FALSE(opt.has_default("valid-parameter"));
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
        CATCH_REQUIRE_FALSE(opt.has_default("valid-parameter"));
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
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", " --verbose --number\t15\t--filenames foo bar blah --string weird -v");
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
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--number 501 --filenames more files");

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
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--number 501 --filenames more files");
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
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--number 709 --filenames more files --string \"hard work in env\"");
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



CATCH_TEST_CASE("config_files", "[config][valid]")
{
    //std::vector<std::string> empty_confs;

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

    char const * confs[] =
    {
        config_filename.c_str(),
        nullptr
    };

    char const * separator_spaces[] =
    {
        " ",
        "\t",
        nullptr
    };

    // new set of options to test the special "--" option
    advgetopt::option const valid_options_with_multiple_list[] =
    {
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE,
            "valid-parameter",
            nullptr,
            "a valid option",
            nullptr,
            nullptr
        },
        {
            'v',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "a verbose like option, select it or not",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "not-specified",
            nullptr,
            "a verbose like option, but never specified anywhere",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "number",
            "111",
            "expect a valid number",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "string",
            "the default string",
            "expect a valid string",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
            "filenames",
            "a.out",
            "expect multiple strings, may be used after the -- and - is added to it too",
            nullptr,
            separator_spaces
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment valid_options_with_multiple;
    valid_options_with_multiple.f_project_name = "unittest";
    valid_options_with_multiple.f_options = valid_options_with_multiple_list;
    valid_options_with_multiple.f_help_header = "Usage: test valid options with multiple";
    valid_options_with_multiple.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
    valid_options_with_multiple.f_configuration_files = confs;

    // yet again, just in case: conf files, environment var, command line
    {
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "- --verbose -- more files --string \"hard work in env\"");
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number      =\t\t\t\t1111\t\t\t\t\n"
                "string      =     strange    \n"
                " filenames =\tfoo\tbar \t blah \n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
            "--valid-parameter",
            "--",
            "extra",
            "-file",
            "names",
            "-", // copied as is since we're after --
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options_with_multiple, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 1111);
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
        CATCH_REQUIRE(opt.get_string("filenames",  0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames",  1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames",  2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames",  3) == "-");
        CATCH_REQUIRE(opt.get_string("filenames",  4) == "more");
        CATCH_REQUIRE(opt.get_string("filenames",  5) == "files");
        CATCH_REQUIRE(opt.get_string("filenames",  6) == "--string");
        CATCH_REQUIRE(opt.get_string("filenames",  7) == "hard work in env");
        CATCH_REQUIRE(opt.get_string("filenames",  8) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames",  9) == "-file");
        CATCH_REQUIRE(opt.get_string("filenames", 10) == "names");
        CATCH_REQUIRE(opt.get_string("filenames", 11) == "-");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 12);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
    }

    // another one with some quotes
    {
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "- --verbose -- 'more files' --string \"hard work in env\"");
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number      =\t\t\t\t1111\t\t\t\t\n"
                "string      =     strange    \n"
                " filenames =\tfoo\t\"bar tender\" \t' blah '\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
            "--valid-parameter",
            "--",
            "'extra stuff '",
            "-file",
            "\"long names\"",
            "-", // copied as is since we're after --
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options_with_multiple, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 1111);
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
        CATCH_REQUIRE(opt.get_string("filenames",  0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames",  1) == "bar tender");
        CATCH_REQUIRE(opt.get_string("filenames",  2) == " blah ");
        CATCH_REQUIRE(opt.get_string("filenames",  3) == "-");
        CATCH_REQUIRE(opt.get_string("filenames",  4) == "more files");
        CATCH_REQUIRE(opt.get_string("filenames",  5) == "--string");
        CATCH_REQUIRE(opt.get_string("filenames",  6) == "hard work in env");
        CATCH_REQUIRE(opt.get_string("filenames",  7) == "'extra stuff '");
        CATCH_REQUIRE(opt.get_string("filenames",  8) == "-file");
        CATCH_REQUIRE(opt.get_string("filenames",  9) == "\"long names\"");
        CATCH_REQUIRE(opt.get_string("filenames", 10) == "-");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 11);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
    }

    // check that multiple flags can be used one after another
    advgetopt::option const valid_short_options_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: test valid short options",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            'a',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
            "a-not-in-v2",
            nullptr,
            "letter option",
            nullptr
        },
        {
            'c',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "c-not-in-v2",
            nullptr,
            "letter option",
            nullptr
        },
        {
            'd',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "d-not-in-v2",
            nullptr,
            "letter option",
            nullptr
        },
        {
            'f',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "f-not-in-v2",
            nullptr,
            "another letter",
            nullptr
        },
        {
            'r',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
            "r-not-in-v2",
            nullptr,
            "another letter",
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

    advgetopt::options_environment valid_short_options;
    valid_short_options.f_project_name = "unittest";
    valid_short_options.f_options = valid_short_options_list;
    valid_short_options.f_help_header = "Usage: test valid short options";
    valid_short_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    // test that we can use -cafard as expected
    {
        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
            "-cafard",
            "alpha",
            "-",
            "recurse",
            nullptr
        };
        int const sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_short_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // 2x 'a' in cafard, but we only keep the last entry
        CATCH_REQUIRE(opt.is_defined("a"));
        CATCH_REQUIRE(opt.get_string("a") == "-");
        CATCH_REQUIRE(opt.get_string("a", 0) == "-");
        CATCH_REQUIRE(opt.get_default("a").empty());
        CATCH_REQUIRE(opt.size("a") == 1);

        // c
        CATCH_REQUIRE(opt.is_defined("c"));
        CATCH_REQUIRE(opt.get_string("c") == "");
        CATCH_REQUIRE(opt.get_default("c").empty());
        CATCH_REQUIRE(opt.size("c") == 1);

        // d
        CATCH_REQUIRE(opt.is_defined("d"));
        CATCH_REQUIRE(opt.get_string("d") == "");
        CATCH_REQUIRE(opt.get_default("d").empty());
        CATCH_REQUIRE(opt.size("d") == 1);

        // f
        CATCH_REQUIRE(opt.is_defined("f"));
        CATCH_REQUIRE(opt.get_string("f") == "");
        CATCH_REQUIRE(opt.get_default("f").empty());
        CATCH_REQUIRE(opt.size("f") == 1);

        // r
        CATCH_REQUIRE(opt.is_defined("r"));
        CATCH_REQUIRE(opt.get_string("r") == "recurse");
        CATCH_REQUIRE(opt.get_string("r", 0) == "recurse");
        CATCH_REQUIRE(opt.get_default("r").empty());
        CATCH_REQUIRE(opt.size("r") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
    }

    // check that an optional option gets its default value if no arguments
    // were specified on the command line
    {
        // we need options with a --filenames that is optional
        const advgetopt::option valid_options_with_optional_filenames_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: test valid options with optional filenames",
            //    advgetopt::argument_mode_t::help_argument
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
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "filenames",
                "a.out",
                "expect multiple strings",
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

        advgetopt::options_environment valid_options_with_optional_filenames;
        valid_options_with_optional_filenames.f_project_name = "unittest";
        valid_options_with_optional_filenames.f_options = valid_options_with_optional_filenames_list;
        valid_options_with_optional_filenames.f_help_header = "Usage: test valid short options";
        valid_options_with_optional_filenames.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            // first try with that option by itself
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
                "--valid-parameter",
                "optional argument",
                "--filenames",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            advgetopt::getopt opt(valid_options_with_optional_filenames, sub_argc, sub_argv);

            // check that the result is valid

            // an invalid parameter, MUST NOT EXIST
            CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

            // valid parameter
            CATCH_REQUIRE(opt.is_defined("valid-parameter"));
            CATCH_REQUIRE(opt.get_string("valid-parameter") == "optional argument"); // same as index = 0
            CATCH_REQUIRE(opt.get_string("valid-parameter", 0) == "optional argument");
            CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
            CATCH_REQUIRE(opt.size("valid-parameter") == 1);

            // filenames
            CATCH_REQUIRE(opt.is_defined("filenames"));
            CATCH_REQUIRE(opt.get_string("filenames").empty()); // same as index = 0
            CATCH_REQUIRE(opt.get_string("filenames", 0).empty());
            CATCH_REQUIRE(opt.has_default("filenames"));
            CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
            CATCH_REQUIRE(opt.size("filenames") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
            CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
        }
        {
            // try again with a -v after the --filenames without filenames
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
                "--filenames",
                "-v",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            advgetopt::getopt opt(valid_options_with_optional_filenames, sub_argc, sub_argv);

            // check that the result is valid

            // an invalid parameter, MUST NOT EXIST
            CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

            // filenames
            CATCH_REQUIRE(opt.is_defined("filenames"));
            CATCH_REQUIRE(opt.get_string("filenames").empty()); // same as index = 0
            CATCH_REQUIRE(opt.get_string("filenames", 0).empty());
            CATCH_REQUIRE(opt.has_default("filenames"));
            CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
            CATCH_REQUIRE(opt.size("filenames") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
            CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
        }
    }

    // strange entry without a name
    {
        const advgetopt::option options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (strange entry without a name)",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR | advgetopt::GETOPT_FLAG_MULTIPLE,
                nullptr, // no name
                "README",
                nullptr, // no help
                nullptr
            },
            {
                'v',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
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
        options.f_help_header = "Usage: try this one and we get a throw (strange entry without a name)";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            const char *cargv2[] =
            {
                "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/no-name-arg-defaults-to-dash-dash",
                "-v",
                "advgetopt.cpp",
                nullptr
            };
            const int argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char **argv2 = const_cast<char **>(cargv2);

            // this initialization works as expected
            {
                CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(options, argc2, argv2), advgetopt::getopt_exception_logic);
            }

#if 0
// it throws, so no results...
            // check that the result is valid

            // an invalid parameter, MUST NOT EXIST
            CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

            // verbose
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_string("verbose") == ""); // same as index = 0
            CATCH_REQUIRE(opt.get_string("verbose", 0) == "");
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // the no name parameter!?
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "advgetopt.cpp"); // same as index = 0
            CATCH_REQUIRE(opt.get_string("--", 0) == "advgetopt.cpp");
            CATCH_REQUIRE(opt.get_default("--") == "README");
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "no-name-arg-defaults-to-dash-dash");
            CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/no-name-arg-defaults-to-dash-dash");
#endif
        }
    }
}


// vim: ts=4 sw=4 et
