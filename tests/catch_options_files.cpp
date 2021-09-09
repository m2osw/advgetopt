// Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved
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


// snapdev lib
//
#include    <snapdev/safe_setenv.h>


// C++ lib
//
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>




CATCH_TEST_CASE("valid_options_files", "[options][valid][files]")
{
    CATCH_START_SECTION("Check the default path with a nullptr (not a very good test, though)")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_env;
        options_env.f_project_name = "this-is-the-name-of-a-test-project-which-wont-ever-exist";
        options_env.f_options = options;
        options_env.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options_env.f_help_header = "Usage: test valid options from file";

        char const * sub_cargv[] =
        {
            "tests/unittests/no_file_to_load",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(options_env, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "no_file_to_load");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/no_file_to_load");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check the default path with an empty string (not a very good test, though)")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_env;
        options_env.f_project_name = "this-is-the-name-of-a-test-project-which-wont-ever-exist";
        options_env.f_options = options;
        options_env.f_options_files_directory = "";
        options_env.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options_env.f_help_header = "Usage: test valid options from file";

        char const * sub_cargv[] =
        {
            "tests/unittests/no_file_to_load",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(options_env, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "no_file_to_load");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/no_file_to_load");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check the parsing of a valid options.ini file")
        // create a file and make sure it's not read if the project name
        // is empty
        //
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/no-project-name.ini");

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
        valid_options_from_file.f_project_name = nullptr;
        valid_options_from_file.f_options = valid_options_from_file_list;
        valid_options_from_file.f_options_files_directory = tmpdir.c_str();
        valid_options_from_file.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        valid_options_from_file.f_help_header = "Usage: test valid options from file";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[no-project-name]\n"
                "shortname=n\n"
                "default='inexistent'\n"
                "help=Testing that this doesn't get loaded\n"
                "allowed=command-line,environment-variable,configuration-file\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/file_not_loaded",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options_from_file, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // "--no-project-name"
        CATCH_REQUIRE(opt.get_option("no-project-name") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("no-project-name"));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "file_not_loaded");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/file_not_loaded");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Project name is an empty string")
        // create a file and make sure it's not read if the project name
        // is empty
        //
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/empty-string.ini");

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
        valid_options_from_file.f_project_name = "";
        valid_options_from_file.f_options = valid_options_from_file_list;
        valid_options_from_file.f_options_files_directory = tmpdir.c_str();
        valid_options_from_file.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        valid_options_from_file.f_help_header = "Usage: test valid options from file";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[no-project-name]\n"
                "shortname=n\n"
                "default='inexistent'\n"
                "help=Testing that this doesn't get loaded\n"
                "allowed=command-line,environment-variable,configuration-file\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/file_not_loaded",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options_from_file, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // "--no-project-name"
        CATCH_REQUIRE(opt.get_option("no-project-name") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("no-project-name"));

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "file_not_loaded");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/file_not_loaded");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check the parsing of a valid options.ini file")
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

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS"
                            , "--verbose"
                             " --more purple"
                             " -f left.txt center.txt right.txt"
                             " --size 519"
                             " --from"
                             " --output destination.txt");

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

                "[size]\n"
                "shortname=s\n"
                "help=Specify the size\n"
                "validator=/[0-9]+/\n"
                "allowed=environment-variable,configuration-file\n"
                "default=31\n"
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
                "help=Request for the geographcal location representing the origin of the files; optionally you can specify the format\n"
                "validator=integer\n"
                "allowed=command-line,environment-variable,configuration-file\n"

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

                "[licence]\n"
                "alias=license\n"
                "allowed=command-line\n"
                "no-arguments\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/valid_options_files",
            "--verbose",
            "--licence",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options_from_file, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
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

        // "--size <value>"
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "519");
        CATCH_REQUIRE(opt.get_string("size", 0) == "519");
        CATCH_REQUIRE(opt.get_default("size") == "31");
        CATCH_REQUIRE(opt.size("size") == 1);
        CATCH_REQUIRE(opt.get_long("size") == 519);

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
        CATCH_REQUIRE(opt.size("from") == 1);
        CATCH_REQUIRE(opt.get_string("from") == "");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number () in parameter --from at offset 0.");
        CATCH_REQUIRE(opt.get_long("from") == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(opt.get_default("from").empty());

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
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");

        char const * sub_cargv2[] =
        {
            "this/is/ignored",
            "--from",
            "1001",
            nullptr
        };
        int const sub_argc2(sizeof(sub_cargv2) / sizeof(sub_cargv2[0]) - 1);
        char ** sub_argv2 = const_cast<char **>(sub_cargv2);

        opt.parse_arguments(sub_argc2, sub_argv2, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        // "--from"
        CATCH_REQUIRE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.size("from") == 1);
        CATCH_REQUIRE(opt.get_string("from") == "1001");
        CATCH_REQUIRE(opt.get_long("from") == 1001);
        CATCH_REQUIRE(opt.get_default("from").empty());

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");

        // keep the last value...
        //
        opt.parse_environment_variable();

        // "--from"
        CATCH_REQUIRE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.size("from") == 1);
        CATCH_REQUIRE(opt.get_string("from") == "");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number () in parameter --from at offset 0.");
        CATCH_REQUIRE(opt.get_long("from") == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(opt.get_default("from").empty());

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");

        // a reset will restore the state
        //
        opt.reset();

        // the valid parameter
        CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 0);

        // "--from"
        CATCH_REQUIRE_FALSE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.get_default("from").empty());
        CATCH_REQUIRE(opt.size("from") == 0);

        opt.parse_environment_variable();
        opt.parse_arguments(sub_argc2, sub_argv2, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        // "--from"
        CATCH_REQUIRE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.get_string("from") == "1001");
        CATCH_REQUIRE(opt.get_long("from") == 1001);
        CATCH_REQUIRE(opt.get_default("from").empty());
        CATCH_REQUIRE(opt.size("from") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");


        // test that the validators do work here (i.e. generate errors as
        // expected when we use the wrong options.)
        //
        {
            snap::safe_setenv subenv("ADVGETOPT_TEST_OPTIONS"
                                , "--verbose"
                                 " --size '1001 meters'"
                                 " -f valid.cpp"
                                 " --from auto-build"
                                 " --more black");

            SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"1001 meters\" given to parameter --size is not considered valid.");
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"valid.cpp\" given to parameter --files is not considered valid.");
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"auto-build\" given to parameter --from is not considered valid.");
            SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"black\" given to parameter --more is not considered valid.");
            opt.parse_environment_variable();
            SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check with validators in the definition")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt-validators-in-table";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/unittest.ini");

        advgetopt::option const valid_options_from_file_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("Specify the size.")
                , advgetopt::Validator("integer(0...100)")
                , advgetopt::DefaultValue("31")
            ),
            advgetopt::define_option(
                  advgetopt::Name("files")
                , advgetopt::ShortName('f')
                , advgetopt::Help("List of file names")
                , advgetopt::Validator("/.*\\.txt/i")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("from")
                , advgetopt::ShortName('F')
                , advgetopt::Help("Request for the geographcal location representing the origin of the files; optionally you can specify the format")
                , advgetopt::Validator("integer")
                , advgetopt::Flags(advgetopt::all_flags<>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("more")
                , advgetopt::ShortName('m')
                , advgetopt::Help("Allow for more stuff to be added")
                , advgetopt::Validator("regex(\"purple|yellow|blue|red|green|orange|brown\")")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::DefaultValue("More Stuff")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment valid_options_from_file;
        valid_options_from_file.f_project_name = "unittest";
        valid_options_from_file.f_options = valid_options_from_file_list;
        valid_options_from_file.f_options_files_directory = tmpdir.c_str();
        valid_options_from_file.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        valid_options_from_file.f_help_header = "Usage: test valid options from file";

        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS"
                            , "--verbose"
                             " --more purple"
                             " -f left.txt center.txt right.txt"
                             " --size 19"
                             " --from"
                             " --output destination.txt");

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

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

                "[licence]\n"
                "alias=license\n"
                "allowed=command-line\n"
                "no-arguments\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/valid_options_files",
            "--verbose",
            "--licence",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options_from_file, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(opt.get_option("invalid-parameter") == nullptr);
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

        // "--size <value>"
        CATCH_REQUIRE(opt.is_defined("size"));
        CATCH_REQUIRE(opt.get_string("size") == "19");
        CATCH_REQUIRE(opt.get_string("size", 0) == "19");
        CATCH_REQUIRE(opt.get_default("size") == "31");
        CATCH_REQUIRE(opt.size("size") == 1);
        CATCH_REQUIRE(opt.get_long("size") == 19);

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
        CATCH_REQUIRE(opt.size("from") == 1);
        CATCH_REQUIRE(opt.get_string("from") == "");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number () in parameter --from at offset 0.");
        CATCH_REQUIRE(opt.get_long("from") == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number () in parameter --from at offset 0.");
        CATCH_REQUIRE(opt.get_long("from") == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(opt.get_default("from").empty());

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
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");

        char const * sub_cargv2[] =
        {
            "this/is/ignored",
            "--from",
            "1001",
            nullptr
        };
        int const sub_argc2(sizeof(sub_cargv2) / sizeof(sub_cargv2[0]) - 1);
        char ** sub_argv2 = const_cast<char **>(sub_cargv2);

        opt.parse_arguments(sub_argc2, sub_argv2, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        // "--from"
        CATCH_REQUIRE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.size("from") == 1);
        CATCH_REQUIRE(opt.get_string("from") == "1001");
        CATCH_REQUIRE(opt.get_long("from") == 1001);
        CATCH_REQUIRE(opt.get_default("from").empty());

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");

        // keep the last value...
        //
        opt.parse_environment_variable();

        // "--from"
        CATCH_REQUIRE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.size("from") == 1);
        CATCH_REQUIRE(opt.get_string("from") == "");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: invalid number () in parameter --from at offset 0.");
        CATCH_REQUIRE(opt.get_long("from") == -1);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
        CATCH_REQUIRE(opt.get_default("from").empty());

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");

        // a reset will restore the state
        //
        opt.reset();

        // the valid parameter
        CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 0);

        // "--from"
        CATCH_REQUIRE_FALSE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.get_default("from").empty());
        CATCH_REQUIRE(opt.size("from") == 0);

        opt.parse_environment_variable();
        opt.parse_arguments(sub_argc2, sub_argv2, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        // "--from"
        CATCH_REQUIRE(opt.is_defined("from"));
        CATCH_REQUIRE(opt.get_string("from") == "1001");
        CATCH_REQUIRE(opt.get_long("from") == 1001);
        CATCH_REQUIRE(opt.get_default("from").empty());
        CATCH_REQUIRE(opt.size("from") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "valid_options_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/valid_options_files");

        // test that the validators do work here (i.e. generate errors as
        // expected when we use the wrong options.)
        //
        char const * sub_cargv3[] =
        {
            "this/is/ignored",
            "--size",
            "1001",
            "-f",
            "valid.cpp",
            "--from",
            "51",
            "--more",
            "black",
            nullptr
        };
        int const sub_argc3(sizeof(sub_cargv3) / sizeof(sub_cargv3[0]) - 1);
        char ** sub_argv3 = const_cast<char **>(sub_cargv3);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"1001\" given to parameter --size is not considered valid.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"valid.cpp\" given to parameter --files is not considered valid.");
        SNAP_CATCH2_NAMESPACE::push_expected_log("error: input \"black\" given to parameter --more is not considered valid.");
        opt.parse_arguments(sub_argc3, sub_argv3, advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_options_files", "[options][invalid][files]")
{
    CATCH_START_SECTION("2+ section names")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/bad-section.ini");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_environment;
        options_environment.f_project_name = "bad-section";
        options_environment.f_options = options;
        options_environment.f_options_files_directory = tmpdir.c_str();
        options_environment.f_environment_variable_name = nullptr;
        options_environment.f_help_header = "Usage: test invalid section name";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[invalid::name]\n"
                "shortname=m\n"
                "default='Invalid Stuff'\n"
                "help=Testing that a section name can't include \"::\"\n"
                "allowed=command-line,environment-variable,configuration-file\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/invalid_name_in_options_ini",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"shortname\" cannot be added to"
                    " section \"invalid::name\" because this"
                    " configuration only accepts one section level.");
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"default\" cannot be added to"
                    " section \"invalid::name\" because this"
                    " configuration only accepts one section level.");
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"help\" cannot be added to"
                    " section \"invalid::name\" because this"
                    " configuration only accepts one section level.");
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"allowed\" cannot be added to"
                    " section \"invalid::name\" because this"
                    " configuration only accepts one section level.");
        advgetopt::getopt::pointer_t opt(std::make_shared<advgetopt::getopt>(options_environment, sub_argc, sub_argv));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(opt->size("invalid::name::shortname") == 0);
        CATCH_REQUIRE(opt->size("shortname") == 0);
    CATCH_END_SECTION()

    CATCH_START_SECTION("short name too long")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/bad-shortname.ini");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_environment;
        options_environment.f_project_name = "bad-shortname";
        options_environment.f_options = options;
        options_environment.f_options_files_directory = tmpdir.c_str();
        options_environment.f_environment_variable_name = nullptr;
        options_environment.f_help_header = "Usage: test invalid shortname";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[badname]\n"
                "shortname=to\n"
                "default='Invalid Stuff'\n"
                "help=Testing that a shotname can't be 2 characters or more\n"
                "allowed=command-line,environment-variable,configuration-file\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/invalid_name_in_options_ini",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        CATCH_REQUIRE_THROWS_MATCHES(std::make_shared<advgetopt::getopt>(options_environment, sub_argc, sub_argv)
                    , advgetopt::getopt_logic_error
                    , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: option \"badname\" has an invalid short name in \""
                            + options_filename
                            + "\", it can't be more than one character."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("missing ')' in validator specification")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/bad-validator-parenthesis.ini");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_environment;
        options_environment.f_project_name = "bad-validator-parenthesis";
        options_environment.f_options = options;
        options_environment.f_options_files_directory = tmpdir.c_str();
        options_environment.f_environment_variable_name = nullptr;
        options_environment.f_help_header = "Usage: test invalid validator specification";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[bad-validator]\n"
                "shortname=b\n"
                "default='Invalid Stuff'\n"
                "help=Testing that a validator with parenthesis must have the ')'\n"
                "validator=regex(\"missing ')'\"\n"
                "allowed=command-line,environment-variable,configuration-file\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/invalid_validator_specification",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        CATCH_REQUIRE_THROWS_MATCHES(std::make_shared<advgetopt::getopt>(options_environment, sub_argc, sub_argv)
                    , advgetopt::getopt_logic_error
                    , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: invalid validator parameter definition: \"regex(\"missing ')'\"\", the ')' is missing."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("alias with help")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/alias-with-help.ini");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_environment;
        options_environment.f_project_name = "alias-with-help";
        options_environment.f_options = options;
        options_environment.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        options_environment.f_options_files_directory = tmpdir.c_str();
        options_environment.f_environment_variable_name = nullptr;
        options_environment.f_help_header = "Usage: test invalid validator specification";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[licence]\n"
                "shortname=l\n"
                "default='Invalid Stuff'\n"
                "alias=license\n"
                "help=Testing that an alias can't accept a help string\n"
                "allowed=command-line,environment-variable,configuration-file\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/invalid_alias_specification",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        CATCH_REQUIRE_THROWS_MATCHES(std::make_shared<advgetopt::getopt>(options_environment, sub_argc, sub_argv)
                    , advgetopt::getopt_logic_error
                    , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: option \"licence\" is an alias and as such it can't include a help=... parameter in \""
                            + options_filename
                            + "\"."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("no-name alias")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/no-name-alias.ini");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_environment;
        options_environment.f_project_name = "no-name-alias";
        options_environment.f_options = options;
        options_environment.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        options_environment.f_options_files_directory = tmpdir.c_str();
        options_environment.f_environment_variable_name = nullptr;
        options_environment.f_help_header = "Usage: test alias with no name specified";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[foo]\n"
                "shortname=f\n"
                "default='Invalid Stuff'\n"
                "alias=\n"      // name missing (with an equal)
                "allowed=command-line\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/non_existant_alias",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        CATCH_REQUIRE_THROWS_MATCHES(std::make_shared<advgetopt::getopt>(options_environment, sub_argc, sub_argv)
                    , advgetopt::getopt_logic_error
                    , Catch::Matchers::ExceptionMessage("getopt_logic_error: the default value of your alias cannot be an empty string for \"foo\"."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("no-name alias v2")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/no-name-alias-v2.ini");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_environment;
        options_environment.f_project_name = "no-name-alias-v2";
        options_environment.f_options = options;
        options_environment.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        options_environment.f_options_files_directory = tmpdir.c_str();
        options_environment.f_environment_variable_name = nullptr;
        options_environment.f_help_header = "Usage: test alias with no name specified";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[foo]\n"
                "shortname=f\n"
                "default='Invalid Stuff'\n"
                "alias\n"      // name missing (no equal)
                "allowed=command-line\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/non_existant_alias",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        CATCH_REQUIRE_THROWS_MATCHES(std::make_shared<advgetopt::getopt>(options_environment, sub_argc, sub_argv)
                    , advgetopt::getopt_logic_error
                    , Catch::Matchers::ExceptionMessage("getopt_logic_error: the default value of your alias cannot be an empty string for \"foo\"."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("non-existant alias")
        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/shared/advgetopt";
        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }
        std::string const options_filename(tmpdir + "/non-existant-alias.ini");

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_all_flags<>())
                , advgetopt::Help("a verbose like option, select it or not.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options_environment;
        options_environment.f_project_name = "non-existant-alias";
        options_environment.f_options = options;
        options_environment.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        options_environment.f_options_files_directory = tmpdir.c_str();
        options_environment.f_environment_variable_name = nullptr;
        options_environment.f_help_header = "Usage: test invalid validator specification";

        {
            std::ofstream options_file;
            options_file.open(options_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(options_file.good());
            options_file <<
                "# Auto-generated\n"

                "[foo]\n"
                "shortname=f\n"
                "default='Invalid Stuff'\n"
                "alias=bar\n"       // option "bar" missing
                "allowed=command-line\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/non_existant_alias",
            "--verbose",
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        CATCH_REQUIRE_THROWS_MATCHES(std::make_shared<advgetopt::getopt>(options_environment, sub_argc, sub_argv)
                    , advgetopt::getopt_logic_error
                    , Catch::Matchers::ExceptionMessage("getopt_logic_error: no option named \"bar\" to satisfy the alias of \"foo\"."));
    CATCH_END_SECTION()
}









// vim: ts=4 sw=4 et
