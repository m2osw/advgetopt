// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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


// C
//
#include    <unistd.h>


// last include
//
#include    <snapdev/poison.h>







CATCH_TEST_CASE("configuration_filenames", "[config][getopt][filenames]")
{
    CATCH_START_SECTION("configuration_filenames: configuration Files")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-any", "any");

        advgetopt::options_environment environment_options;

        char const * confs[] =
        {
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            ".config/file.mdi",
            "/etc/snapwebsites/server.conf",
            nullptr
        };
    
        environment_options.f_project_name = "unittest-any";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing all possible filenames";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        advgetopt::string_list_t const filenames(opt.get_configuration_filenames(false, false));

        CATCH_REQUIRE(filenames.size() == 6);
        CATCH_REQUIRE(filenames[0] == SNAP_CATCH2_NAMESPACE::g_config_filename);
        CATCH_REQUIRE(filenames[1] == SNAP_CATCH2_NAMESPACE::g_config_project_filename);
        CATCH_REQUIRE(filenames[2] == ".config/file.mdi");
        CATCH_REQUIRE(filenames[3] == ".config/unittest-any.d/50-file.mdi");
        CATCH_REQUIRE(filenames[4] == "/etc/snapwebsites/server.conf");
        CATCH_REQUIRE(filenames[5] == "/etc/snapwebsites/unittest-any.d/50-server.conf");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("configuration_filenames: configuration Files (writable)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-writable", "writable");

        advgetopt::options_environment environment_options;

        char const * confs[] =
        {
            ".config/file.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/server.conf",
            nullptr
        };
    
        environment_options.f_project_name = "unittest-writable";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing all possible filenames";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        advgetopt::string_list_t const filenames(opt.get_configuration_filenames(false, true));

        CATCH_REQUIRE(filenames.size() == 3);
        CATCH_REQUIRE(filenames[0] == ".config/unittest-writable.d/50-file.mdi");
        CATCH_REQUIRE(filenames[1] == SNAP_CATCH2_NAMESPACE::g_config_project_filename);
        CATCH_REQUIRE(filenames[2] == "/etc/snapwebsites/unittest-writable.d/50-server.conf");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("configuration_filenames: configuration file + directories")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-with-directories", "with-dirs", true);

        advgetopt::options_environment environment_options;

        char const * dirs[] =
        {
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            ".config",
            "/etc/snapwebsites",
            nullptr
        };
    
        environment_options.f_project_name = "unittest-with-directories";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing all possible filenames";
        environment_options.f_configuration_filename = "snapfirewall.conf";
        environment_options.f_configuration_directories = dirs;

        advgetopt::getopt opt(environment_options);

        advgetopt::string_list_t const filenames(opt.get_configuration_filenames(false, false));

        CATCH_REQUIRE(filenames.size() == 6);
        CATCH_REQUIRE(filenames[0] == SNAP_CATCH2_NAMESPACE::g_config_filename + "/snapfirewall.conf");
        CATCH_REQUIRE(filenames[1] == SNAP_CATCH2_NAMESPACE::g_config_project_filename + "/50-snapfirewall.conf");
        CATCH_REQUIRE(filenames[2] == ".config/snapfirewall.conf");
        CATCH_REQUIRE(filenames[3] == ".config/unittest-with-directories.d/50-snapfirewall.conf");
        CATCH_REQUIRE(filenames[4] == "/etc/snapwebsites/snapfirewall.conf");
        CATCH_REQUIRE(filenames[5] == "/etc/snapwebsites/unittest-with-directories.d/50-snapfirewall.conf");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("configuration_filenames: configuration file + directories + '--config-dir'")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-with-directories-and-config-dir", "with-many-dirs", true);

        advgetopt::options_environment environment_options;

        char const * dirs[] =
        {
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            ".config",
            "/etc/advgetopt",
            nullptr
        };
    
        environment_options.f_project_name = "unittest-with-directories-and-config-dir";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing all possible filenames";
        environment_options.f_configuration_filename = "snapmerger.conf";
        environment_options.f_configuration_directories = dirs;

        char const * cargv[] =
        {
            "/usr/bin/config",
            "--config-dir",
            "/var/lib/advgetopt",
            "--config-dir",
            "/opt/config",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt opt(environment_options, argc, argv);

        advgetopt::string_list_t const filenames(opt.get_configuration_filenames(false, false));

        CATCH_REQUIRE(filenames.size() == 10);
        CATCH_REQUIRE(filenames[0] == "/var/lib/advgetopt/snapmerger.conf");
        CATCH_REQUIRE(filenames[1] == "/var/lib/advgetopt/unittest-with-directories-and-config-dir.d/50-snapmerger.conf");
        CATCH_REQUIRE(filenames[2] == "/opt/config/snapmerger.conf");
        CATCH_REQUIRE(filenames[3] == "/opt/config/unittest-with-directories-and-config-dir.d/50-snapmerger.conf");
        CATCH_REQUIRE(filenames[4] == SNAP_CATCH2_NAMESPACE::g_config_filename + "/snapmerger.conf");
        CATCH_REQUIRE(filenames[5] == SNAP_CATCH2_NAMESPACE::g_config_project_filename + "/50-snapmerger.conf");
        CATCH_REQUIRE(filenames[6] == ".config/snapmerger.conf");
        CATCH_REQUIRE(filenames[7] == ".config/unittest-with-directories-and-config-dir.d/50-snapmerger.conf");
        CATCH_REQUIRE(filenames[8] == "/etc/advgetopt/snapmerger.conf");
        CATCH_REQUIRE(filenames[9] == "/etc/advgetopt/unittest-with-directories-and-config-dir.d/50-snapmerger.conf");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("configuration_filenames: existing configuration files")
    {
        CATCH_WHEN("R/W Config must exist--no user defined config")
        {
            SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-must-exist", "must-be-here");

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "ip=192.168.0.1\n"
                    "wall=iptables\n"
                ;
            }

            unlink(SNAP_CATCH2_NAMESPACE::g_config_project_filename.c_str());

            char const * confs[] =
            {
                SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
                ".config/file-which-was-never-created.mdi",
                "/etc/snapwebsites/not-an-existing-file.conf",
                nullptr
            };

            advgetopt::options_environment environment_options;
            environment_options.f_project_name = "unittest";
            environment_options.f_options = nullptr;
            environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
            environment_options.f_help_header = "Testing all possible filenames";
            environment_options.f_configuration_files = confs;

            advgetopt::getopt opt(environment_options);

            advgetopt::string_list_t const filenames(opt.get_configuration_filenames(true, false));

            CATCH_REQUIRE(filenames.size() == 1);
            CATCH_REQUIRE(filenames[0] == SNAP_CATCH2_NAMESPACE::g_config_filename);
        }

        CATCH_WHEN("R/W Config must exist--user defined config exists")
        {
            SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-user-exist", "existing");

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "block-ip=192.168.6.11\n"
                    "firewall=iptables\n"
                ;
            }

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "ip=10.0.2.5\n"
                    "duration=6h\n"
                ;
            }

            char const * confs[] =
            {
                ".config/file-which-was-never-created.mdi",
                "/etc/snapwebsites/not-an-existing-file.conf",
                SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
                nullptr
            };

            advgetopt::options_environment environment_options;
            environment_options.f_project_name = "unittest-user-exist";
            environment_options.f_options = nullptr;
            environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
            environment_options.f_help_header = "Testing all possible filenames";
            environment_options.f_configuration_files = confs;

            advgetopt::getopt opt(environment_options);

            advgetopt::string_list_t const filenames(opt.get_configuration_filenames(true, false));

            CATCH_REQUIRE(filenames.size() == 2);
            CATCH_REQUIRE(filenames[0] == SNAP_CATCH2_NAMESPACE::g_config_filename);
            CATCH_REQUIRE(filenames[1] == SNAP_CATCH2_NAMESPACE::g_config_project_filename);
        }

        CATCH_WHEN("Writable Config must exist--user defined config exists")
        {
            SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-writable-exist", "present");

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "block-ip=192.168.6.11\n"
                    "firewall=iptables\n"
                ;
            }

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "ip=10.0.2.5\n"
                    "duration=6h\n"
                ;
            }

            char const * confs[] =
            {
                ".config/file-which-was-never-created.mdi",
                SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
                "/etc/snapwebsites/not/an-existing-file.conf",
                nullptr
            };

            advgetopt::options_environment environment_options;
            environment_options.f_project_name = "unittest-writable-exist";
            environment_options.f_options = nullptr;
            environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
            environment_options.f_help_header = "Testing all possible filenames";
            environment_options.f_configuration_files = confs;

            advgetopt::getopt opt(environment_options);

            advgetopt::string_list_t const filenames(opt.get_configuration_filenames(true, true));

            CATCH_REQUIRE(filenames.size() == 1);
            CATCH_REQUIRE(filenames[0] == SNAP_CATCH2_NAMESPACE::g_config_project_filename);
        }

        CATCH_WHEN("Writable Config must exist--user defined config exists and we test with a user folder")
        {
            SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-writable-user", "user-write");

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "block-ip=192.168.6.11\n"
                    "firewall=iptables\n"
                ;
            }

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "ip=10.0.2.5\n"
                    "duration=6h\n"
                ;
            }

            char const * confs[] =
            {
                "~/.config/file-which-was-never-created.mdi",
                SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
                "/etc/snapwebsites/not/an-existing-file.conf",
                nullptr
            };

            advgetopt::options_environment environment_options;
            environment_options.f_project_name = "unittest-writable-user";
            environment_options.f_options = nullptr;
            environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
            environment_options.f_help_header = "Testing all possible filenames";
            environment_options.f_configuration_files = confs;

            advgetopt::getopt opt(environment_options);

            advgetopt::string_list_t const filenames(opt.get_configuration_filenames(true, true));

            CATCH_REQUIRE(filenames.size() == 1);
            CATCH_REQUIRE(filenames[0] == SNAP_CATCH2_NAMESPACE::g_config_project_filename);
        }

        CATCH_WHEN("R/W Config test must exist--user defined config exists and we test with a user folder")
        {
            SNAP_CATCH2_NAMESPACE::init_tmp_dir("unittest-user-folder", "tilde");

            std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir());
            tmpdir += "/.config/home-that-never-gets-created";
            snapdev::safe_setenv env("HOME", tmpdir);

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "ip=10.0.2.5\n"
                    "duration=6h\n"
                ;
            }

            char const * dirs[] =
            {
                "~/.config/folder-which-was-never-created",
                "/etc/snapwebsites/not-an-existing-folder",
                SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
                nullptr
            };

            advgetopt::options_environment environment_options;
            environment_options.f_project_name = "unittest-user-folder";
            environment_options.f_options = nullptr;
            environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
            environment_options.f_help_header = "Testing all possible filenames";
            environment_options.f_configuration_filename = "snapfirewall.conf";
            environment_options.f_configuration_directories = dirs;

            advgetopt::getopt opt(environment_options);

            advgetopt::string_list_t const filenames(opt.get_configuration_filenames(false, false));

            CATCH_REQUIRE(filenames.size() == 5);
            CATCH_REQUIRE(filenames[0] == tmpdir + "/.config/folder-which-was-never-created/snapfirewall.conf");
            CATCH_REQUIRE(filenames[1] == "/etc/snapwebsites/not-an-existing-folder/snapfirewall.conf");
            CATCH_REQUIRE(filenames[2] == "/etc/snapwebsites/not-an-existing-folder/unittest-user-folder.d/50-snapfirewall.conf");
            CATCH_REQUIRE(filenames[3] == SNAP_CATCH2_NAMESPACE::g_config_filename + "/snapfirewall.conf");
            CATCH_REQUIRE(filenames[4] == SNAP_CATCH2_NAMESPACE::g_config_filename + "/unittest-user-folder.d/50-snapfirewall.conf");
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("load_configuration_file", "[config][getopt][filenames]")
{
    CATCH_START_SECTION("load_configuration_file: load a configuration file")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("load", "tool");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=132\n"
                "filenames=green,orange,blue brown white\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        char const * const separators[] {
            ",",
            " ",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filenames")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a list of filenames.")
                , advgetopt::DefaultValue("a.out")
                , advgetopt::Separators(separators)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "load";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading a filenames";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "132");

        CATCH_REQUIRE(opt.size("filenames") == 5);
        CATCH_REQUIRE(opt.get_string("filenames") == "green");
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "green");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "orange");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blue");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "brown");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "white");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_configuration_file: load an extended configuration file")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("load-extended", "extended");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=132\n"
                "object=property.obj\n"
                "filenames=green,orange,blue brown white\n"
                "visibility=hidden\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        char const * const separators[] {
            ",",
            " ",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filenames")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a list of filenames.")
                , advgetopt::DefaultValue("a.out")
                , advgetopt::Separators(separators)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "load-extended";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS | advgetopt::GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS;
        environment_options.f_help_header = "Testing loading filenames";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "132");

        CATCH_REQUIRE(opt.size("filenames") == 5);
        CATCH_REQUIRE(opt.get_string("filenames") == "green");
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "green");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "orange");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blue");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "brown");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "white");

        CATCH_REQUIRE(opt.size("object") == 1);
        CATCH_REQUIRE(opt.get_string("object") == "property.obj");

        CATCH_REQUIRE(opt.size("visibility") == 1);
        CATCH_REQUIRE(opt.get_string("visibility") == "hidden");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_configuration_file: load a configuration file with sections")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("load-with-sections", "sections");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "\n"
                "[integers]\n"
                "sizes=132\n"
                "\n"
                "[objects]\n"
                "object=property.obj\n"
                "filenames=green orange blue brown white\n"
                "\n"
                "[flags]\n"
                "visibility=hidden\n"
                "\n"
                "[integers]\n"
                "max=1111\n"
                "\n"
                "# vim: ts=4 sw=4 et\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        char const * const separators[] {
            ",",
            " ",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("objects::object")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("object.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("integers::sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("objects::filenames")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a list of filenames.")
                , advgetopt::DefaultValue("a.out")
                , advgetopt::Separators(separators)
            ),
            advgetopt::define_option(
                  advgetopt::Name("integers::max")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("maximum value.")
                , advgetopt::DefaultValue("+oo")
            ),
            advgetopt::define_option(
                  advgetopt::Name("flags::visibility")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("visibility.")
                , advgetopt::DefaultValue("flashy")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "load-sections";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading sections";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(opt.size("integers::sizes") == 1);
        CATCH_REQUIRE(opt.get_string("integers::sizes") == "132");

        CATCH_REQUIRE(opt.size("objects::filenames") == 5);
        CATCH_REQUIRE(opt.get_string("objects::filenames") == "green");
        CATCH_REQUIRE(opt.get_string("objects::filenames", 0) == "green");
        CATCH_REQUIRE(opt.get_string("objects::filenames", 1) == "orange");
        CATCH_REQUIRE(opt.get_string("objects::filenames", 2) == "blue");
        CATCH_REQUIRE(opt.get_string("objects::filenames", 3) == "brown");
        CATCH_REQUIRE(opt.get_string("objects::filenames", 4) == "white");

        CATCH_REQUIRE(opt.size("integers::max") == 1);
        CATCH_REQUIRE(opt.get_string("integers::max") == "1111");

        CATCH_REQUIRE(opt.size("objects::object") == 1);
        CATCH_REQUIRE(opt.get_string("objects::object") == "property.obj");

        CATCH_REQUIRE(opt.size("flags::visibility") == 1);
        CATCH_REQUIRE(opt.get_string("flags::visibility") == "hidden");

        std::string const name(advgetopt::CONFIGURATION_SECTIONS);
        CATCH_REQUIRE(opt.size(name) == 3);
        CATCH_REQUIRE(opt.get_string(name) == "flags");
        CATCH_REQUIRE(opt.get_string(name, 0) == "flags");
        CATCH_REQUIRE(opt.get_string(name, 1) == "integers");
        CATCH_REQUIRE(opt.get_string(name, 2) == "objects");
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("load_multiple_configurations", "[config][getopt][filenames]")
{
    CATCH_START_SECTION("load_multiple_configurations: configuration files")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("multiple", "multiplicity");

        advgetopt::options_environment environment_options;

        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir());
        tmpdir += "/.config/home";

        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }

        snapdev::safe_setenv env("HOME", tmpdir);

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "ip=10.0.2.5\n"
                "duration=6h\n"
                "size=604\n"
                "gap=6\n"
                "filename=utf9.txt\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "ip=10.1.7.205\n"
                "gap=9\n"
                "filename=utf7.txt\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(tmpdir + "/advgetopt.conf", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "duration=105min\n"
                "filename=utf8.txt\n"
            ;
        }

        char const * confs[] =
        {
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            SNAP_CATCH2_NAMESPACE::g_config_project_filename.c_str(),
            "~/advgetopt.conf",
            nullptr
        };
    
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("size.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filename")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a filenames.")
                , advgetopt::DefaultValue("a.out")
            ),
            advgetopt::define_option(
                  advgetopt::Name("duration")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("how long it lasts.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("gap")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("gap size.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("ip")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("enter the ip address.")
            ),
            advgetopt::end_options()
        };

        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing a load with multiple filenames and see that we get the latest";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        opt.parse_configuration_files(0, nullptr);

        CATCH_REQUIRE(opt.size("size") == 1);
        CATCH_REQUIRE(opt.get_string("size") == "604");

        // although it is marked as multiple, the old entries are still
        // overwritten with newer versions; if the last entry had multiple
        // filenames, then we'd get get multiple names here
        //
        CATCH_REQUIRE(opt.size("filename") == 1);
        CATCH_REQUIRE(opt.get_string("filename", 0) == "utf8.txt");

        CATCH_REQUIRE(opt.size("duration") == 1);
        CATCH_REQUIRE(opt.get_string("duration") == "105min");

        CATCH_REQUIRE(opt.size("gap") == 1);
        CATCH_REQUIRE(opt.get_string("gap") == "9");

        CATCH_REQUIRE(opt.size("ip") == 1);
        CATCH_REQUIRE(opt.get_string("ip") == "10.1.7.205");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_multiple_configurations: configuration files with sections")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("multiple-with-sections", "multiplicity-with-sections");

        advgetopt::options_environment environment_options;

        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir());
        tmpdir += "/.config/home2";

        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }

        snapdev::safe_setenv env("HOME", tmpdir);

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "\n"
                "[connection]\n"
                "ip=10.0.2.5\n"
                "duration=6h\n"
                "size=604\n"
                "\n"
                "[data-settings]\n"
                "gap=6\n"
                "filename=utf9.txt\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "\n"
                "[connection]\n"
                "duration=3min\n"
                "\n"
                "[data-settings]\n"
                "gap=9\n"
                "filename=utf7.txt\n"
            ;
        }

        {
            std::ofstream config_file;
            config_file.open(tmpdir + "/advgetopt.conf", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "\n"
                "[connection]\n"
                "ip=192.168.255.3\n"
                "\n"
                "[data-settings]\n"
                "filename=utf8.txt\n"
            ;
        }

        char const * confs[] =
        {
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            SNAP_CATCH2_NAMESPACE::g_config_project_filename.c_str(),
            "~/advgetopt.conf",
            nullptr
        };
    
        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("connection::size")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("size.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("data-settings::filename")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a filenames.")
                , advgetopt::DefaultValue("a.out")
            ),
            advgetopt::define_option(
                  advgetopt::Name("connection::duration")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("how long it lasts.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("data-settings::gap")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("gap size.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("connection::ip")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("enter the ip address.")
            ),
            advgetopt::end_options()
        };

        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing a load with multiple filenames and see that we get the latest";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        opt.parse_configuration_files(0, nullptr);

        CATCH_REQUIRE(opt.size("connection::size") == 1);
        CATCH_REQUIRE(opt.get_string("connection::size") == "604");

        // although it is marked as multiple, the old entries are still
        // overwritten with newer versions; if the last entry had multiple
        // filenames, then we'd get get multiple names here
        //
        CATCH_REQUIRE(opt.size("data-settings::filename") == 1);
        CATCH_REQUIRE(opt.get_string("data-settings::filename", 0) == "utf8.txt");

        CATCH_REQUIRE(opt.size("connection::duration") == 1);
        CATCH_REQUIRE(opt.get_string("connection::duration") == "3min");

        CATCH_REQUIRE(opt.size("data-settings::gap") == 1);
        CATCH_REQUIRE(opt.get_string("data-settings::gap") == "9");

        CATCH_REQUIRE(opt.size("connection::ip") == 1);
        CATCH_REQUIRE(opt.get_string("connection::ip") == "192.168.255.3");

        std::string const name(advgetopt::CONFIGURATION_SECTIONS);
        CATCH_REQUIRE(opt.size(name) == 2);
        CATCH_REQUIRE(opt.get_string(name) == "connection");
        CATCH_REQUIRE(opt.get_string(name, 1) == "data-settings");
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("load_invalid_configuration_file", "[config][getopt][filenames][invalid]")
{
    CATCH_START_SECTION("load_invalid_configuration_file: load with unexpected parameter name (one letter--dynamic allowed)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("loading-invalid", "invalid-one-letter");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=-132\n"
                "f=dynamic\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        char const * const separators[] {
            ",",
            " ",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filenames")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a list of filenames.")
                , advgetopt::DefaultValue("a.out")
                , advgetopt::Separators(separators)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "loading-invalid";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS | advgetopt::GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS;
        environment_options.f_help_header = "Testing loading a one letter parameter";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: unknown option \"f\" found in configuration file \""
                + SNAP_CATCH2_NAMESPACE::g_config_filename
                + "\" on line 3.");
        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-132");
        CATCH_REQUIRE(opt.get_long("sizes") == -132);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_invalid_configuration_file: load with unexpected parameter name (one letter--no dynamic allowed)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("loading-undefined", "undefined-one-letter");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=-132\n"
                "f=dynamic\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        char const * const separators[] {
            ",",
            " ",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filenames")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a list of filenames.")
                , advgetopt::DefaultValue("a.out")
                , advgetopt::Separators(separators)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "loading-invalid";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading a one letter parameter";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: unknown option \"f\" found in configuration file \""
                + SNAP_CATCH2_NAMESPACE::g_config_filename
                + "\" on line 3.");
        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-132");
        CATCH_REQUIRE(opt.get_long("sizes") == -132);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_invalid_configuration_file: load with unexpected parameter name (undefined & no dynamic fields are allowed)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("loading-invalid-dynamic", "invalid-dynamic");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=-1001\n"
                "dynamic=\"undefined argument\"\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        char const * const separators[] {
            ",",
            " ",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filenames")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a list of filenames.")
                , advgetopt::DefaultValue("a.out")
                , advgetopt::Separators(separators)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "loading-invalid";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading an unknown parameter and no dynamic allowed";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: unknown option \"dynamic\" found in configuration file \""
                + SNAP_CATCH2_NAMESPACE::g_config_filename
                + "\" on line 3.");
        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-1001");
        CATCH_REQUIRE(opt.get_long("sizes") == -1001);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_invalid_configuration_file: load with parameter not supported in configuration files")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("loading-invalid-config", "invalid-param-in-config");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=-1001\n"
                "filenames=unexpected, argument, in, configuration, file\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        char const * const separators[] {
            ",",
            " ",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("filenames")
                , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED, advgetopt::GETOPT_FLAG_MULTIPLE>())
                , advgetopt::Help("enter a list of filenames.")
                , advgetopt::DefaultValue("a.out")
                , advgetopt::Separators(separators)
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "loading-invalid";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading an unknown parameter and no dynamic allowed";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: option \"filenames\" is not supported in configuration files (found in \""
                + SNAP_CATCH2_NAMESPACE::g_config_filename
                + "\").");
        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-1001");
        CATCH_REQUIRE(opt.get_long("sizes") == -1001);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_invalid_configuration_file: load a configuration file with a flag given a value other than true or false")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("load-flag-with-value", "unexpected-value-in-config");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=4153629\n"
                "color-flag=turn it on\n"   // not true or false
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("color-flag")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("flag that you want color.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "load";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading an invalid flag";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: option \"color_flag\" cannot be given value \"turn it on\" in configuration file \""
                + SNAP_CATCH2_NAMESPACE::g_config_filename
                + "\". It only accepts \"true\" or \"false\".");
        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "4153629");

        CATCH_REQUIRE(opt.size("color-flag") == 0);
        CATCH_REQUIRE_FALSE(opt.is_defined("color-flag"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_invalid_configuration_file: load a configuration file with a flag given the value \"true\"")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("load-flag-with-true", "true-value-in-config");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=4153629\n"
                "color-flag=true\n"   // true is like specifying it on the command line
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("color-flag")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("flag that you want color.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "load";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading an invalid flag";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "4153629");

        CATCH_REQUIRE(opt.size("color-flag") == 1);
        CATCH_REQUIRE(opt.is_defined("color-flag"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_invalid_configuration_file: load a configuration file with a flag given the value \"false\"")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("load-flag-with-false", "false-value-in-config");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "sizes=4153629\n"
                "color-flag=false\n"   // false is like "unspecifying" it on the command line
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name("color-flag")
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_FLAG>())
                , advgetopt::Help("flag that you want color.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "load";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading an invalid flag";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "4153629");

        CATCH_REQUIRE(opt.size("color-flag") == 0);
        CATCH_REQUIRE_FALSE(opt.is_defined("color-flag"));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load_invalid_configuration_file: load a configuration file with an invalid sections definition")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("load-with-invalid-sections", "invalid-sections");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "[integers]\n"
                "sizes=639\n"
            ;
        }

        char const * confs[] =
        {
            "~/.config/file-which-was-never-created.mdi",
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            "/etc/snapwebsites/not/an-existing-file.conf",
            nullptr
        };

        advgetopt::option const options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("integers::sizes")
                , advgetopt::ShortName('s')
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("sizes.")
            ),
            advgetopt::define_option(
                  advgetopt::Name(advgetopt::CONFIGURATION_SECTIONS)
                , advgetopt::Flags(advgetopt::all_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
                , advgetopt::Help("MULTIPLE missing.")
            ),
            advgetopt::end_options()
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "load-invalid-sections";
        environment_options.f_options = options;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing loading invalid sections declaration";
        environment_options.f_configuration_files = confs;

        advgetopt::getopt opt(environment_options);

        SNAP_CATCH2_NAMESPACE::push_expected_log("error: option \"configuration_sections\" must have GETOPT_FLAG_MULTIPLE set.");
        opt.process_configuration_file(SNAP_CATCH2_NAMESPACE::g_config_filename);
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        // it failed early so it's not considered to be 100% initialized
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  opt.size("integers::sizes-parameter")
                , advgetopt::getopt_initialization
                , Catch::Matchers::ExceptionMessage(
                              "getopt_exception: function called too soon, parser is not done yet (i.e. is_defined(), get_string(), get_long(), get_double() cannot be called until the parser is done)"));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et nowrap
