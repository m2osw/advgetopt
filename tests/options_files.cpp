/*
 * Files:
 *    tests/options_files.cpp
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


// C++ lib
//
#include <fstream>




CATCH_TEST_CASE("valid_options_files", "options")
{
    std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
    tmpdir += "/shared/advgetopt";
    std::stringstream ss;
    ss << "mkdir -p " << tmpdir;
    if(system(ss.str().c_str()) != 0)
    {
        std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
        exit(1);
    }
    std::string const options_filename(tmpdir + "/unittest.ini");

    // new set of options to test the special "--" option
    advgetopt::option const valid_options_from_file_list[] =
    {
        advgetopt::define_option(
              advgetopt::Name("verbose")
            , advgetopt::ShortName('v')
            , advgetopt::Flags(advgetopt::standalone_all_flags<>())
            , advgetopt::Help("a verbose like option, select it or not.")
        ),
        advgetopt::end_options()
    };

    advgetopt::options_environment valid_options_from_file;
    valid_options_from_file.f_project_name = "unittest";
    valid_options_from_file.f_options = valid_options_from_file_list;
    valid_options_from_file.f_options_files_directory = tmpdir.c_str();
    valid_options_from_file.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
    valid_options_from_file.f_help_header = "Usage: test valid options from file";

    // yet again, just in case: conf files, environment var, command line
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=--verbose"
                                                    " --more purple"
                                                    " --files left.txt center.txt right.txt"
                                                    " --from"
                                                    " --output destination.txt"));
        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[more]\n"
                "shortname=m\n"
                "default='More Stuff'\n"
                "help=Allow for more stuff to be added\n"
                "validator=regex(\"purple|yellow|blue|red|green|orange|brown\")\n"
                "allowed=command-line,environment-variable,configuration-file\n"
                "show-usage-on-error\n"
                "required\n"

                "[files]\n"
                "shortname=f\n"
                "help=List of file names\n"
                "validator=/.*\\.txt/i\n"
                "allowed=command-line,environment-variable\n"
                "multiple\n"
                "required\n"

                "[from]\n"
                "shortname=F\n"
                "help=request for the geographcal location representing the origin of the files\n"
                "validator=integer\n"
                "allowed=environment-variable,configuration-file\n"
                "no-arguments\n"

                "[output]\n"
                "shortname=o\n"
                "default=a.out\n"
                "help=output file\n"
                "allowed=environment-variable\n"
                "required\n"

                "[license]\n"
                "shortname=l\n"
                "help=show this test license\n"
                "allowed=command-line\n"
                "no-arguments\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_options_files",
            "--verbose",
            "--license",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        unittest::push_expected_log("error: option --license is not supported.");
        advgetopt::getopt opt(valid_options_from_file, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // "--more"
        CATCH_REQUIRE(opt.is_defined("more"));
        CATCH_REQUIRE(opt.get_string("more") == "purple");
        CATCH_REQUIRE(opt.get_default("more") == "More Stuff");
        CATCH_REQUIRE(opt.size("more") == 1);

        // "--files"
        CATCH_REQUIRE(opt.is_defined("files"));
        CATCH_REQUIRE(opt.get_string("files") == "left.txt");
        CATCH_REQUIRE(opt.get_string("files", 0) == "left.txt");
        CATCH_REQUIRE(opt.get_string("files", 1) == "center.txt");
        CATCH_REQUIRE(opt.get_string("files", 2) == "right.txt");
        CATCH_REQUIRE(opt.get_default("files").empty());
        CATCH_REQUIRE(opt.size("files") == 3);

        // "--from"
        CATCH_REQUIRE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.get_string("from") == "");
        CATCH_REQUIRE(opt.get_default("from").empty());
        CATCH_REQUIRE(opt.size("from") == 1);

        // "--output"
        CATCH_REQUIRE(opt.is_defined("output"));
        CATCH_REQUIRE(opt.get_string("output") == "destination.txt"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("output",  0) == "destination.txt");
        CATCH_REQUIRE(opt.get_default("output") == "a.out");
        CATCH_REQUIRE(opt.size("output") == 1);

        // "--from"
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_string("license") == "");
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_options_files");
    }
}









// vim: ts=4 sw=4 et
