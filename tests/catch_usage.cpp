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
#include    <advgetopt/flags.h>
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







CATCH_TEST_CASE("usage_function", "[getopt][usage]")
{
    std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir());
    tmpdir += "/.config/home";
    snapdev::safe_setenv env("HOME", tmpdir);

    CATCH_START_SECTION("usage() using \"--filename\" for the default option accepting multiple entries")
    {
        advgetopt::option const options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("validate")
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("this is used to validate different things.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("long")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                           , advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("used to validate that invalid numbers generate an error.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("out-of-bounds")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                      , advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED>())
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
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR
                                                                     , advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
                //, advgetopt::Help("make sure everything is unique.") -- do not show in --help
            ),
            advgetopt::define_option(
                  advgetopt::Name("quiet")
                , advgetopt::ShortName('q')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR
                                                      , advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filename")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                           , advgetopt::GETOPT_FLAG_MULTIPLE
                                                           , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
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

        advgetopt::group_description const groups[] =
        {
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
                , advgetopt::GroupName("verbosity")
                , advgetopt::GroupDescription("Verbosity options:")
            ),
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
                , advgetopt::GroupName("option")
                , advgetopt::GroupDescription("Special options:")
            ),
            advgetopt::end_groups()
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_group_name = "grouptest";
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
                                "   . All Existing Configuration Files = [%g]\n"
                                "   . All Possible Configuration Files = [%*g]\n"
                                "   . Option File Directory = [%i]\n"
                                "   . License = [%l]\n"
                                "   . Output File [%o]\n"
                                "   . Program Name = [%p]\n"
                                "   . Program Fullname = [%*p]\n"
                                "   . Group Name = [%s]\n"
                                "   . Build Time = [%t]\n"
                                "   . Version = [%v]\n"
                                "   . Existing Writable Configuration Files = [%w]\n"
                ;
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1";
        options.f_license = "MIT";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved";
        options.f_build_date = "Jun  4 2019";
        options.f_build_time = "23:02:36";
        options.f_groups = groups;

        // this initialization works as expected
        //
        advgetopt::getopt opt(options, argc2, argv2);

        CATCH_REQUIRE(opt.get_group_name() == "grouptest");

        {
            advgetopt::group_description const * verbosity_group(opt.find_group(advgetopt::GETOPT_FLAG_GROUP_COMMANDS));
            CATCH_REQUIRE(verbosity_group != nullptr);
            CATCH_REQUIRE(verbosity_group->f_group == advgetopt::GETOPT_FLAG_GROUP_COMMANDS);
            CATCH_REQUIRE(verbosity_group->f_name == std::string("verbosity"));
            CATCH_REQUIRE(verbosity_group->f_description == std::string("Verbosity options:"));
        }

        {
            advgetopt::group_description const * option_group(opt.find_group(advgetopt::GETOPT_FLAG_GROUP_OPTIONS));
            CATCH_REQUIRE(option_group != nullptr);
            CATCH_REQUIRE(option_group->f_group == advgetopt::GETOPT_FLAG_GROUP_OPTIONS);
            CATCH_REQUIRE(option_group->f_name == std::string("option"));
            CATCH_REQUIRE(option_group->f_description == std::string("Special options:"));
        }

        {
            advgetopt::group_description const * group_seven(opt.find_group(advgetopt::GETOPT_FLAG_GROUP_SEVEN));
            CATCH_REQUIRE(group_seven == nullptr);
        }

        std::string const footer(advgetopt::getopt::breakup_line(
"\n"
"And this is the footer where we can include many parameters:\n"
"   . Percent = [%]\n"
"   . Project Name = [unittest]\n"
"   . Build Date = [Jun  4 2019]\n"
"   . Copyright = [Copyright (c) 2019  Made to Order Software Corp. -- All Rights "
"Reserved]\n"
"   . Directories = [/etc/sys/advgetopt]\n"
"   . All Directories = [/etc/sys/advgetopt, /etc/advgetopt, "
"/etc/advgetopt/advgetopt.d, ~/.config/advgetopt]\n"
"   . Environment Variable = [ADVGETOPT_TEST_OPTIONS]\n"
"   . Environment Variable and Value = [ADVGETOPT_TEST_OPTIONS (not set)]\n"
"   . Configuration Files = [advgetopt.conf]\n"
"   . All Configuration Files = [advgetopt.conf, advgetopt.ini, advgetopt.xml, "
"advgetopt.yaml]\n"
"   . All Existing Configuration Files = []\n"
"   . All Possible Configuration Files = [advgetopt.conf, "
"grouptest.d/50-advgetopt.conf, advgetopt.ini, grouptest.d/50-advgetopt.ini, "
"advgetopt.xml, grouptest.d/50-advgetopt.xml, advgetopt.yaml, "
"grouptest.d/50-advgetopt.yaml, /etc/sys/advgetopt/advgetopt.conf, "
"/etc/sys/advgetopt/grouptest.d/50-advgetopt.conf, /etc/advgetopt/advgetopt.conf, "
"/etc/advgetopt/grouptest.d/50-advgetopt.conf, "
"/etc/advgetopt/advgetopt.d/advgetopt.conf, "
"/etc/advgetopt/advgetopt.d/grouptest.d/50-advgetopt.conf, "
+ tmpdir + "/.config/advgetopt/advgetopt.conf]\n"
"   . Option File Directory = [/etc/advgetopt/]\n"
"   . License = [MIT]\n"
"   . Output File [" + tmpdir + "/.config/advgetopt/advgetopt.conf]\n"
"   . Program Name = [unittest_advgetopt]\n"
"   . Program Fullname = [tests/unittests/unittest_advgetopt]\n"
"   . Group Name = [grouptest]\n"
"   . Build Time = [23:02:36]\n"
"   . Version = [2.0.1]\n"
"   . Existing Writable Configuration Files = []\n"
"", 0, advgetopt::getopt::get_line_width()));

        // test a standard "--help" type of option
        //
        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long <arg>"
            , "used to validate that invalid numbers generate an error."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--string <arg>"
            , "string parameter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--unique or -u"
            , "make sure everything is unique."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--validate"
            , "this is used to validate different things."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Verbosity options:\n"
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--option-help"
            , "show help from the \"option\" group of options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--quiet or -q {<arg>}"
            , "make it quiet (opposite of verbose)."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbosity-help"
            , "show help from the \"verbosity\" group of options."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Special options:\n"
+ advgetopt::getopt::format_usage_string(
              "--out-of-bounds or -o <arg>"
            , "valid values from 1 to 9."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "[default arguments]"
            , "other parameters are viewed as filenames."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );

        // test a "--help-all" type of option
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long <arg>"
            , "used to validate that invalid numbers generate an error."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-with-invalid-default <arg> {<arg>}"
              " (default is \"123abc\")"
            , "test that an invalid default value can be returned as is."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--string <arg>"
            , "string parameter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--unique or -u"
            , "make sure everything is unique."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--validate"
            , "this is used to validate different things."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Verbosity options:\n"
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--option-help"
            , "show help from the \"option\" group of options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--quiet or -q {<arg>}"
            , "make it quiet (opposite of verbose)."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbosity-help"
            , "show help from the \"verbosity\" group of options."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Special options:\n"
+ advgetopt::getopt::format_usage_string(
              "--not-specified-and-no-default <arg>"
            , "test long without having used the option and no default."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--out-of-bounds or -o <arg>"
            , "valid values from 1 to 9."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "[default arguments]"
            , "other parameters are viewed as filenames."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );

        // pretend an error occurred
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--validate"
            , "this is used to validate different things."
            , 30
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Verbosity options:\n"
+ advgetopt::getopt::format_usage_string(
              "--quiet or -q {<arg>}"
            , "make it quiet (opposite of verbose)."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );

        // show GROUP1
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP1),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ "\n"
  "Special options:\n"
+ advgetopt::getopt::format_usage_string(
              "--not-specified-and-no-default <arg>"
            , "test long without having used the option and no default."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );

        // show GROUP2
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP2),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")"
            , "test that an invalid default value can be returned as is."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("usage() using \"--filename\" for the default option accepting multiple entries which are required when \"--filename\" is used")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("validate")
                , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("this is used to validate different things.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("long")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("used to validate that invalid numbers generate an error.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("out-of-bounds")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
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
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filename")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
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
        options.f_options_files_directory = "/usr/share/advgetopt";
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
                                "   . All Existing Configuration Files = [%g]\n"
                                "   . All Possible Configuration Files = [%*g]\n"
                                "   . Option File Directory = [%i]\n"
                                "   . License = [%l]\n"
                                "   . Output File [%o]\n"
                                "   . Program Name = [%p]\n"
                                "   . Program Fullname = [%*p]\n"
                                "   . Build Time = [%t]\n"
                                "   . Version = [%v]\n"
                                "   . Existing Writable Configuration Files = [%w]\n"
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

        CATCH_REQUIRE(opt.find_group(advgetopt::GETOPT_FLAG_GROUP_COMMANDS) == nullptr);
        CATCH_REQUIRE(opt.find_group(advgetopt::GETOPT_FLAG_GROUP_OPTIONS) == nullptr);
  
        std::string const footer(advgetopt::getopt::breakup_line(
"\n"
"And this is the footer where we can include many parameters:\n"
"   . Percent = [%]\n"
"   . Project Name = [unittest]\n"
"   . Build Date = [Jun  4 2019]\n"
"   . Copyright = [Copyright (c) 2019  Made to Order Software Corp. -- All Rights "
"Reserved]\n"
"   . Directories = [/etc/sys/advgetopt]\n"
"   . All Directories = [/etc/sys/advgetopt, /etc/advgetopt, "
"/etc/advgetopt/advgetopt.d, ~/.config/advgetopt]\n"
"   . Environment Variable = [ADVGETOPT_TEST_OPTIONS]\n"
"   . Environment Variable and Value = [ADVGETOPT_TEST_OPTIONS (not set)]\n"
"   . Configuration Files = [advgetopt.conf]\n"
"   . All Configuration Files = [advgetopt.conf, advgetopt.ini, advgetopt.xml, "
"advgetopt.yaml]\n"
"   . All Existing Configuration Files = []\n"
"   . All Possible Configuration Files = [advgetopt.conf, "
"unittest.d/50-advgetopt.conf, advgetopt.ini, unittest.d/50-advgetopt.ini, "
"advgetopt.xml, unittest.d/50-advgetopt.xml, advgetopt.yaml, "
"unittest.d/50-advgetopt.yaml, /etc/sys/advgetopt/advgetopt.conf, "
"/etc/sys/advgetopt/unittest.d/50-advgetopt.conf, /etc/advgetopt/advgetopt.conf, "
"/etc/advgetopt/unittest.d/50-advgetopt.conf, "
"/etc/advgetopt/advgetopt.d/advgetopt.conf, "
"/etc/advgetopt/advgetopt.d/unittest.d/50-advgetopt.conf, "
+ tmpdir + "/.config/advgetopt/advgetopt.conf]\n"
"   . Option File Directory = [/usr/share/advgetopt/]\n"
"   . License = [MIT]\n"
"   . Output File [" + tmpdir + "/.config/advgetopt/advgetopt.conf]\n"
"   . Program Name = [unittest_advgetopt]\n"
"   . Program Fullname = [tests/unittests/unittest_advgetopt]\n"
"   . Build Time = [23:02:36]\n"
"   . Version = [2.0.1]\n"
"   . Existing Writable Configuration Files = []\n"
, 0, advgetopt::getopt::get_line_width()));

        // test a standard "--help" type of option
        //
        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long <arg>"
            , "used to validate that invalid numbers generate an error."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--out-of-bounds or -o <arg>"
            , "valid values from 1 to 9."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--quiet or -q {<arg>}"
            , "make it quiet (opposite of verbose)."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--string <arg>"
            , "string parameter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--unique or -u"
            , "make sure everything is unique."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--validate"
            , "this is used to validate different things."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "<default arguments>"
            , "other parameters are viewed as filenames."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );

        // test a "--help-all" type of option
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long <arg>"
            , "used to validate that invalid numbers generate an error."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-and-no-default <arg>"
            , "test long without having used the option and no default."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")"
            , "test that an invalid default value can be returned as is."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--out-of-bounds or -o <arg>"
            , "valid values from 1 to 9."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--quiet or -q {<arg>}"
            , "make it quiet (opposite of verbose)."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--string <arg>"
            , "string parameter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--unique or -u"
            , "make sure everything is unique."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--validate"
            , "this is used to validate different things."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "<default arguments>"
            , "other parameters are viewed as filenames."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );

        // pretend an error occurred
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR),
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --quiet or -q {<arg>}      make it quiet (opposite of verbose).\n"
"   --validate                 this is used to validate different things.\n"
+ footer
                        );

        // show GROUP1
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP1),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-and-no-default <arg>"
            , "test long without having used the option and no default."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );

        // show GROUP2
        //
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_GROUP2),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")"
            , "test that an invalid default value can be returned as is."
            , 30
            , advgetopt::getopt::get_line_width())
+ footer
                        );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("usage() using \"--filename\" for the default option accepting one required item")
    {
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
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED>())
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
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_MULTIPLE, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-in-v2-though")
                , advgetopt::ShortName('l')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE, advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
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
//                CATCH_REQUIRE_THROWS_AS(opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exit);
//            }
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL),
advgetopt::getopt::breakup_line(
              "Usage: try this one and we get a throw (valid options + usage calls bis)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long <arg>"
            , "used to validate that invalid numbers generate an error."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-in-v2-though or -l <arg>"
            , "long with just a letter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-and-no-default <arg>"
            , "test long without having used the option and no default."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")"
            , "test that an invalid default value can be returned as is."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--out-of-bounds or -o <arg>"
            , "valid values from 1 to 9."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--quiet or -q {<arg>}"
            , "make it quiet (opposite of verbose)."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--string <arg>"
            , "string parameter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--unique or -u"
            , "make sure everything is unique."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--validate"
            , "this is used to validate different things."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "<default argument>"
            , "other parameters are viewed as filenames; and we"
              " need at least one option with a very long help to"
              " check that it wraps perfectly (we'd really need to"
              " get the output of the command and check that"
              " against what is expected because at this time the"
              " test is rather blind in that respect! FIXED IN"
              " v2!)"
            , 30
            , advgetopt::getopt::get_line_width())
                    );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("usage() using \"--\" for the default option accepting one item")
    {
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
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("valid values from 1 to 9.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-and-no-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("test long without having used the option and no default.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-with-invalid-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("test that an invalid default value can be returned as is.")
                , advgetopt::DefaultValue("123abc")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-specified-string-without-default")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Alias("string")
            ),
            advgetopt::define_option(
                  advgetopt::Name("string")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED>())
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
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("make it quiet (opposite of verbose).")
            ),
            advgetopt::define_option(
                  advgetopt::Name("option-argument")
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
                , advgetopt::Help("command line option which accepts an optional argument.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("not-in-v2-though")
                , advgetopt::ShortName('l')
                , advgetopt::Flags(advgetopt::any_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                      , advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
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
        options.f_help_header = "Usage: %p try this one and we get a throw (valid options + usage calls bis)";
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
//                CATCH_REQUIRE_THROWS_AS(opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exit);
//            }
        CATCH_REQUIRE_LONG_STRING(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL),
advgetopt::getopt::breakup_line(
              "Usage: unittest_advgetopt try this one and we get a throw"
              " (valid options + usage calls bis)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long <arg>"
            , "used to validate that invalid numbers generate an error."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--long-help or -?"
            , "show all the help from all the available options."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-in-v2-though or -l <arg>"
            , "long with just a letter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-and-no-default <arg>"
            , "test long without having used the option and no default."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")"
            , "test that an invalid default value can be returned as is."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--option-argument [<arg>]"
            , "command line option which accepts an optional argument."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--out-of-bounds or -o <arg>"
            , "valid values from 1 to 9."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--quiet or -q {<arg>}"
            , "make it quiet (opposite of verbose)."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--string <arg>"
            , "string parameter."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--unique or -u"
            , "make sure everything is unique."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--validate"
            , "this is used to validate different things."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "[default argument]"
            , "other parameters are viewed as filenames; and we need at least"
              " one option with a very long help to check that it wraps"
              " perfectly (we'd really need to get the output of the command"
              " and check that against what is expected because at this time"
              " the test is rather blind in that respect! FIXED IN v2!)"
            , 30
            , advgetopt::getopt::get_line_width())
                    );
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("help_string_percent", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Percent")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: %"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--this-is-a-very-long-argument-so-we-can-see-that-such-a-crazy-long-option-(who-does-that-though)-gets-broken-up-as-expected"
            , "I guess the help is not necessary now..."
            , 30
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on: %."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Percent: %"
            , 0
            , advgetopt::getopt::get_line_width())
                );

        CATCH_REQUIRE(opt.process_help_string(nullptr) == std::string());
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_project_name", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Project Name (name defined)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: unittest"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on: unittest."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Project Name: unittest"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Project Name (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Project Name: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Project Name (\"\")")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Project Name: \n"
                );
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("help_string_build_date", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Build Date (defined)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: Jun  4 2019 %b"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on: Jun  4 2019 %b."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Build Date: Jun  4 2019 %b"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Date (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Date: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Date (\"\")")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Date: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_copyright", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Copyright (defined)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: Copyright (c) 2019  Made to Order Software Corp. -- All"
              " Rights Reserved %c"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " Copyright (c) 2019  Made to Order Software Corp."
              " -- All Rights Reserved %c."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Copyright: Copyright (c) 2019  Made to Order Software"
              " Corp. -- All Rights Reserved %c"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Copyright (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Copyright: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Copyright (\"\")")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Copyright: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_directories", "[getopt][usage][config]")
{
    CATCH_START_SECTION("Percent Directories (fully defined)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: /etc/sys/advgetopt"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " /etc/sys/advgetopt."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Directories: /etc/sys/advgetopt"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Directories (fully defined)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: /etc/sys/advgetopt, /etc/advgetopt,"
              " /etc/advgetopt/advgetopt.d, ~/.config/advgetopt"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " /etc/sys/advgetopt, /etc/advgetopt,"
              " /etc/advgetopt/advgetopt.d, ~/.config/advgetopt."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Directories: /etc/sys/advgetopt, /etc/advgetopt,"
              " /etc/advgetopt/advgetopt.d, ~/.config/advgetopt"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Directories (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Directories (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Directories (empty array)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Directories (empty array)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Directories: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_environment_variable", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Environment Variable (fully defined, variable set)")
    {
        snapdev::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: ADVGETOPT_TEST_OPTIONS"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " ADVGETOPT_TEST_OPTIONS."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Environment Variable: ADVGETOPT_TEST_OPTIONS"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (fully defined, variable not set)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: ADVGETOPT_TEST_OPTIONS"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " ADVGETOPT_TEST_OPTIONS."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Environment Variable: ADVGETOPT_TEST_OPTIONS"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (fully defined, variable set)")
    {
        snapdev::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: ADVGETOPT_TEST_OPTIONS=--verbose"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " ADVGETOPT_TEST_OPTIONS=--verbose."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Environment Variable: ADVGETOPT_TEST_OPTIONS=--verbose"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (fully defined, variable not set)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: ADVGETOPT_TEST_OPTIONS (not set)"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " ADVGETOPT_TEST_OPTIONS (not set)."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Environment Variable: ADVGETOPT_TEST_OPTIONS (not set)"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (nullptr, variable set)")
    {
        snapdev::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (nullptr, variable not set)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (nullptr, variable set)")
    {
        snapdev::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (nullptr, variable not set)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (empty string, variable set)")
    {
        snapdev::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Environment Variable (empty string, variable not set)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (empty string, variable set)")
    {
        snapdev::safe_setenv env("ADVGETOPT_TEST_OPTIONS", "--verbose");

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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Environment Variable (empty string, variable not set)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Environment Variable: \n"
                );
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("help_string_configuration_files", "[getopt][usage][config]")
{
    CATCH_START_SECTION("Percent Configuration Files with f (fully defined)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: system.conf"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on: system.conf."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Configuration Files: system.conf"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files with f (fully defined)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: system.conf, advgetopt.conf,"
              " advgetopt.ini, user.config"
            , 0
            , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on:"
              " system.conf, advgetopt.conf, advgetopt.ini,"
              " user.config."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Configuration Files: system.conf, advgetopt.conf,"
              " advgetopt.ini, user.config"
            , 0
            , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files with f (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files with f (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files with f (empty array)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files with f (empty array)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_configuration_files_functions", "[getopt][usage][config]")
{
    CATCH_START_SECTION("Percent Configuration Files with g (fully defined)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("config_filenames", "existing_g");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %g.")
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
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "advgetopt.conf",
            "advgetopt.ini",
            "user.config",
            SNAP_CATCH2_NAMESPACE::g_config_project_filename.c_str(),   // <- this is wrong, it should be done automatically, but we have a name mismatch...
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";        // name mismatch on purpose
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %g";
        options.f_help_footer = "Percent Configuration Files: %g";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%g";
        options.f_license = "MIT-%g";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %g";
        options.f_build_date = "Jun  4 2019 %g";
        options.f_build_time = "23:02:36 %g";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: "
            + SNAP_CATCH2_NAMESPACE::g_config_filename
            + ", "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose", "inform you of what we're currently working on: "
            + SNAP_CATCH2_NAMESPACE::g_config_filename + ", "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename + "."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Configuration Files: "
            + SNAP_CATCH2_NAMESPACE::g_config_filename
            + ", "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files with g (fully defined)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("config_file_names", "with_existing_g");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %g.")
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
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "advgetopt.conf",
            "advgetopt.ini",
            "user.config",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "config_file_names";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %g";
        options.f_help_footer = "Percent Configuration Files: %g";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%g";
        options.f_license = "MIT-%g";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %g";
        options.f_build_date = "Jun  4 2019 %g";
        options.f_build_time = "23:02:36 %g";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: "
            + SNAP_CATCH2_NAMESPACE::g_config_filename
            + ", "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose", "inform you of what we're currently working on: "
            + SNAP_CATCH2_NAMESPACE::g_config_filename + ", "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename + "."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Configuration Files: "
            + SNAP_CATCH2_NAMESPACE::g_config_filename
            + ", "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files with g (fully defined)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*g.")
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
        options.f_help_header = "Usage: test usage: %*g";
        options.f_help_footer = "Percent Configuration Files: %*g";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*g";
        options.f_license = "MIT-%*g";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*g";
        options.f_build_date = "Jun  4 2019 %*g";
        options.f_build_time = "23:02:36 %*g";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
          "Usage: test usage: system.conf, unittest.d/50-system.conf,"
          " advgetopt.conf, unittest.d/50-advgetopt.conf, advgetopt.ini,"
          " unittest.d/50-advgetopt.ini, user.config,"
          " unittest.d/50-user.config"
        , 0
        , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
          "--verbose"
        , "inform you of what we're currently working on:"
          " system.conf, unittest.d/50-system.conf,"
          " advgetopt.conf, unittest.d/50-advgetopt.conf,"
          " advgetopt.ini, unittest.d/50-advgetopt.ini,"
          " user.config, unittest.d/50-user.config."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
          "Percent Configuration Files: system.conf, unittest.d/50-system.conf,"
          " advgetopt.conf, unittest.d/50-advgetopt.conf, advgetopt.ini,"
          " unittest.d/50-advgetopt.ini, user.config, unittest.d/50-user.config"
        , 0
        , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files with g (nullptr)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %g.")
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
        options.f_help_header = "Usage: test usage: %g";
        options.f_help_footer = "Percent Configuration Files: %g";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%g";
        options.f_license = "MIT-%g";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %g";
        options.f_build_date = "Jun  4 2019 %g";
        options.f_build_time = "23:02:36 %g";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files with g (nullptr)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*g.")
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
        options.f_help_header = "Usage: test usage: %*g";
        options.f_help_footer = "Percent Configuration Files: %*g";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*g";
        options.f_license = "MIT-%*g";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*g";
        options.f_build_date = "Jun  4 2019 %*g";
        options.f_build_time = "23:02:36 %*g";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files with g (empty array)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %g.")
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
        options.f_help_header = "Usage: test usage: %g";
        options.f_help_footer = "Percent Configuration Files: %g";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%g";
        options.f_license = "MIT-%g";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %g";
        options.f_build_date = "Jun  4 2019 %g";
        options.f_build_time = "23:02:36 %g";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Configuration Files with g (empty array)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %*g.")
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
        options.f_help_header = "Usage: test usage: %*g";
        options.f_help_footer = "Percent Configuration Files: %*g";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%*g";
        options.f_license = "MIT-%*g";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %*g";
        options.f_build_date = "Jun  4 2019 %*g";
        options.f_build_time = "23:02:36 %*g";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_option_file_directory", "[getopt][usage][config]")
{
    CATCH_START_SECTION("Percent Configuration Files with i (fully defined)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %i.")
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
            "advgetopt.conf",
            "advgetopt.ini",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "/opt/advgetopt/config";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %i";
        options.f_help_footer = "Percent Configuration Files: %i";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%i";
        options.f_license = "MIT-%i";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %i";
        options.f_build_date = "Jun  4 2019 %i";
        options.f_build_time = "23:02:36 %i";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
              "Usage: test usage: /opt/advgetopt/config/\n"
+ advgetopt::getopt::format_usage_string(
              "--verbose", "inform you of what we're currently working on: "
              "/opt/advgetopt/config/."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Configuration Files: /opt/advgetopt/config/"
        , 0
        , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files with i (nullptr)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %i.")
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
            "/ignored/in/this/test",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = nullptr;
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %i";
        options.f_help_footer = "Percent Configuration Files: %i";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%i";
        options.f_license = "MIT-%i";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %i";
        options.f_build_date = "Jun  4 2019 %i";
        options.f_build_time = "23:02:36 %i";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: /usr/share/advgetopt/options/", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on: /usr/share/advgetopt/options/."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line("Percent Configuration Files: /usr/share/advgetopt/options/", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Files with i (empty string)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %i.")
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
            "/ignored/in/this/test",
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_options_files_directory = "";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %i";
        options.f_help_footer = "Percent Configuration Files: %i";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%i";
        options.f_license = "MIT-%i";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %i";
        options.f_build_date = "Jun  4 2019 %i";
        options.f_build_time = "23:02:36 %i";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: /usr/share/advgetopt/options/", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
                  "--verbose"
                , "inform you of what we're currently working on: /usr/share/advgetopt/options/."
                , 30
                , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line("Percent Configuration Files: /usr/share/advgetopt/options/", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_license", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent License (defined)")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: MIT-%l", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on: MIT-%l."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line("Percent License: MIT-%l", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent License (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent License: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent License (\"\")")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent License: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_configuration_output_file", "[getopt][usage][config]")
{
    CATCH_START_SECTION("Percent Configuration Output File (fully defined)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %o.")
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
        options.f_help_header = "Usage: test usage: %o";
        options.f_help_footer = "Percent Configuration Files: %o";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%o";
        options.f_license = "MIT-%o";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %o";
        options.f_build_date = "Jun  4 2019 %o";
        options.f_build_time = "23:02:36 %o";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: unittest.d/50-user.config", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
          "--verbose"
        , "inform you of what we're currently working on: unittest.d/50-user.config."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line("Percent Configuration Files: unittest.d/50-user.config", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Output File (nullptr)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %o.")
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
        options.f_help_header = "Usage: test usage: %o";
        options.f_help_footer = "Percent Configuration Files: %o";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%o";
        options.f_license = "MIT-%o";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %o";
        options.f_build_date = "Jun  4 2019 %o";
        options.f_build_time = "23:02:36 %o";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Configuration Output File (empty array)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %o.")
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
        options.f_help_header = "Usage: test usage: %o";
        options.f_help_footer = "Percent Configuration Files: %o";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%o";
        options.f_license = "MIT-%o";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %o";
        options.f_build_date = "Jun  4 2019 %o";
        options.f_build_time = "23:02:36 %o";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()
}






CATCH_TEST_CASE("help_string_program_name", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Program Name")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: usage", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
          "--verbose"
        , "inform you of what we're currently working on: usage."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line("Percent Program Name: usage", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Asterisk Program Name")
    {
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

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: tests/unittests/usage", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose"
            , "inform you of what we're currently working on: tests/unittests/usage."
            , 30
            , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line("Percent Program Name: tests/unittests/usage", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Program Name (empty--before parsing the arguments)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Program Name: \n"
                );
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("help_string_build_time", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Build Time (defined)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: 23:02:36 %t", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
          "--verbose"
        , "inform you of what we're currently working on: 23:02:36 %t."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line("Percent Build Time: 23:02:36 %t", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Time (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Time: \n"
// TBD -- apparently that works as is?!
//advgetopt::getopt::breakup_line("Usage: test usage: ", 0, advgetopt::getopt::get_line_width())
//+ advgetopt::getopt::format_usage_string(
//          "--verbose"
//        , "inform you of what we're currently working on: ."
//        , 30
//        , advgetopt::getopt::get_line_width()) + "\n"
//+ advgetopt::getopt::breakup_line("Percent Build Time: ", 0, advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Build Time (\"\")")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Build Time: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("help_string_version", "[getopt][usage]")
{
    CATCH_START_SECTION("Percent Version (defined)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line("Usage: test usage: 2.0.1-%v", 0, advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
          "--verbose"
        , "inform you of what we're currently working on: 2.0.1-%v."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
  "Percent Version: 2.0.1-%v\n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Version (nullptr)")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Version: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Version (\"\")")
    {
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
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Version: \n"
                );
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("help_string_writable_configuration_files", "[getopt][usage][config]")
{
    CATCH_START_SECTION("Percent Writable Configuration Files (fully defined--one file)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("config_writable_filenames", "writable_filenames");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %w.")
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
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "advgetopt.ini",
            "user.config",
            SNAP_CATCH2_NAMESPACE::g_config_project_filename.c_str(),
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "config_writable_filenames";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %w";
        options.f_help_footer = "Percent Configuration Files: %w";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%w";
        options.f_license = "MIT-%w";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %w";
        options.f_build_date = "Jun  4 2019 %w";
        options.f_build_time = "23:02:36 %w";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose", "inform you of what we're currently working on: "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename + "."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Configuration Files: "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Writable Configuration Files (fully defined)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("config_writable_filenames", "writable_filenames");
        std::string const save_config_filename(SNAP_CATCH2_NAMESPACE::g_config_filename);
        std::string const save_config_project_filename(SNAP_CATCH2_NAMESPACE::g_config_project_filename);

        SNAP_CATCH2_NAMESPACE::init_tmp_dir("config_writable_filenames", "writable_filenames_two");

        {
            std::ofstream config_file;
            config_file.open(save_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(save_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
            ;
        }

        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %w.")
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
            save_config_project_filename.c_str(),
            "advgetopt.conf",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "advgetopt.ini",
            save_config_filename.c_str(),
            "user.config",
            SNAP_CATCH2_NAMESPACE::g_config_project_filename.c_str(),
            nullptr
        };

        advgetopt::options_environment options;
        options.f_project_name = "config_writable_filenames";
        options.f_options = options_list;
        options.f_options_files_directory = "/etc/advgetopt";
        options.f_configuration_files = configuration_files;
        options.f_configuration_filename = "advgetopt.conf";
        options.f_configuration_directories = nullptr;
        options.f_help_header = "Usage: test usage: %w";
        options.f_help_footer = "Percent Configuration Files: %w";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%w";
        options.f_license = "MIT-%w";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %w";
        options.f_build_date = "Jun  4 2019 %w";
        options.f_build_time = "23:02:36 %w";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);

        CATCH_REQUIRE_LONG_STRING(opt.usage(),
advgetopt::getopt::breakup_line(
              "Usage: test usage: "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
            + ", "
            + save_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
+ advgetopt::getopt::format_usage_string(
              "--verbose", "inform you of what we're currently working on: "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
            + ", "
            + save_config_project_filename + "."
        , 30
        , advgetopt::getopt::get_line_width()) + "\n"
+ advgetopt::getopt::breakup_line(
              "Percent Configuration Files: "
            + SNAP_CATCH2_NAMESPACE::g_config_project_filename
            + ", "
            + save_config_project_filename
        , 0
        , advgetopt::getopt::get_line_width())
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Writable Configuration Files (nullptr)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %w.")
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
        options.f_help_header = "Usage: test usage: %w";
        options.f_help_footer = "Percent Configuration Files: %w";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%w";
        options.f_license = "MIT-%w";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %w";
        options.f_build_date = "Jun  4 2019 %w";
        options.f_build_time = "23:02:36 %w";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Percent Writable Configuration Files (empty array)")
    {
        const advgetopt::option options_list[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
                , advgetopt::Help("inform you of what we're currently working on: %w.")
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
        options.f_help_header = "Usage: test usage: %w";
        options.f_help_footer = "Percent Configuration Files: %w";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        options.f_version = "2.0.1-%w";
        options.f_license = "MIT-%w";
        options.f_copyright = "Copyright (c) 2019  Made to Order Software Corp. -- All Rights Reserved %w";
        options.f_build_date = "Jun  4 2019 %w";
        options.f_build_time = "23:02:36 %w";

        advgetopt::getopt opt(options, argc, argv);

        CATCH_REQUIRE(advgetopt::GETOPT_FLAG_SHOW_MOST == 0);
        CATCH_REQUIRE_LONG_STRING(opt.usage(),
"Usage: test usage: \n"
"   --verbose                  inform you of what we're currently working on: .\n"
"\n"
"Percent Configuration Files: \n"
                );
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("invalid_group_for_find_group", "[getopt][usage][config][invalid]")
{
    CATCH_START_SECTION("find_group() with invalid flags")
    {
        for(int idx(0); idx < 32; ++idx)
        {
            advgetopt::flag_t const invalid_group(1UL << idx);
            if((invalid_group & ~advgetopt::GETOPT_FLAG_GROUP_MASK) == 0)
            {
                // this is a valid group, skip
                //
                continue;
            }

            advgetopt::group_description const groups[] =
            {
                advgetopt::define_group(
                      advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
                ),
                advgetopt::define_group(
                      advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
                ),
                advgetopt::end_groups()
            };


            advgetopt::options_environment options;
            options.f_project_name = "unittest";
            options.f_groups = groups;
            advgetopt::getopt opt(options);

            CATCH_REQUIRE_THROWS_MATCHES(
                      opt.find_group(invalid_group)
                    , advgetopt::getopt_logic_error
                    , Catch::Matchers::ExceptionMessage(
                                  "getopt_logic_error: group parameter must represent a valid group."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("find_group() with GETOPT_FLAG_GROUP_NONE")
    {
        advgetopt::group_description const groups[] =
        {
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
            ),
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
            ),
            advgetopt::end_groups()
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_groups = groups;
        advgetopt::getopt opt(options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.find_group(advgetopt::GETOPT_FLAG_GROUP_NONE)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: group NONE cannot be assigned a name so you cannot search for it."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("find_group() with invalid group definitions")
    {
        // define groups without name nor description
        // (later the define_group() will err at compile time on those
        // so we'll have to switch to a "manual" definition instead to
        // verify that this indeed fails as expected.)
        //
        advgetopt::group_description const groups[] =
        {
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
            ),
            advgetopt::define_group(
                  advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
            ),
            advgetopt::end_groups()
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_groups = groups;
        advgetopt::getopt opt(options);

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.find_group(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: at least one of a group name or description must be defined (a non-empty string)."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.find_group(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: at least one of a group name or description must be defined (a non-empty string)."));
    }
    CATCH_END_SECTION()
}








// vim: ts=4 sw=4 et
