/*
 * Files:
 *    tests/options_parser.cpp
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

// C++ lib
//
#include <fstream>





CATCH_TEST_CASE("options_parser", "[options][valid]")
{
    CATCH_START_SECTION("System options only")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test valid options from system options only";

        char const * cargv[] =
        {
            "tests/options-parser",
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
        CATCH_REQUIRE(opt.get_option("verbose") == nullptr);
        CATCH_REQUIRE(opt.get_option('v') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 0);

        // "--help"
        CATCH_REQUIRE(opt.get_option("help") != nullptr);
        CATCH_REQUIRE(opt.get_option('h') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("help"));
        CATCH_REQUIRE(opt.get_default("help").empty());
        CATCH_REQUIRE(opt.size("help") == 0);

        // "--version"
        CATCH_REQUIRE(opt.get_option("version") != nullptr);
        CATCH_REQUIRE(opt.get_option('V') != nullptr);
        CATCH_REQUIRE(opt.get_option('V') == opt.get_option("version"));
        CATCH_REQUIRE_FALSE(opt.is_defined("version"));
        CATCH_REQUIRE(opt.get_default("version").empty());
        CATCH_REQUIRE(opt.size("version") == 0);

        // "--copyright"
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // "--license"
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_string("license").empty());
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // "--build-date"
        CATCH_REQUIRE(opt.get_option("build-date") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("build-date"));
        CATCH_REQUIRE(opt.get_default("build-date").empty());
        CATCH_REQUIRE(opt.size("build-date") == 0);

        // "--environment-variable-name"
        CATCH_REQUIRE(opt.get_option("environment-variable-name") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 0);

        // "--configuration-filename"
        CATCH_REQUIRE(opt.get_option("configuration-filenames") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 0);

        // "--path-to-option-definitions"
        CATCH_REQUIRE(opt.get_option("path-to-option-definitions") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_default("path-to-option-definitions").empty());
        CATCH_REQUIRE(opt.size("path-to-option-definitions") == 0);


        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "options-parser");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/options-parser");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Duplicated options")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('V')        // duplicate version short name
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("copyright")     // duplicate copyright
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test valid options with duplicates";

        char const * cargv[] =
        {
            "options-parser",
            "--verbose",
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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('V') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // "--help"
        CATCH_REQUIRE(opt.get_option("help") != nullptr);
        CATCH_REQUIRE(opt.get_option('h') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("help"));
        CATCH_REQUIRE(opt.get_default("help").empty());
        CATCH_REQUIRE(opt.size("help") == 0);

        // "--version"
        CATCH_REQUIRE(opt.get_option("version") != nullptr);
        CATCH_REQUIRE(opt.get_option('V') != nullptr);      // 'V' is defined, but it's for "verbose"...
        CATCH_REQUIRE(opt.get_option('V') != opt.get_option("version"));
        CATCH_REQUIRE(opt.get_option('V') == opt.get_option("verbose"));
        CATCH_REQUIRE_FALSE(opt.is_defined("version"));
        CATCH_REQUIRE(opt.get_default("version").empty());
        CATCH_REQUIRE(opt.size("version") == 0);

        // "--copyright"
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') == nullptr);      // no short name in our definition (which overwrites the system definition)
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // "--license"
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_string("license").empty());
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // "--build-date"
        CATCH_REQUIRE(opt.get_option("build-date") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("build-date"));
        CATCH_REQUIRE(opt.get_default("build-date").empty());
        CATCH_REQUIRE(opt.size("build-date") == 0);

        // "--environment-variable-name"
        CATCH_REQUIRE(opt.get_option("environment-variable-name") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 0);

        // "--configuration-filename"
        CATCH_REQUIRE(opt.get_option("configuration-filenames") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 0);

        // "--path-to-option-definitions"
        CATCH_REQUIRE(opt.get_option("path-to-option-definitions") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_default("path-to-option-definitions").empty());
        CATCH_REQUIRE(opt.size("path-to-option-definitions") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "options-parser");
        CATCH_REQUIRE(opt.get_program_fullname() == "options-parser");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Default option")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filenames")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("enter a list of filenames.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test valid options with duplicates";

        char const * cargv[] =
        {
            "/usr/bin/options-parser",
            "file1",
            "file2",
            "file3",
            "file4",
            "file5",
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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 0);

        // "--help"
        CATCH_REQUIRE(opt.get_option("help") == nullptr);
        CATCH_REQUIRE(opt.get_option('h') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("help"));
        CATCH_REQUIRE(opt.get_default("help").empty());
        CATCH_REQUIRE(opt.size("help") == 0);

        // "--version"
        CATCH_REQUIRE(opt.get_option("version") == nullptr);
        CATCH_REQUIRE(opt.get_option('V') == nullptr);      // 'V' is defined, but it's for "verbose"...
        CATCH_REQUIRE_FALSE(opt.is_defined("version"));
        CATCH_REQUIRE(opt.get_default("version").empty());
        CATCH_REQUIRE(opt.size("version") == 0);

        // "--copyright"
        CATCH_REQUIRE(opt.get_option("copyright") == nullptr);
        CATCH_REQUIRE(opt.get_option('C') == nullptr);      // no short name in our definition (which overwrites the system definition)
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // "--license"
        CATCH_REQUIRE(opt.get_option("license") == nullptr);
        CATCH_REQUIRE(opt.get_option('L') == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 0);

        // "--build-date"
        CATCH_REQUIRE(opt.get_option("build-date") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("build-date"));
        CATCH_REQUIRE(opt.get_default("build-date").empty());
        CATCH_REQUIRE(opt.size("build-date") == 0);

        // "--environment-variable-name"
        CATCH_REQUIRE(opt.get_option("environment-variable-name") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 0);

        // "--configuration-filename"
        CATCH_REQUIRE(opt.get_option("configuration-filenames") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 0);

        // "--path-to-option-definitions"
        CATCH_REQUIRE(opt.get_option("path-to-option-definitions") == nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_default("path-to-option-definitions").empty());
        CATCH_REQUIRE(opt.size("path-to-option-definitions") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "options-parser");
        CATCH_REQUIRE(opt.get_program_fullname() == "/usr/bin/options-parser");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Alias option")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("licence")    // to allow French spelling
                , advgetopt::Alias("license")
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test valid options with duplicates";

        char const * cargv[] =
        {
            "options-parser",
            "--verbose",
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
        CATCH_REQUIRE(opt.get_option("verbose") != nullptr);
        CATCH_REQUIRE(opt.get_option('v') != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // "--help"
        CATCH_REQUIRE(opt.get_option("help") != nullptr);
        CATCH_REQUIRE(opt.get_option('h') != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("help"));
        CATCH_REQUIRE(opt.get_default("help").empty());
        CATCH_REQUIRE(opt.size("help") == 0);

        // "--version"
        CATCH_REQUIRE(opt.get_option("version") != nullptr);
        CATCH_REQUIRE(opt.get_option('V') != nullptr);      // 'V' is defined, but it's for "verbose"...
        CATCH_REQUIRE(opt.get_option('V') == opt.get_option("version"));
        CATCH_REQUIRE(opt.get_option('V') != opt.get_option("verbose"));
        CATCH_REQUIRE_FALSE(opt.is_defined("version"));
        CATCH_REQUIRE(opt.get_default("version").empty());
        CATCH_REQUIRE(opt.size("version") == 0);

        // "--copyright"
        CATCH_REQUIRE(opt.get_option("copyright") != nullptr);
        CATCH_REQUIRE(opt.get_option('C') != nullptr);      // no short name in our definition (which overwrites the system definition)
        CATCH_REQUIRE_FALSE(opt.is_defined("copyright"));
        CATCH_REQUIRE(opt.get_default("copyright").empty());
        CATCH_REQUIRE(opt.size("copyright") == 0);

        // "--license"
        CATCH_REQUIRE(opt.get_option("license") != nullptr);
        CATCH_REQUIRE(opt.get_option('L') != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        CATCH_REQUIRE(opt.get_string("license").empty());
        CATCH_REQUIRE(opt.get_default("license").empty());
        CATCH_REQUIRE(opt.size("license") == 1);

        // "--build-date"
        CATCH_REQUIRE(opt.get_option("build-date") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("build-date"));
        CATCH_REQUIRE(opt.get_default("build-date").empty());
        CATCH_REQUIRE(opt.size("build-date") == 0);

        // "--environment-variable-name"
        CATCH_REQUIRE(opt.get_option("environment-variable-name") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("environment-variable-name"));
        CATCH_REQUIRE(opt.get_default("environment-variable-name").empty());
        CATCH_REQUIRE(opt.size("environment-variable-name") == 0);

        // "--configuration-filename"
        CATCH_REQUIRE(opt.get_option("configuration-filenames") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("configuration-filenames"));
        CATCH_REQUIRE(opt.get_default("configuration-filenames").empty());
        CATCH_REQUIRE(opt.size("configuration-filenames") == 0);

        // "--path-to-option-definitions"
        CATCH_REQUIRE(opt.get_option("path-to-option-definitions") != nullptr);
        CATCH_REQUIRE_FALSE(opt.is_defined("path-to-option-definitions"));
        CATCH_REQUIRE(opt.get_default("path-to-option-definitions").empty());
        CATCH_REQUIRE(opt.size("path-to-option-definitions") == 0);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "options-parser");
        CATCH_REQUIRE(opt.get_program_fullname() == "options-parser");
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("invalid_options_parser", "[options][invalid]")
{
    CATCH_START_SECTION("No options")
        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: test detection of no options available at all";

        char const * cargv[] =
        {
            "tests/no-options-parser",
            "--missing",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Options without a name (null pointer)")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            {
                // we have to enter this manually because define_option()
                // forces you to enter a name, with C++17, we will also
                // be able to verify the whole table at compile time
                //
                '\0',
                advgetopt::GETOPT_FLAG_FLAG,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
            },
            advgetopt::define_option(
                  advgetopt::Name("licence")    // to allow French spelling
                , advgetopt::Alias("license")
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: name is nullptr";

        char const * cargv[] =
        {
            "tests/option-without-a-name",
            "--missing-name",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Options without a name (empty string)")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            {
                // we have to enter this manually because define_option()
                // forces you to enter a name, with C++17, we will also
                // be able to verify the whole table at compile time
                //
                '\0',
                advgetopt::GETOPT_FLAG_FLAG,
                "",
                nullptr,
                nullptr,
                nullptr,
            },
            advgetopt::define_option(
                  advgetopt::Name("licence")
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: name has a string but it's empty";

        char const * cargv[] =
        {
            "tests/option-without-a-name",
            "--missing-name",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Options with a one letter name")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            {
                // we have to enter this manually because define_option()
                // forces you to enter a name, with C++17, we will also
                // be able to verify the whole table at compile time
                //
                '\0',
                advgetopt::GETOPT_FLAG_FLAG,
                "h",
                nullptr,
                nullptr,
                nullptr,
            },
            advgetopt::define_option(
                  advgetopt::Name("licence")
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: name is only one letter";

        char const * cargv[] =
        {
            "tests/option-with-name-too-short",
            "--missing-name",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Duplicated Options (Long Name)")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("licence")
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::define_option(
                  advgetopt::Name("licence") // duplicate
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: one name can't be redefined";

        char const * cargv[] =
        {
            "tests/duplicated-option",
            "--missing-name",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Duplicated Options (short name)")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("look")
                , advgetopt::ShortName('l')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::define_option(
                  advgetopt::Name("lock")
                , advgetopt::ShortName('l') // duplicate
                , advgetopt::Flags(advgetopt::standalone_command_flags())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: one name can't be redefined";

        char const * cargv[] =
        {
            "tests/duplicated-option",
            "--missing-name",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Duplicated Default Options")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("ins")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("outs")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: one name can't be redefined";

        char const * cargv[] =
        {
            "tests/duplicated-option",
            "--missing-name",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Default Option marked as being a FLAG")
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::standalone_command_flags())
                , advgetopt::Help("print info as we work.")
            ),
            // the define_option() already catches this error at compile time
            {
                  'o'
                , advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION | advgetopt::GETOPT_FLAG_FLAG
                , "output"
                , nullptr
                , nullptr
                , nullptr
            },
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = 0;
        environment_options.f_help_header = "Usage: one name can't be redefined";

        char const * cargv[] =
        {
            "tests/duplicated-option",
            "--missing-name",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        CATCH_REQUIRE_THROWS_AS(std::make_shared<advgetopt::getopt>(environment_options, argc, argv), advgetopt::getopt_exception_logic);
    CATCH_END_SECTION()
}









// vim: ts=4 sw=4 et
