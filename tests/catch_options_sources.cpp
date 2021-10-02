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


// boost lib
//
#include    <boost/algorithm/string/replace.hpp>


// snapdev lib
//
#include    <snapdev/safe_setenv.h>


// C++ lib
//
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>





CATCH_TEST_CASE("options_sources", "[options][sources][valid]")
{
    CATCH_START_SECTION("System options only")
    {
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::command_flags<
                              advgetopt::GETOPT_FLAG_REQUIRED
                            | advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION>())
                , advgetopt::Help("make it all verbose.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_group_name = "sources";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test valid options from system options only";

        char const * cargv[] =
        {
            "tests/options-parser",
            "--license",
            "--show-option-sources",
            "--version",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        advgetopt::option_info::pointer_t verbose(opt.get_option("verbose"));
        CATCH_REQUIRE(verbose != nullptr);
        verbose->set_value(0, "true");

        advgetopt::option_info::pointer_t copyright(opt.get_option("copyright"));
        CATCH_REQUIRE(copyright != nullptr);
        copyright->set_value(0, "1", advgetopt::option_source_t::SOURCE_DYNAMIC);
        copyright->reset();

        advgetopt::option_info::pointer_t version(opt.get_option("version"));
        CATCH_REQUIRE(version != nullptr);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option \"--version\" can't be directly updated.");
        version->set_value(0, "true");
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // check that we got source info as expected

        advgetopt::option_info::pointer_t license(opt.get_option("license"));
        CATCH_REQUIRE(license != nullptr);
        advgetopt::string_list_t license_sources(license->trace_sources());
        CATCH_REQUIRE(license_sources.size() == 1);
        CATCH_REQUIRE(license_sources[0] == "license= [command-line]");

        advgetopt::option_info::pointer_t show(opt.get_option("show-option-sources"));
        CATCH_REQUIRE(show != nullptr);
        advgetopt::string_list_t show_option_sources(show->trace_sources());
        CATCH_REQUIRE(show_option_sources.size() == 1);
        CATCH_REQUIRE(show_option_sources[0] == "show-option-sources= [command-line]");

        advgetopt::string_list_t verbose_sources(verbose->trace_sources());
        CATCH_REQUIRE(verbose_sources.size() == 1);
        CATCH_REQUIRE(verbose_sources[0] == "verbose=true [direct]");

        advgetopt::option_info::pointer_t help(opt.get_option("help"));
        CATCH_REQUIRE(help != nullptr);
        advgetopt::string_list_t help_sources(help->trace_sources());
        CATCH_REQUIRE(help_sources.size() == 0);

        advgetopt::string_list_t copyright_sources(copyright->trace_sources());
        CATCH_REQUIRE(copyright_sources.size() == 2);
        CATCH_REQUIRE(copyright_sources[0] == "copyright=1 [dynamic]");
        CATCH_REQUIRE(copyright_sources[1] == "copyright [*undefined-source*]");

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == (advgetopt::SYSTEM_OPTION_SHOW_OPTION_SOURCES
                               | advgetopt::SYSTEM_OPTION_VERSION
                               | advgetopt::SYSTEM_OPTION_LICENSE));
        CATCH_REQUIRE_LONG_STRING(ss.str(),
"warning: no version found.\n"
"warning: no license found.\n"
"Option Sources:\n"
"  1. option \"build-date\" (undefined)\n"
"\n"
"  2. option \"compiler-version\" (undefined)\n"
"\n"
"  3. option \"configuration-filenames\" (undefined)\n"
"\n"
"  4. option \"copyright\"\n"
"     copyright=1 [dynamic]\n"
"     copyright [*undefined-source*]\n"
"\n"
"  5. option \"environment-variable-name\" (undefined)\n"
"\n"
"  6. option \"has-sanitizer\" (undefined)\n"
"\n"
"  7. option \"help\" (undefined)\n"
"\n"
"  8. option \"license\"\n"
"     license= [command-line]\n"
"\n"
"  9. option \"path-to-option-definitions\" (undefined)\n"
"\n"
"  10. option \"show-option-sources\"\n"
"     show-option-sources= [command-line]\n"
"\n"
"  11. option \"verbose\"\n"
"     verbose=true [direct]\n"
"\n"
"  12. option \"version\"\n"
"     version= [command-line]\n"
"\n"
);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("options_sources_environment_variable_and_config_file", "[options][sources][valid][config][environment_variable]")
{
    CATCH_START_SECTION("load options from a config file, environment variable, and command line with --config-dir on the command line")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("sources", "src");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "unique     = perfect  \n"
                "definition = long value here\n"
                "sources    = just fine \t\n"
                "multiple   = value\r\n"
                "good       = variable \n"
                "organized  = set\n"
                "more       = data\t \n"
            ;
        }

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("unique")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("unique option.")
                , advgetopt::DefaultValue("long")
            ),
            advgetopt::define_option(
                  advgetopt::Name("color")
                , advgetopt::ShortName('c')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("specify a color.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("definition")
                , advgetopt::ShortName('d')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("complete definition.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("sources")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("where it all came from.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("multiple")
                , advgetopt::ShortName('m')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("whether we have one or more.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("zap")
                , advgetopt::ShortName('z')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("zap all of it.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("good")
                , advgetopt::ShortName('g')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("current status.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("organized")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("how the whole thing was organized.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("more")
                , advgetopt::ShortName('+')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("anything more you can think of.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("anything more you can think of.")
            ),
            advgetopt::end_options()
        };

        snap::safe_setenv env(
                  "ADVGETOPT_TEST_OPTIONS"
                , "--verbose --more=instructions --color black orange purple --sources=all");

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_group_name = "sources";
        environment_options.f_options = options;
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        environment_options.f_configuration_filename = "src.config";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test source from command line, environment variable, and configuration file";
        environment_options.f_version = "1.2.3";

        std::string const config_dir("--config-dir=" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config");
        char const * cargv[] =
        {
            "tests/options-parser",
            "--version",
            "--show-option-sources",
            "--more=magical",
            "--organized",
            "logically",
            "--zap",
            config_dir.c_str(),
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // verify command line options

        advgetopt::option_info::pointer_t version(opt.get_option("version"));
        CATCH_REQUIRE(version != nullptr);
        CATCH_REQUIRE(opt.is_defined("version"));
        advgetopt::string_list_t version_sources(version->trace_sources());
        CATCH_REQUIRE(version_sources.size() == 1);
        CATCH_REQUIRE(version_sources[0] == "version= [command-line]");

        advgetopt::option_info::pointer_t show(opt.get_option("show-option-sources"));
        CATCH_REQUIRE(show != nullptr);
        CATCH_REQUIRE(opt.is_defined("show-option-sources"));
        advgetopt::string_list_t show_option_sources(show->trace_sources());
        CATCH_REQUIRE(show_option_sources.size() == 1);
        CATCH_REQUIRE(show_option_sources[0] == "show-option-sources= [command-line]");

        advgetopt::option_info::pointer_t more(opt.get_option("more"));
        CATCH_REQUIRE(more != nullptr);
        CATCH_REQUIRE(opt.is_defined("more"));
        advgetopt::string_list_t more_sources(more->trace_sources());
        CATCH_REQUIRE(more_sources.size() == 3);
        std::string expected_config("more=data [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]");
        CATCH_REQUIRE(more_sources[0] == expected_config);
        CATCH_REQUIRE(more_sources[1] == "more=instructions [environment-variable]");
        CATCH_REQUIRE(more_sources[2] == "more=magical [command-line]");

        advgetopt::option_info::pointer_t organized(opt.get_option("organized"));
        CATCH_REQUIRE(organized != nullptr);
        CATCH_REQUIRE(opt.is_defined("organized"));
        advgetopt::string_list_t organized_sources(organized->trace_sources());
        CATCH_REQUIRE(organized_sources.size() == 2);
        expected_config = "organized=set [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]";
        CATCH_REQUIRE(organized_sources[0] == expected_config);
        CATCH_REQUIRE(organized_sources[1] == "organized=logically [command-line]");

        advgetopt::option_info::pointer_t zap(opt.get_option("zap"));
        CATCH_REQUIRE(zap != nullptr);
        CATCH_REQUIRE(opt.is_defined("zap"));
        advgetopt::string_list_t zap_sources(zap->trace_sources());
        CATCH_REQUIRE(zap_sources.size() == 1);
        CATCH_REQUIRE(zap_sources[0] == "zap= [command-line]");

        advgetopt::option_info::pointer_t config_dir_opt(opt.get_option("config-dir"));
        CATCH_REQUIRE(config_dir_opt != nullptr);
        CATCH_REQUIRE(opt.is_defined("config_dir"));
        advgetopt::string_list_t config_dir_sources(config_dir_opt->trace_sources());
        CATCH_REQUIRE(config_dir_sources.size() == 1);
        expected_config = "config-dir[0]=" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config [command-line]";
        CATCH_REQUIRE(config_dir_sources[0] == expected_config);

        // verify environment variable options which are not also on the command line

        advgetopt::option_info::pointer_t verbose(opt.get_option("verbose"));
        CATCH_REQUIRE(verbose != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        advgetopt::string_list_t verbose_sources(verbose->trace_sources());
        CATCH_REQUIRE(verbose_sources.size() == 1);
        CATCH_REQUIRE(verbose_sources[0] == "verbose= [environment-variable]");

        advgetopt::option_info::pointer_t color(opt.get_option("color"));
        CATCH_REQUIRE(color != nullptr);
        CATCH_REQUIRE(opt.is_defined("color"));
        advgetopt::string_list_t color_sources(color->trace_sources());
        CATCH_REQUIRE(color_sources.size() == 3);
        CATCH_REQUIRE(color_sources[0] == "color[0]=black [environment-variable]");
        CATCH_REQUIRE(color_sources[1] == "color[1]=orange [environment-variable]");
        CATCH_REQUIRE(color_sources[2] == "color[2]=purple [environment-variable]");

        advgetopt::option_info::pointer_t sources(opt.get_option("sources"));
        CATCH_REQUIRE(sources != nullptr);
        CATCH_REQUIRE(opt.is_defined("sources"));
        advgetopt::string_list_t sources_sources(sources->trace_sources());
        CATCH_REQUIRE(sources_sources.size() == 2);
        expected_config = "sources=just fine [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]";
        CATCH_REQUIRE(sources_sources[0] == expected_config);
        CATCH_REQUIRE(sources_sources[1] == "sources=all [environment-variable]");

        // verify configuration file parameters that appear no where else

        advgetopt::option_info::pointer_t unique(opt.get_option("unique"));
        CATCH_REQUIRE(unique != nullptr);
        CATCH_REQUIRE(opt.is_defined("unique"));
        advgetopt::string_list_t unique_sources(unique->trace_sources());
        CATCH_REQUIRE(unique_sources.size() == 1);
        expected_config = "unique=perfect [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]";
        CATCH_REQUIRE(unique_sources[0] == expected_config);

        advgetopt::option_info::pointer_t definition(opt.get_option("definition"));
        CATCH_REQUIRE(definition != nullptr);
        CATCH_REQUIRE(opt.is_defined("definition"));
        advgetopt::string_list_t definition_sources(definition->trace_sources());
        CATCH_REQUIRE(definition_sources.size() == 1);
        expected_config = "definition=long value here [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]";
        CATCH_REQUIRE(definition_sources[0] == expected_config);

        advgetopt::option_info::pointer_t multiple(opt.get_option("multiple"));
        CATCH_REQUIRE(multiple != nullptr);
        CATCH_REQUIRE(opt.is_defined("multiple"));
        advgetopt::string_list_t multiple_sources(multiple->trace_sources());
        CATCH_REQUIRE(multiple_sources.size() == 1);
        expected_config = "multiple[0]=value [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]";
        CATCH_REQUIRE(multiple_sources[0] == expected_config);

        advgetopt::option_info::pointer_t good(opt.get_option("good"));
        CATCH_REQUIRE(good != nullptr);
        CATCH_REQUIRE(opt.is_defined("good"));
        advgetopt::string_list_t good_sources(good->trace_sources());
        CATCH_REQUIRE(good_sources.size() == 1);
        expected_config = "good=variable [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]";
        CATCH_REQUIRE(good_sources[0] == expected_config);

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == (advgetopt::SYSTEM_OPTION_SHOW_OPTION_SOURCES
                               | advgetopt::SYSTEM_OPTION_VERSION
                               | advgetopt::SYSTEM_OPTION_CONFIG_DIR));
        CATCH_REQUIRE_LONG_STRING(ss.str(),
"1.2.3\n"
"Option Sources:\n"
"  1. option \"build-date\" (undefined)\n"
"\n"
"  2. option \"color\"\n"
"     color[0]=black [environment-variable]\n"
"     color[1]=orange [environment-variable]\n"
"     color[2]=purple [environment-variable]\n"
"\n"
"  3. option \"compiler-version\" (undefined)\n"
"\n"
"  4. option \"config-dir\"\n"
"     config-dir[0]=" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config [command-line]\n"
"\n"
"  5. option \"configuration-filenames\" (undefined)\n"
"\n"
"  6. option \"copyright\" (undefined)\n"
"\n"
"  7. option \"definition\"\n"
"     definition=long value here [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]\n"
"\n"
"  8. option \"environment-variable-name\" (undefined)\n"
"\n"
"  9. option \"good\"\n"
"     good=variable [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]\n"
"\n"
"  10. option \"has-sanitizer\" (undefined)\n"
"\n"
"  11. option \"help\" (undefined)\n"
"\n"
"  12. option \"license\" (undefined)\n"
"\n"
"  13. option \"long-help\" (undefined)\n"
"\n"
"  14. option \"more\"\n"
"     more=data [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]\n"
"     more=instructions [environment-variable]\n"
"     more=magical [command-line]\n"
"\n"
"  15. option \"multiple\"\n"
"     multiple[0]=value [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]\n"
"\n"
"  16. option \"organized\"\n"
"     organized=set [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]\n"
"     organized=logically [command-line]\n"
"\n"
"  17. option \"path-to-option-definitions\" (undefined)\n"
"\n"
"  18. option \"show-option-sources\"\n"
"     show-option-sources= [command-line]\n"
"\n"
"  19. option \"sources\"\n"
"     sources=just fine [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]\n"
"     sources=all [environment-variable]\n"
"\n"
"  20. option \"unique\"\n"
"     unique=perfect [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/src.config\"]\n"
"\n"
"  21. option \"verbose\"\n"
"     verbose= [environment-variable]\n"
"\n"
"  22. option \"version\"\n"
"     version= [command-line]\n"
"\n"
"  23. option \"zap\"\n"
"     zap= [command-line]\n"
"\n"
);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load options from a config file, environment variable, and command line with --config-dir in the environment variable")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("sources", "hidden");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "unique     = perfect  \n"
                "definition = long value here\n"
                "sources    = just fine \t\n"
                "multiple   = the origin of life\r\n"
                "good       = variable \n"
                "organized  = set\n"
                "more       = data\t \n"
            ;
        }

        std::string const sub_config_file(boost::replace_all_copy(
                  SNAP_CATCH2_NAMESPACE::g_config_project_filename
                , "50"
                , "65"));
        {
            std::ofstream config_file;
            config_file.open(sub_config_file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "definition=\"this is a better definition\"\n"
                "#sources=overwrite\n"
                "multiple=should we not see this one instead?\n"
                "good=enhanced\n"
                "# Auto-generated\n"
            ;
        }

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("unique")
                , advgetopt::ShortName('u')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("unique option.")
                , advgetopt::DefaultValue("long")
            ),
            advgetopt::define_option(
                  advgetopt::Name("color")
                , advgetopt::ShortName('c')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("specify a color.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("definition")
                , advgetopt::ShortName('d')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("complete definition.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("sources")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("where it all came from.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("multiple")
                , advgetopt::ShortName('m')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_MULTIPLE
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("whether we have one or more.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("zap")
                , advgetopt::ShortName('z')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("zap all of it.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("good")
                , advgetopt::ShortName('g')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("current status.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("organized")
                , advgetopt::ShortName('o')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP2>())
                , advgetopt::Help("how the whole thing was organized.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("more")
                , advgetopt::ShortName('+')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED
                                                      , advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("anything more you can think of.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("verbose")
                , advgetopt::ShortName('v')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_SHOW_GROUP1>())
                , advgetopt::Help("anything more you can think of.")
            ),
            advgetopt::end_options()
        };

        std::string const config_dir("--config-dir=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config\"");
        snap::safe_setenv env(
                  "ADVGETOPT_TEST_OPTIONS"
                , "--verbose --more=instructions "
                + config_dir
                + " --color black orange purple --sources=all --multiple here too");

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_group_name = "sources";
        environment_options.f_options = options;
        environment_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
        environment_options.f_configuration_filename = "hidden.config";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Usage: test source from command line, environment variable, and configuration file";
        environment_options.f_license = "GPL 2 or GPL 3";

        char const * cargv[] =
        {
            "tests/options-parser",
            "--license",
            "--show-option-sources",
            "--more=magical",
            "--multiple=first",
            "--organized",
            "logically",
            "--zap",
            "--multiple=last",
            "--copyright",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        // verify command line options

        advgetopt::option_info::pointer_t license(opt.get_option("license"));
        CATCH_REQUIRE(license != nullptr);
        CATCH_REQUIRE(opt.is_defined("license"));
        advgetopt::string_list_t license_sources(license->trace_sources());
        CATCH_REQUIRE(license_sources.size() == 1);
        CATCH_REQUIRE(license_sources[0] == "license= [command-line]");

        advgetopt::option_info::pointer_t show(opt.get_option("show-option-sources"));
        CATCH_REQUIRE(show != nullptr);
        CATCH_REQUIRE(opt.is_defined("show-option-sources"));
        advgetopt::string_list_t show_option_sources(show->trace_sources());
        CATCH_REQUIRE(show_option_sources.size() == 1);
        CATCH_REQUIRE(show_option_sources[0] == "show-option-sources= [command-line]");

        advgetopt::option_info::pointer_t more(opt.get_option("more"));
        CATCH_REQUIRE(more != nullptr);
        CATCH_REQUIRE(opt.is_defined("more"));
        advgetopt::string_list_t more_sources(more->trace_sources());
        CATCH_REQUIRE(more_sources.size() == 3);
        std::string expected_config("more=data [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]");
        CATCH_REQUIRE(more_sources[0] == expected_config);
        CATCH_REQUIRE(more_sources[1] == "more=instructions [environment-variable]");
        CATCH_REQUIRE(more_sources[2] == "more=magical [command-line]");

        advgetopt::option_info::pointer_t organized(opt.get_option("organized"));
        CATCH_REQUIRE(organized != nullptr);
        CATCH_REQUIRE(opt.is_defined("organized"));
        advgetopt::string_list_t organized_sources(organized->trace_sources());
        CATCH_REQUIRE(organized_sources.size() == 2);
        expected_config = "organized=set [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]";
        CATCH_REQUIRE(organized_sources[0] == expected_config);
        CATCH_REQUIRE(organized_sources[1] == "organized=logically [command-line]");

        advgetopt::option_info::pointer_t multiple(opt.get_option("multiple"));
        CATCH_REQUIRE(multiple != nullptr);
        CATCH_REQUIRE(opt.is_defined("multiple"));
        advgetopt::string_list_t multiple_sources(multiple->trace_sources());
        CATCH_REQUIRE(multiple_sources.size() == 6);
        expected_config = "multiple[0]=the origin of life [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]";
        CATCH_REQUIRE(multiple_sources[0] == expected_config);
        expected_config = "multiple[0]=should we not see this one instead? [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/sources.d/65-hidden.config\"]";       // TODO: this should not be [0]?
        CATCH_REQUIRE(multiple_sources[1] == expected_config);
        expected_config = "multiple[1]=here [environment-variable]";       // TODO: this should not be [1]?
        CATCH_REQUIRE(multiple_sources[2] == expected_config);
        expected_config = "multiple[2]=too [environment-variable]";       // TODO: this should not be [2]?
        CATCH_REQUIRE(multiple_sources[3] == expected_config);
        expected_config = "multiple[0]=first [command-line]";       // TODO: this should not be [0]?
        CATCH_REQUIRE(multiple_sources[4] == expected_config);
        expected_config = "multiple[0]=last [command-line]";        // TODO: this should not be [0]?
        CATCH_REQUIRE(multiple_sources[5] == expected_config);

        advgetopt::option_info::pointer_t zap(opt.get_option("zap"));
        CATCH_REQUIRE(zap != nullptr);
        CATCH_REQUIRE(opt.is_defined("zap"));
        advgetopt::string_list_t zap_sources(zap->trace_sources());
        CATCH_REQUIRE(zap_sources.size() == 1);
        CATCH_REQUIRE(zap_sources[0] == "zap= [command-line]");

        // verify environment variable options which are not also on the command line

        advgetopt::option_info::pointer_t verbose(opt.get_option("verbose"));
        CATCH_REQUIRE(verbose != nullptr);
        CATCH_REQUIRE(opt.is_defined("verbose"));
        advgetopt::string_list_t verbose_sources(verbose->trace_sources());
        CATCH_REQUIRE(verbose_sources.size() == 1);
        CATCH_REQUIRE(verbose_sources[0] == "verbose= [environment-variable]");

        advgetopt::option_info::pointer_t color(opt.get_option("color"));
        CATCH_REQUIRE(color != nullptr);
        CATCH_REQUIRE(opt.is_defined("color"));
        advgetopt::string_list_t color_sources(color->trace_sources());
        CATCH_REQUIRE(color_sources.size() == 3);
        CATCH_REQUIRE(color_sources[0] == "color[0]=black [environment-variable]");
        CATCH_REQUIRE(color_sources[1] == "color[1]=orange [environment-variable]");
        CATCH_REQUIRE(color_sources[2] == "color[2]=purple [environment-variable]");

        advgetopt::option_info::pointer_t sources(opt.get_option("sources"));
        CATCH_REQUIRE(sources != nullptr);
        CATCH_REQUIRE(opt.is_defined("sources"));
        advgetopt::string_list_t sources_sources(sources->trace_sources());
        CATCH_REQUIRE(sources_sources.size() == 2);
        expected_config = "sources=just fine [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]";
        CATCH_REQUIRE(sources_sources[0] == expected_config);
        CATCH_REQUIRE(sources_sources[1] == "sources=all [environment-variable]");

        advgetopt::option_info::pointer_t config_dir_opt(opt.get_option("config-dir"));
        CATCH_REQUIRE(config_dir_opt != nullptr);
        CATCH_REQUIRE(opt.is_defined("config_dir"));
        advgetopt::string_list_t config_dir_sources(config_dir_opt->trace_sources());
        CATCH_REQUIRE(config_dir_sources.size() == 1);
        expected_config = "config-dir[0]=" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config [environment-variable]";
        CATCH_REQUIRE(config_dir_sources[0] == expected_config);

        // verify configuration file parameters that appear no where else

        advgetopt::option_info::pointer_t unique(opt.get_option("unique"));
        CATCH_REQUIRE(unique != nullptr);
        CATCH_REQUIRE(opt.is_defined("unique"));
        advgetopt::string_list_t unique_sources(unique->trace_sources());
        CATCH_REQUIRE(unique_sources.size() == 1);
        expected_config = "unique=perfect [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]";
        CATCH_REQUIRE(unique_sources[0] == expected_config);

        advgetopt::option_info::pointer_t definition(opt.get_option("definition"));
        CATCH_REQUIRE(definition != nullptr);
        CATCH_REQUIRE(opt.is_defined("definition"));
        advgetopt::string_list_t definition_sources(definition->trace_sources());
        CATCH_REQUIRE(definition_sources.size() == 2);
        expected_config = "definition=long value here [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]";
        CATCH_REQUIRE(definition_sources[0] == expected_config);
        expected_config = "definition=this is a better definition [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/sources.d/65-hidden.config\"]";
        CATCH_REQUIRE(definition_sources[1] == expected_config);

        advgetopt::option_info::pointer_t good(opt.get_option("good"));
        CATCH_REQUIRE(good != nullptr);
        CATCH_REQUIRE(opt.is_defined("good"));
        advgetopt::string_list_t good_sources(good->trace_sources());
        CATCH_REQUIRE(good_sources.size() == 2);
        expected_config = "good=variable [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]";
        CATCH_REQUIRE(good_sources[0] == expected_config);
        expected_config = "good=enhanced [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/sources.d/65-hidden.config\"]";
        CATCH_REQUIRE(good_sources[1] == expected_config);

        // process system options now
        std::stringstream ss;
        advgetopt::flag_t const result(opt.process_system_options(ss));
        CATCH_REQUIRE(result == (advgetopt::SYSTEM_OPTION_SHOW_OPTION_SOURCES
                               | advgetopt::SYSTEM_OPTION_LICENSE
                               | advgetopt::SYSTEM_OPTION_COPYRIGHT
                               | advgetopt::SYSTEM_OPTION_CONFIG_DIR));
        CATCH_REQUIRE_LONG_STRING(ss.str(),
"warning: no copyright notice found.\n"
"GPL 2 or GPL 3\n"
"Option Sources:\n"
"  1. option \"build-date\" (undefined)\n"
"\n"
"  2. option \"color\"\n"
"     color[0]=black [environment-variable]\n"
"     color[1]=orange [environment-variable]\n"
"     color[2]=purple [environment-variable]\n"
"\n"
"  3. option \"compiler-version\" (undefined)\n"
"\n"
"  4. option \"config-dir\"\n"
"     config-dir[0]=" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config [environment-variable]\n"
"\n"
"  5. option \"configuration-filenames\" (undefined)\n"
"\n"
"  6. option \"copyright\"\n"
"     copyright= [command-line]\n"
"\n"
"  7. option \"definition\"\n"
"     definition=long value here [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]\n"
"     definition=this is a better definition [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/sources.d/65-hidden.config\"]\n"
"\n"
"  8. option \"environment-variable-name\" (undefined)\n"
"\n"
"  9. option \"good\"\n"
"     good=variable [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]\n"
"     good=enhanced [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/sources.d/65-hidden.config\"]\n"
"\n"
"  10. option \"has-sanitizer\" (undefined)\n"
"\n"
"  11. option \"help\" (undefined)\n"
"\n"
"  12. option \"license\"\n"
"     license= [command-line]\n"
"\n"
"  13. option \"long-help\" (undefined)\n"
"\n"
"  14. option \"more\"\n"
"     more=data [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]\n"
"     more=instructions [environment-variable]\n"
"     more=magical [command-line]\n"
"\n"
"  15. option \"multiple\"\n"
"     multiple[0]=the origin of life [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]\n"
"     multiple[0]=should we not see this one instead? [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/sources.d/65-hidden.config\"]\n"
"     multiple[1]=here [environment-variable]\n"
"     multiple[2]=too [environment-variable]\n"
"     multiple[0]=first [command-line]\n"
"     multiple[0]=last [command-line]\n"
"\n"
"  16. option \"organized\"\n"
"     organized=set [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]\n"
"     organized=logically [command-line]\n"
"\n"
"  17. option \"path-to-option-definitions\" (undefined)\n"
"\n"
"  18. option \"show-option-sources\"\n"
"     show-option-sources= [command-line]\n"
"\n"
"  19. option \"sources\"\n"
"     sources=just fine [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]\n"
"     sources=all [environment-variable]\n"
"\n"
"  20. option \"unique\"\n"
"     unique=perfect [configuration=\"" + SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/.config/hidden.config\"]\n"
"\n"
"  21. option \"verbose\"\n"
"     verbose= [environment-variable]\n"
"\n"
"  22. option \"version\" (undefined)\n"
"\n"
"  23. option \"zap\"\n"
"     zap= [command-line]\n"
"\n"
);
    }
    CATCH_END_SECTION()
}




// vim: ts=4 sw=4 et
