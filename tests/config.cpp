/*
 * Files:
 *    tests/config.cpp
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

// C lib
//
#include <unistd.h>



namespace
{


std::string     g_config_filename;
std::string     g_config_project_filename;

void init_tmp_dir(std::string const & project_name, std::string const & prefname)
{
    std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
    tmpdir += "/.config";
    std::stringstream ss;
    ss << "mkdir -p " << tmpdir << "/" << project_name << ".d";
    if(system(ss.str().c_str()) != 0)
    {
        std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
        exit(1);
    }
    g_config_filename = tmpdir + "/" + prefname + ".config";
    g_config_project_filename = tmpdir + "/" + project_name + ".d/" + prefname + ".config";
}



} // no name namespace




CATCH_TEST_CASE("configuration_filenames", "[config][getopt][filenames]")
{
    CATCH_START_SECTION("Configuration Files")
        init_tmp_dir("unittest-any", "any");

        advgetopt::options_environment environment_options;

        char const * confs[] =
        {
            g_config_filename.c_str(),
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
        CATCH_REQUIRE(filenames[0] == g_config_filename);
        CATCH_REQUIRE(filenames[1] == g_config_project_filename);
        CATCH_REQUIRE(filenames[2] == ".config/file.mdi");
        CATCH_REQUIRE(filenames[3] == ".config/unittest-any.d/file.mdi");
        CATCH_REQUIRE(filenames[4] == "/etc/snapwebsites/server.conf");
        CATCH_REQUIRE(filenames[5] == "/etc/snapwebsites/unittest-any.d/server.conf");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Configuration Files (writable)")
        init_tmp_dir("unittest-writable", "writable");

        advgetopt::options_environment environment_options;

        char const * confs[] =
        {
            ".config/file.mdi",
            g_config_filename.c_str(),
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
        CATCH_REQUIRE(filenames[0] == ".config/unittest-writable.d/file.mdi");
        CATCH_REQUIRE(filenames[1] == g_config_project_filename);
        CATCH_REQUIRE(filenames[2] == "/etc/snapwebsites/unittest-writable.d/server.conf");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Configuration File + Directories")
        init_tmp_dir("unittest-with-directories", "with-dirs");

        advgetopt::options_environment environment_options;

        char const * dirs[] =
        {
            g_config_filename.c_str(),
            ".config",
            "/etc/snapwebsites",
            nullptr
        };
    
        environment_options.f_project_name = "unittest";
        environment_options.f_options = nullptr;
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_help_header = "Testing all possible filenames";
        environment_options.f_configuration_filename = "snapfirewall.conf";
        environment_options.f_configuration_directories = dirs;

        advgetopt::getopt opt(environment_options);

        advgetopt::string_list_t const filenames(opt.get_configuration_filenames(false, false));

        CATCH_REQUIRE(filenames.size() == 6);
        CATCH_REQUIRE(filenames[0] == "/home/snapwebsites/snapcpp/contrib/advgetopt/tmp/advgetopt/.config/with-dirs.config/snapfirewall.conf");
        CATCH_REQUIRE(filenames[1] == "/home/snapwebsites/snapcpp/contrib/advgetopt/tmp/advgetopt/.config/with-dirs.config/unittest.d/snapfirewall.conf");
        CATCH_REQUIRE(filenames[2] == ".config/snapfirewall.conf");
        CATCH_REQUIRE(filenames[3] == ".config/unittest.d/snapfirewall.conf");
        CATCH_REQUIRE(filenames[4] == "/etc/snapwebsites/snapfirewall.conf");
        CATCH_REQUIRE(filenames[5] == "/etc/snapwebsites/unittest.d/snapfirewall.conf");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Existing Configuration Files")

        CATCH_WHEN("R/W Config must exist--no user defined config")
        {
            init_tmp_dir("unittest-must-exist", "must-be-here");

            {
                std::ofstream config_file;
                config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "ip=192.168.0.1\n"
                    "wall=iptables\n"
                ;
            }

            unlink(g_config_project_filename.c_str());

            char const * confs[] =
            {
                g_config_filename.c_str(),
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
            CATCH_REQUIRE(filenames[0] == g_config_filename);
        }

        CATCH_WHEN("R/W Config must exist--user defined config exists")
        {
            init_tmp_dir("unittest-user-exist", "existing");

            {
                std::ofstream config_file;
                config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "block-ip=192.168.6.11\n"
                    "firewall=iptables\n"
                ;
            }

            {
                std::ofstream config_file;
                config_file.open(g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
                g_config_filename.c_str(),
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
            CATCH_REQUIRE(filenames[0] == g_config_filename);
            CATCH_REQUIRE(filenames[1] == g_config_project_filename);
        }

        CATCH_WHEN("Writable Config must exist--user defined config exists")
        {
            init_tmp_dir("unittest-writable-exist", "present");

            {
                std::ofstream config_file;
                config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "block-ip=192.168.6.11\n"
                    "firewall=iptables\n"
                ;
            }

            {
                std::ofstream config_file;
                config_file.open(g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
                g_config_filename.c_str(),
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
            CATCH_REQUIRE(filenames[0] == g_config_project_filename);
        }

        CATCH_WHEN("Writable Config must exist--user defined config exists and we test with a user folder")
        {
            init_tmp_dir("unittest-writable-user", "user-write");

            {
                std::ofstream config_file;
                config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "block-ip=192.168.6.11\n"
                    "firewall=iptables\n"
                ;
            }

            {
                std::ofstream config_file;
                config_file.open(g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
                g_config_filename.c_str(),
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
            CATCH_REQUIRE(filenames[0] == g_config_project_filename);
        }

        CATCH_WHEN("R/W Config test must exist--user defined config exists and we test with a user folder")
        {
            init_tmp_dir("unittest-user-folder", "tilde");

            {
                std::ofstream config_file;
                config_file.open(g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
                g_config_filename.c_str(),
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
            CATCH_REQUIRE(filenames[0] == "/home/alexis/.config/folder-which-was-never-created/snapfirewall.conf");
            CATCH_REQUIRE(filenames[1] == "/etc/snapwebsites/not-an-existing-folder/snapfirewall.conf");
            CATCH_REQUIRE(filenames[2] == "/etc/snapwebsites/not-an-existing-folder/unittest-user-folder.d/snapfirewall.conf");
            CATCH_REQUIRE(filenames[3] == g_config_filename + "/snapfirewall.conf");
            CATCH_REQUIRE(filenames[4] == g_config_filename + "/unittest-user-folder.d/snapfirewall.conf");
        }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("load_configuration_file", "[config][getopt][filenames]")
{
    CATCH_START_SECTION("Load a Configuration File")
        init_tmp_dir("load", "tool");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            g_config_filename.c_str(),
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

        opt.process_configuration_file(g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "132");

        CATCH_REQUIRE(opt.size("filenames") == 5);
        CATCH_REQUIRE(opt.get_string("filenames") == "green");
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "green");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "orange");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blue");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "brown");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "white");
    CATCH_END_SECTION()

    CATCH_START_SECTION("Load an Extended Configuration File")
        init_tmp_dir("load-extended", "extended");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            g_config_filename.c_str(),
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

        opt.process_configuration_file(g_config_filename);

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
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("load_multiple_configurations", "[config][getopt][filenames]")
{
    init_tmp_dir("multiple", "multiplicity");

//string_list_t getopt::get_configuration_filenames(bool exists, bool writable)

    CATCH_START_SECTION("Configuration Files")
        advgetopt::options_environment environment_options;

        std::string tmpdir(SNAP_CATCH2_NAMESPACE::g_tmp_dir);
        tmpdir += "/.config/home";

        std::stringstream ss;
        ss << "mkdir -p " << tmpdir;
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
            exit(1);
        }

        snap::safe_setenv env("HOME", tmpdir);

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            config_file.open(g_config_project_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            g_config_filename.c_str(),
            g_config_project_filename.c_str(),
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

        opt.parse_configuration_files();

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
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("load_invalid_configuration_file", "[config][getopt][filenames][invalid]")
{
    CATCH_START_SECTION("Load with Unexpected Parameter Name (one letter--dynamic allowed)")
        init_tmp_dir("loading-invalid", "invalid-one-letter");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            g_config_filename.c_str(),
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

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: unknown option \"f\" found in configuration file \""
                + g_config_filename
                + "\".");
        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-132");
        CATCH_REQUIRE(opt.get_long("sizes") == -132);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Load with Unexpected Parameter Name (one letter--no dynamic allowed)")
        init_tmp_dir("loading-invalid", "invalid-one-letter");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            g_config_filename.c_str(),
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

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: unknown option \"f\" found in configuration file \""
                + g_config_filename
                + "\".");
        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-132");
        CATCH_REQUIRE(opt.get_long("sizes") == -132);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Load with Unexpected Parameter Name (undefined & no dynamic fields are allowed)")
        init_tmp_dir("loading-invalid", "invalid-dynamic");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            g_config_filename.c_str(),
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

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: unknown option \"dynamic\" found in configuration file \""
                + g_config_filename
                + "\".");
        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-1001");
        CATCH_REQUIRE(opt.get_long("sizes") == -1001);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    CATCH_END_SECTION()

    CATCH_START_SECTION("Load with Parameter not Supported in Configuration Files")
        init_tmp_dir("loading-invalid-config", "invalid-param-in-config");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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
            g_config_filename.c_str(),
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

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: option \"filenames\" is not supported in configuration files (found in \""
                + g_config_filename
                + "\").");
        advgetopt::getopt opt(environment_options);

        opt.process_configuration_file(g_config_filename);

        CATCH_REQUIRE(opt.size("sizes") == 1);
        CATCH_REQUIRE(opt.get_string("sizes") == "-1001");
        CATCH_REQUIRE(opt.get_long("sizes") == -1001);

        CATCH_REQUIRE(opt.size("filenames") == 0);
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et