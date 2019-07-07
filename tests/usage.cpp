/*
 * Files:
 *    tests/usage.cpp
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







CATCH_TEST_CASE("usage_function", "[getopt][usage]")
{
    CATCH_START_SECTION("usage() using \"--filename\" for the default option accepting multiple entries")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("validate")
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("this is used to validate different things.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("long")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("used to validate that invalid numbers generate an error.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("out-of-bounds")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("string parameter.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("unique")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("make sure everything is unique.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("noisy")
                , advgetopt::ShortName('n')
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                //, advgetopt::Help("make sure everything is unique.") -- do not show in --help
            ),
            advgetopt::define_option(
                  advgetopt::Name("quiet")
                , advgetopt::ShortName('q')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filename")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("other parameters are viewed as filenames.")
            ),
            advgetopt::end_options()
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

        const char * const configuration_files[] =
        {
            "advgetopt.conf",
            "advgetopt.ini",
            "advgetopt.xml",
            "advgetopt.yaml",
            nullptr
        };

        const char * const configuration_directories[] =
        {
            "/etc/sys/advgetopt",
            "/etc/advgetopt",
            "/etc/advgetopt/advgetopt.d",
            "~/.config/advgetopt",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = configuration_directories;
        options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls)";
        options.f_help_footer = "And this is the footer where we can include many parameters:\n"
                                "   . Percent = [%%]\n"
                                "   . Project Name = [%a]\n"
                                "   . Build Date = [%b]\n"
                                "   . Copyright = [%c]\n"
                                "   . Directories = [%d]\n"
                                "   . All Directories = [%*d]\n"
                                "   . Environment Variable = [%e]\n"
                                "   . Environment Variable and Value = [%*e]\n"
                                "   . Configuration Files = [%f]\n"
                                "   . All Configuration Files = [%*f]\n"
                                "   . License = [%l]\n"
                                "   . Program Name = [%p]\n"
                                "   . Program Fullname = [%*p]\n"
                                "   . Build Time = [%t]\n"
                                "   . Version = [%v]\n"
                ;
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1";
        options.f_license = "MIT";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved";
        options.f_build_date = "Jun  4 2019";
        options.f_build_time = "23:02:36";

        // this initialization works as expected
        //
        advgetopt::getopt opt(options, argc2, argv2);

  
        std::string const footer(
"\n"
"And this is the footer where we can include many parameters:\n"
"   . Percent = [%]\n"
"   . Project Name = [unittest]\n"
"   . Build Date = [Jun  4 2019]\n"
"   . Copyright = [Copyright (c) 2019  Made to Order Software Corp. -- All Rights\n"
"Reserved]\n"
"   . Directories = [/etc/sys/advgetopt]\n"
"   . All Directories = [/etc/sys/advgetopt, /etc/advgetopt,\n"
"/etc/advgetopt/advgetopt.d, ~/.config/advgetopt]\n"
"   . Environment Variable = [ADVGETOPT_TEST_OPTIONS]\n"
"   . Environment Variable and Value = [ADVGETOPT_TEST_OPTIONS (not set)]\n"
"   . Configuration Files = [advgetopt.conf]\n"
"   . All Configuration Files = [advgetopt.conf, advgetopt.ini, advgetopt.xml,\n"
"advgetopt.yaml]\n"
"   . License = [MIT]\n"
"   . Program Name = [unittest_advgetopt]\n"
"   . Program Fullname = [tests/unittests/unittest_advgetopt]\n"
"   . Build Time = [23:02:36]\n"
"   . Version = [2.0.1]\n"
"\n");

        // test a standard "--help" type of option
        //
        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --long <arg>               used to validate that invalid numbers generate an\n"
"                              error.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --string <arg>             string parameter.\n"
"   --unique or -u             make sure everything is unique.\n"
"   --validate                 this is used to validate different things.\n"
"   [default arguments]        other parameters are viewed as filenames.\n"
+ footer
                        );

        // test a "--help-all" type of option
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --long <arg>               used to validate that invalid numbers generate an\n"
"                              error.\n"
"   --not-specified-and-no-default <arg>\n"
"                              test long without having used the option and no\n"
"                              default.\n"
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test that an invalid default value can be returned\n"
"                              as is.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --string <arg>             string parameter.\n"
"   --unique or -u             make sure everything is unique.\n"
"   --validate                 this is used to validate different things.\n"
"   [default arguments]        other parameters are viewed as filenames.\n"
+ footer
                        );

        // pretend an error occurred
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --validate                 this is used to validate different things.\n"
+ footer
                        );

        // show GROUP1
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP1) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --not-specified-and-no-default <arg>\n"
"                              test long without having used the option and no\n"
"                              default.\n"
+ footer
                        );

        // show GROUP2
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP2) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test that an invalid default value can be returned\n"
"                              as is.\n"
+ footer
                        );
    CATCH_END_SECTION()

    CATCH_START_SECTION("usage() using \"--filename\" for the default option accepting multiple entries which are required when \"--filename\" is used")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("validate")
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("this is used to validate different things.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("long")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("used to validate that invalid numbers generate an error.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("out-of-bounds")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("string parameter.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("unique")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("make sure everything is unique.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("noisy")
                , advgetopt::ShortName('n')
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                //, advgetopt::Help("make sure everything is unique.") -- do not show in --help
            ),
            advgetopt::define_option(
                  advgetopt::Name("quiet")
                , advgetopt::ShortName('q')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filename")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("other parameters are viewed as filenames.")
            ),
            advgetopt::end_options()
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

        const char * const configuration_files[] =
        {
            "advgetopt.conf",
            "advgetopt.ini",
            "advgetopt.xml",
            "advgetopt.yaml",
            nullptr
        };

        const char * const configuration_directories[] =
        {
            "/etc/sys/advgetopt",
            "/etc/advgetopt",
            "/etc/advgetopt/advgetopt.d",
            "~/.config/advgetopt",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = configuration_directories;
        options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls)";
        options.f_help_footer = "And this is the footer where we can include many parameters:\n"
                                "   . Percent = [%%]\n"
                                "   . Project Name = [%a]\n"
                                "   . Build Date = [%b]\n"
                                "   . Copyright = [%c]\n"
                                "   . Directories = [%d]\n"
                                "   . All Directories = [%*d]\n"
                                "   . Environment Variable = [%e]\n"
                                "   . Environment Variable and Value = [%*e]\n"
                                "   . Configuration Files = [%f]\n"
                                "   . All Configuration Files = [%*f]\n"
                                "   . License = [%l]\n"
                                "   . Program Name = [%p]\n"
                                "   . Program Fullname = [%*p]\n"
                                "   . Build Time = [%t]\n"
                                "   . Version = [%v]\n"
                ;
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1";
        options.f_license = "MIT";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved";
        options.f_build_date = "Jun  4 2019";
        options.f_build_time = "23:02:36";

        // this initialization works as expected
        //
        advgetopt::getopt opt(options, argc2, argv2);

  
        std::string const footer(
"\n"
"And this is the footer where we can include many parameters:\n"
"   . Percent = [%]\n"
"   . Project Name = [unittest]\n"
"   . Build Date = [Jun  4 2019]\n"
"   . Copyright = [Copyright (c) 2019  Made to Order Software Corp. -- All Rights\n"
"Reserved]\n"
"   . Directories = [/etc/sys/advgetopt]\n"
"   . All Directories = [/etc/sys/advgetopt, /etc/advgetopt,\n"
"/etc/advgetopt/advgetopt.d, ~/.config/advgetopt]\n"
"   . Environment Variable = [ADVGETOPT_TEST_OPTIONS]\n"
"   . Environment Variable and Value = [ADVGETOPT_TEST_OPTIONS (not set)]\n"
"   . Configuration Files = [advgetopt.conf]\n"
"   . All Configuration Files = [advgetopt.conf, advgetopt.ini, advgetopt.xml,\n"
"advgetopt.yaml]\n"
"   . License = [MIT]\n"
"   . Program Name = [unittest_advgetopt]\n"
"   . Program Fullname = [tests/unittests/unittest_advgetopt]\n"
"   . Build Time = [23:02:36]\n"
"   . Version = [2.0.1]\n"
"\n");

        // test a standard "--help" type of option
        //
        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --long <arg>               used to validate that invalid numbers generate an\n"
"                              error.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --string <arg>             string parameter.\n"
"   --unique or -u             make sure everything is unique.\n"
"   --validate                 this is used to validate different things.\n"
"   <default arguments>        other parameters are viewed as filenames.\n"
+ footer
                        );

        // test a "--help-all" type of option
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --long <arg>               used to validate that invalid numbers generate an\n"
"                              error.\n"
"   --not-specified-and-no-default <arg>\n"
"                              test long without having used the option and no\n"
"                              default.\n"
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test that an invalid default value can be returned\n"
"                              as is.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --string <arg>             string parameter.\n"
"   --unique or -u             make sure everything is unique.\n"
"   --validate                 this is used to validate different things.\n"
"   <default arguments>        other parameters are viewed as filenames.\n"
+ footer
                        );

        // pretend an error occurred
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --validate                 this is used to validate different things.\n"
+ footer
                        );

        // show GROUP1
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP1) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --not-specified-and-no-default <arg>\n"
"                              test long without having used the option and no\n"
"                              default.\n"
+ footer
                        );

        // show GROUP2
        //
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP2) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test that an invalid default value can be returned\n"
"                              as is.\n"
+ footer
                        );
    CATCH_END_SECTION()

    CATCH_START_SECTION("usage() using \"--filename\" for the default option accepting one required item")
        // valid initialization + usage calls with a few different options
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("validate")
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("this is used to validate different things.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("long")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("used to validate that invalid numbers generate an error.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("out-of-bounds")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("string parameter.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("unique")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("make sure everything is unique.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("quiet")
                , advgetopt::ShortName('q')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-in-v2-though")
                , advgetopt::ShortName('l')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("long with just a letter.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filename")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("other parameters are viewed as filenames; and we need at least one option with a very long help to check that it wraps perfectly (we'd really need to get the output of the command and check that against what is expected because at this time the test is rather blind in that respect! FIXED IN v2!)")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls bis)";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

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
//                CATCH_REQUIRE_THROWS_AS(opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exception_exiting);
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
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test that an invalid default value can be returned\n"
"                              as is.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --string <arg>             string parameter.\n"
"   --unique or -u             make sure everything is unique.\n"
"   --validate                 this is used to validate different things.\n"
"   <default argument>         other parameters are viewed as filenames; and we\n"
"                              need at least one option with a very long help to\n"
"                              check that it wraps perfectly (we'd really need to\n"
"                              get the output of the command and check that\n"
"                              against what is expected because at this time the\n"
"                              test is rather blind in that respect! FIXED IN\n"
"                              v2!)\n"
                    );
    CATCH_END_SECTION()

    CATCH_START_SECTION("usage() using \"--\" for the default option accepting one item")
        // valid initialization + usage calls with a few different options
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("validate")
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("this is used to validate different things.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("long")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("used to validate that invalid numbers generate an error.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("out-of-bounds")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("string parameter.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("unique")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("make sure everything is unique.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("quiet")
                , advgetopt::ShortName('q')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("option-argument")
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("command line option which accepts an optional argument.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-in-v2-though")
                , advgetopt::ShortName('l')
                , advgetopt::Flags(advgetopt::optional_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("long with just a letter.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("--")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
                , advgetopt::Help("other parameters are viewed as filenames; and we need at least one option with a very long help to check that it wraps perfectly (we'd really need to get the output of the command and check that against what is expected because at this time the test is rather blind in that respect! FIXED IN v2!)")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls bis)";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

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
//                CATCH_REQUIRE_THROWS_AS(opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exception_exiting);
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
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test that an invalid default value can be returned\n"
"                              as is.\n"
"   --option-argument [<arg>]  command line option which accepts an optional\n"
"                              argument.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --string <arg>             string parameter.\n"
"   --unique or -u             make sure everything is unique.\n"
"   --validate                 this is used to validate different things.\n"
"   [default argument]         other parameters are viewed as filenames; and we\n"
"                              need at least one option with a very long help to\n"
"                              check that it wraps perfectly (we'd really need to\n"
"                              get the output of the command and check that\n"
"                              against what is expected because at this time the\n"
"                              test is rather blind in that respect! FIXED IN\n"
"                              v2!)\n"
                    );
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("help_string_percent", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Percent")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %%.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("this-is-a-very-long-argument-so-we-can-see-that-such-a-crazy-long-option-(who-does-that-though)-gets-broken-up-as-expected")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("I guess the help is not necessary now...")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %%";
        options.f_help_footer = "Percent Percent: %%";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%%";
        options.f_license = "MIT-%%";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %%";
        options.f_build_date = "Jun  4 2019 %%";
        options.f_build_time = "23:02:36 %%";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: %\n"
"   --this-is-a-very-long-argument-so-we-can-see-that-such-a-crazy-long-option-(w\n"
"   ho-does-that-though)-gets-broken-up-as-expected\n"
"                              I guess the help is not necessary now...\n"
"   --verbose                  inform you of what we're currently working on: %.\n"
"\n"
"Percent Percent: %\n"
                );

        CATCH_REQUIRE(opt.process_help_string(nullptr) == std::string());
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_project_name", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Project Name (name defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %a.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %a";
        options.f_help_footer = "Percent Project Name: %a";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%a";
        options.f_license = "MIT-%a";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %a";
        options.f_build_date = "Jun  4 2019 %a";
        options.f_build_time = "23:02:36 %a";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: unittest\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              unittest.\n"
"\n"
"Percent Project Name: unittest\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Project Name (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %a.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = nullptr;
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %a";
        options.f_help_footer = "Percent Project Name: %a";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%a";
        options.f_license = "MIT-%a";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %a";
        options.f_build_date = "Jun  4 2019 %a";
        options.f_build_time = "23:02:36 %a";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Project Name: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Project Name (\"\")")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %a.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %a";
        options.f_help_footer = "Percent Project Name: %a";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%a";
        options.f_license = "MIT-%a";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %a";
        options.f_build_date = "Jun  4 2019 %a";
        options.f_build_time = "23:02:36 %a";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Project Name: \n"
                );
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("help_string_build_date", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Build Date (defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %b.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %b";
        options.f_help_footer = "Percent Build Date: %b";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%b";
        options.f_license = "MIT-%b";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %b";
        options.f_build_date = "Jun  4 2019 %b";
        options.f_build_time = "23:02:36 %b";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: Jun  4 2019 %b\n"
"   --verbose                  inform you of what we're currently working on: Jun\n"
"                              4 2019 %b.\n"
"\n"
"Percent Build Date: Jun  4 2019 %b\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Date (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %b.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %b";
        options.f_help_footer = "Percent Build Date: %b";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%b";
        options.f_license = "MIT-%b";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %b";
        options.f_build_date = nullptr;
        options.f_build_time = "23:02:36 %b";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Date: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Date (\"\")")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %b.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %b";
        options.f_help_footer = "Percent Build Date: %b";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%b";
        options.f_license = "MIT-%b";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %b";
        options.f_build_date = "";
        options.f_build_time = "23:02:36 %b";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Date: \n"
                );
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_copyright", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Copyright (defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %c.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %c";
        options.f_help_footer = "Percent Copyright: %c";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%c";
        options.f_license = "MIT-%c";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %c";
        options.f_build_date = "Jun  4 2019 %c";
        options.f_build_time = "23:02:36 %c";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

//std::string a = opt.usage();
//std::string b = "Usage: test usage: Copyright (c) 2019  Made to Order Software Corp. -- All\n"
//"Rights Reserved %c\n"
//"   --verbose                  inform you of what we're currently working on:\n"
//"                              Copyright (c) 2019  Made to Order Software Corp.\n"
//"                              -- All Rights Reserved %c.\n"
//"\n"
//"Percent Copyright: Copyright (c) 2019  Made to Order Software Corp. -- All\n"
//"Rights Reserved %c\n";
//int max(std::min(a.length(), b.length()));
//for(int idx(0); idx < max;++idx)
//{
//    std::cerr << "[" << a[idx] << "] == [" << b[idx] << "] = " << (a[idx] == b[idx] ? "TRUE" : "FALSE") << "\n";
//}

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: Copyright (c) 2019  Made to Order Software Corp. -- All\n"
"Rights Reserved %c\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              Copyright (c) 2019  Made to Order Software Corp.\n"
"                              -- All Rights Reserved %c.\n"
"\n"
"Percent Copyright: Copyright (c) 2019  Made to Order Software Corp. -- All\n"
"Rights Reserved %c\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Copyright (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %c.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %c";
        options.f_help_footer = "Percent Copyright: %c";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%c";
        options.f_license = "MIT-%c";
        options.f_copyright = nullptr;
        options.f_build_date = "Jun  4 2019 %c";
        options.f_build_time = "23:02:36 %c";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Copyright: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Copyright (\"\")")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %c.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %c";
        options.f_help_footer = "Percent Copyright: %c";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%c";
        options.f_license = "MIT-%c";
        options.f_copyright = "";
        options.f_build_date = "Jun  4 2019 %c";
        options.f_build_time = "23:02:36 %c";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Copyright: \n"
                );
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_directories", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Directories (fully defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %d.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_directories[] =
        {
            "/etc/sys/advgetopt",
            "/etc/advgetopt",
            "/etc/advgetopt/advgetopt.d",
            "~/.config/advgetopt",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = configuration_directories;
        options.f_help_header = "Usage: test usage: %d";
        options.f_help_footer = "Percent Directories: %d";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%d";
        options.f_license = "MIT-%d";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %d";
        options.f_build_date = "Jun  4 2019 %d";
        options.f_build_time = "23:02:36 %d";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: /etc/sys/advgetopt\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              /etc/sys/advgetopt.\n"
"\n"
"Percent Directories: /etc/sys/advgetopt\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Directories (fully defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*d.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_directories[] =
        {
            "/etc/sys/advgetopt",
            "/etc/advgetopt",
            "/etc/advgetopt/advgetopt.d",
            "~/.config/advgetopt",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = configuration_directories;
        options.f_help_header = "Usage: test usage: %*d";
        options.f_help_footer = "Percent Directories: %*d";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*d";
        options.f_license = "MIT-%*d";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*d";
        options.f_build_date = "Jun  4 2019 %*d";
        options.f_build_time = "23:02:36 %*d";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: /etc/sys/advgetopt, /etc/advgetopt,\n"
"/etc/advgetopt/advgetopt.d, ~/.config/advgetopt\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              /etc/sys/advgetopt, /etc/advgetopt,\n"
"                              /etc/advgetopt/advgetopt.d, ~/.config/advgetopt.\n"
"\n"
"Percent Directories: /etc/sys/advgetopt, /etc/advgetopt,\n"
"/etc/advgetopt/advgetopt.d, ~/.config/advgetopt\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Directories (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %d.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %d";
        options.f_help_footer = "Percent Directories: %d";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%d";
        options.f_license = "MIT-%d";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %d";
        options.f_build_date = "Jun  4 2019 %d";
        options.f_build_time = "23:02:36 %d";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Directories (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*d.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*d";
        options.f_help_footer = "Percent Directories: %*d";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*d";
        options.f_license = "MIT-%*d";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*d";
        options.f_build_date = "Jun  4 2019 %*d";
        options.f_build_time = "23:02:36 %*d";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Directories (empty array)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %d.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_directories[] =
        {
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = configuration_directories;
        options.f_help_header = "Usage: test usage: %d";
        options.f_help_footer = "Percent Directories: %d";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%d";
        options.f_license = "MIT-%d";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %d";
        options.f_build_date = "Jun  4 2019 %d";
        options.f_build_time = "23:02:36 %d";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Directories (empty array)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*d.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_directories[] =
        {
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = configuration_directories;
        options.f_help_header = "Usage: test usage: %*d";
        options.f_help_footer = "Percent Directories: %*d";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*d";
        options.f_license = "MIT-%*d";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*d";
        options.f_build_date = "Jun  4 2019 %*d";
        options.f_build_time = "23:02:36 %*d";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_environment_variable", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Environment Variable (fully defined, variable set)")
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %e";
        options.f_help_footer = "Percent Environment Variable: %e";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%e";
        options.f_license = "MIT-%e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %e";
        options.f_build_date = "Jun  4 2019 %e";
        options.f_build_time = "23:02:36 %e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: ADVGETOPT_TEST_OPTIONS\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              ADVGETOPT_TEST_OPTIONS.\n"
"\n"
"Percent Environment Variable: ADVGETOPT_TEST_OPTIONS\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (fully defined, variable not set)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %e";
        options.f_help_footer = "Percent Environment Variable: %e";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%e";
        options.f_license = "MIT-%e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %e";
        options.f_build_date = "Jun  4 2019 %e";
        options.f_build_time = "23:02:36 %e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: ADVGETOPT_TEST_OPTIONS\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              ADVGETOPT_TEST_OPTIONS.\n"
"\n"
"Percent Environment Variable: ADVGETOPT_TEST_OPTIONS\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (fully defined, variable set)")
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %*e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*e";
        options.f_help_footer = "Percent Environment Variable: %*e";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*e";
        options.f_license = "MIT-%*e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*e";
        options.f_build_date = "Jun  4 2019 %*e";
        options.f_build_time = "23:02:36 %*e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: ADVGETOPT_TEST_OPTIONS=--verbose\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              ADVGETOPT_TEST_OPTIONS=--verbose.\n"
"\n"
"Percent Environment Variable: ADVGETOPT_TEST_OPTIONS=--verbose\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (fully defined, variable not set)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %*e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*e";
        options.f_help_footer = "Percent Environment Variable: %*e";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*e";
        options.f_license = "MIT-%*e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*e";
        options.f_build_date = "Jun  4 2019 %*e";
        options.f_build_time = "23:02:36 %*e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: ADVGETOPT_TEST_OPTIONS (not set)\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              ADVGETOPT_TEST_OPTIONS (not set).\n"
"\n"
"Percent Environment Variable: ADVGETOPT_TEST_OPTIONS (not set)\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (nullptr, variable set)")
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %e";
        options.f_help_footer = "Percent Environment Variable: %e";
        options.f_environment_variable_name = nullptr;
        options.f_version = "2.0.1-%e";
        options.f_license = "MIT-%e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %e";
        options.f_build_date = "Jun  4 2019 %e";
        options.f_build_time = "23:02:36 %e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (nullptr, variable not set)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %e";
        options.f_help_footer = "Percent Environment Variable: %e";
        options.f_environment_variable_name = nullptr;
        options.f_version = "2.0.1-%e";
        options.f_license = "MIT-%e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %e";
        options.f_build_date = "Jun  4 2019 %e";
        options.f_build_time = "23:02:36 %e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (nullptr, variable set)")
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %*e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*e";
        options.f_help_footer = "Percent Environment Variable: %*e";
        options.f_environment_variable_name = nullptr;
        options.f_version = "2.0.1-%*e";
        options.f_license = "MIT-%*e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*e";
        options.f_build_date = "Jun  4 2019 %*e";
        options.f_build_time = "23:02:36 %*e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (nullptr, variable not set)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %*e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*e";
        options.f_help_footer = "Percent Environment Variable: %*e";
        options.f_environment_variable_name = nullptr;
        options.f_version = "2.0.1-%*e";
        options.f_license = "MIT-%*e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*e";
        options.f_build_date = "Jun  4 2019 %*e";
        options.f_build_time = "23:02:36 %*e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (empty string, variable set)")
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);


        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %e";
        options.f_help_footer = "Percent Environment Variable: %e";
        options.f_environment_variable_name = "";
        options.f_version = "2.0.1-%e";
        options.f_license = "MIT-%e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %e";
        options.f_build_date = "Jun  4 2019 %e";
        options.f_build_time = "23:02:36 %e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (empty string, variable not set)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);


        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %e";
        options.f_help_footer = "Percent Environment Variable: %e";
        options.f_environment_variable_name = "";
        options.f_version = "2.0.1-%e";
        options.f_license = "MIT-%e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %e";
        options.f_build_date = "Jun  4 2019 %e";
        options.f_build_time = "23:02:36 %e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (empty string, variable set)")
        snap::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %*e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*e";
        options.f_help_footer = "Percent Environment Variable: %*e";
        options.f_environment_variable_name = "";
        options.f_version = "2.0.1-%*e";
        options.f_license = "MIT-%*e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*e";
        options.f_build_date = "Jun  4 2019 %*e";
        options.f_build_time = "23:02:36 %*e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (empty string, variable not set)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::var_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %*e.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*e";
        options.f_help_footer = "Percent Environment Variable: %*e";
        options.f_environment_variable_name = "";
        options.f_version = "2.0.1-%*e";
        options.f_license = "MIT-%*e";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*e";
        options.f_build_date = "Jun  4 2019 %*e";
        options.f_build_time = "23:02:36 %*e";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("help_string_configuration_files", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Configuration Files (fully defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %f.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_files[] =
        {
            "system.conf",
            "advgetopt.conf",
            "advgetopt.ini",
            "user.config",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %f";
        options.f_help_footer = "Percent Configuration Files: %f";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%f";
        options.f_license = "MIT-%f";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %f";
        options.f_build_date = "Jun  4 2019 %f";
        options.f_build_time = "23:02:36 %f";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: system.conf\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              system.conf.\n"
"\n"
"Percent Configuration Files: system.conf\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files (fully defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*f.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_files[] =
        {
            "system.conf",
            "advgetopt.conf",
            "advgetopt.ini",
            "user.config",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*f";
        options.f_help_footer = "Percent Configuration Files: %*f";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*f";
        options.f_license = "MIT-%*f";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*f";
        options.f_build_date = "Jun  4 2019 %*f";
        options.f_build_time = "23:02:36 %*f";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: system.conf, advgetopt.conf, advgetopt.ini, user.config\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              system.conf, advgetopt.conf, advgetopt.ini,\n"
"                              user.config.\n"
"\n"
"Percent Configuration Files: system.conf, advgetopt.conf, advgetopt.ini,\n"
"user.config\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %f.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %f";
        options.f_help_footer = "Percent Configuration Files: %f";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%f";
        options.f_license = "MIT-%f";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %f";
        options.f_build_date = "Jun  4 2019 %f";
        options.f_build_time = "23:02:36 %f";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*f.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*f";
        options.f_help_footer = "Percent Configuration Files: %*f";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*f";
        options.f_license = "MIT-%*f";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*f";
        options.f_build_date = "Jun  4 2019 %*f";
        options.f_build_time = "23:02:36 %*f";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files (empty array)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %f.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_files[] =
        {
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %f";
        options.f_help_footer = "Percent Configuration Files: %f";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%f";
        options.f_license = "MIT-%f";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %f";
        options.f_build_date = "Jun  4 2019 %f";
        options.f_build_time = "23:02:36 %f";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files (empty array)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*f.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        const char * const configuration_files[] =
        {
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*f";
        options.f_help_footer = "Percent Configuration Files: %*f";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*f";
        options.f_license = "MIT-%*f";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*f";
        options.f_build_date = "Jun  4 2019 %*f";
        options.f_build_time = "23:02:36 %*f";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_license", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent License (defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %l.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %l";
        options.f_help_footer = "Percent License: %l";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%l";
        options.f_license = "MIT-%l";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %l";
        options.f_build_date = "Jun  4 2019 %l";
        options.f_build_time = "23:02:36 %l";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: MIT-%l\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              MIT-%l.\n"
"\n"
"Percent License: MIT-%l\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent License (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %l.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %l";
        options.f_help_footer = "Percent License: %l";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%l";
        options.f_license = nullptr;
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %l";
        options.f_build_date = "Jun  4 2019 %l";
        options.f_build_time = "23:02:36 %l";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent License: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent License (\"\")")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %l.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %l";
        options.f_help_footer = "Percent License: %l";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%l";
        options.f_license = "";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %l";
        options.f_build_date = "Jun  4 2019 %l";
        options.f_build_time = "23:02:36 %l";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent License: \n"
                );
    CATCH_END_SECTION()
}






CATCH_TEST_CASE("help_string_program_name", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Program Name")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %p.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %p";
        options.f_help_footer = "Percent Program Name: %p";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%p";
        options.f_license = "MIT-%p";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %p";
        options.f_build_date = "Jun  4 2019 %p";
        options.f_build_time = "23:02:36 %p";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: usage\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              usage.\n"
"\n"
"Percent Program Name: usage\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Program Name")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %*p.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %*p";
        options.f_help_footer = "Percent Program Name: %*p";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*p";
        options.f_license = "MIT-%*p";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*p";
        options.f_build_date = "Jun  4 2019 %*p";
        options.f_build_time = "23:02:36 %*p";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: tests/unittests/usage\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              tests/unittests/usage.\n"
"\n"
"Percent Program Name: tests/unittests/usage\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Program Name (empty--before parsing the arguments)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("inform you of what we're currently working on: %p.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %p";
        options.f_help_footer = "Percent Program Name: %p";
        options.f_environment_variable_name = nullptr;
        options.f_version = "2.0.1-%p";
        options.f_license = "MIT-%p";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %p";
        options.f_build_date = "Jun  4 2019 %p";
        options.f_build_time = "23:02:36 %p";

        advgetopt::getopt opt(options);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Program Name: \n"
                );
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("help_string_build_time", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Build Time (defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %t.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %t";
        options.f_help_footer = "Percent Build Time: %t";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%t";
        options.f_license = "MIT-%t";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %t";
        options.f_build_date = "Jun  4 2019 %t";
        options.f_build_time = "23:02:36 %t";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: 23:02:36 %t\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              23:02:36 %t.\n"
"\n"
"Percent Build Time: 23:02:36 %t\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Time (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %t.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %t";
        options.f_help_footer = "Percent Build Time: %t";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%t";
        options.f_license = "MIT-%t";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %t";
        options.f_build_date = "Jun  4 2019 %t";
        options.f_build_time = nullptr;

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Time: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Time (\"\")")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %t.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %t";
        options.f_help_footer = "Percent Build Time: %t";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%t";
        options.f_license = "MIT-%t";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %t";
        options.f_build_date = "Jun  4 2019 %t";
        options.f_build_time = "";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Time: \n"
                );
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_version", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Version (defined)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %v.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %v";
        options.f_help_footer = "Percent Version: %v";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%v";
        options.f_license = "MIT-%v";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %v";
        options.f_build_date = "Jun  4 2019 %v";
        options.f_build_time = "23:02:36 %v";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: 2.0.1-%v\n"
"   --verbose                  inform you of what we're currently working on:\n"
"                              2.0.1-%v.\n"
"\n"
"Percent Version: 2.0.1-%v\n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Version (nullptr)")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %v.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %v";
        options.f_help_footer = "Percent Version: %v";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = nullptr;
        options.f_license = "MIT-%v";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %v";
        options.f_build_date = "Jun  4 2019 %v";
        options.f_build_time = "23:02:36 %v";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Version: \n"
                );
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Version (\"\")")
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %v.")
            ),
            advgetopt::end_options()
        };
        char const * cargv[] =
        {
            "tests/unittests/usage",
            "--verbose",
            nullptr
        };
        int const argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = nullptr;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %v";
        options.f_help_footer = "Percent Version: %v";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "";
        options.f_license = "MIT-%v";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %v";
        options.f_build_date = "Jun  4 2019 %v";
        options.f_build_time = "23:02:36 %v";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE(opt.usage() ==
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Version: \n"
                );
    CATCH_END_SECTION()
}







// vim: ts=4 sw=4 et