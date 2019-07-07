/*
 * Files:
 *    tests/config_files.cpp
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
#include <advgetopt/conf_file.h>
#include <advgetopt/exception.h>

// snapdev lib
//
#include <snapdev/safe_setenv.h>
#include <snapdev/tokenize_string.h>

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




CATCH_TEST_CASE("configuration_spaces", "[config][getopt]")
{
    CATCH_START_SECTION("Verify Configuration Spaces")
        for(int c(0); c < 0x110000; ++c)
        {
            if(c == '\r'
            || c == '\n')
            {
                CATCH_REQUIRE_FALSE(advgetopt::iswspace(c));
            }
            else if(std::iswspace(c))
            {
                CATCH_REQUIRE(advgetopt::iswspace(c));
            }
            else
            {
                CATCH_REQUIRE_FALSE(advgetopt::iswspace(c));
            }
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("configuration_setup", "[config][getopt]")
{
    CATCH_START_SECTION("Check All Setups")
        // 5 * 6 * 8 * 8 * 16 = 30720
        for(int count(0); count < 5; ++count)
        {
            int const id(rand());
            std::string const name("setup-file-" + std::to_string(id));

            init_tmp_dir("setup", name);

            {
                std::ofstream config_file;
                config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "param=optional\n"
                ;
            }

            for(int lc(static_cast<int>(advgetopt::line_continuation_t::single_line));
                lc <= static_cast<int>(advgetopt::line_continuation_t::semicolon);
                ++lc)
            {
                for(advgetopt::assignment_operator_t ao(0);
                    ao <= advgetopt::ASSIGNMENT_OPERATOR_MASK;
                    ++ao)
                {
                    for(advgetopt::comment_t c(0);
                        c < advgetopt::COMMENT_MASK;
                        ++c)
                    {
                        for(advgetopt::section_operator_t so(0);
                            so < advgetopt::SECTION_OPERATOR_MASK;
                            ++so)
                        {
                            advgetopt::conf_file_setup setup(g_config_filename
                                                , static_cast<advgetopt::line_continuation_t>(lc)
                                                , ao
                                                , c
                                                , so);

                            advgetopt::assignment_operator_t real_ao(ao == 0 ? advgetopt::ASSIGNMENT_OPERATOR_EQUAL : ao);

                            CATCH_REQUIRE(setup.is_valid());
                            CATCH_REQUIRE(setup.get_filename() == g_config_filename);
                            CATCH_REQUIRE(setup.get_line_continuation() == static_cast<advgetopt::line_continuation_t>(lc));
                            CATCH_REQUIRE(setup.get_assignment_operator() == real_ao);
                            CATCH_REQUIRE(setup.get_comment() == c);
                            CATCH_REQUIRE(setup.get_section_operator() == so);

                            std::string const url(setup.get_config_url());
//std::cerr << "+++ " << lc << " / " << ao << " / " << c << " / " << so << " URL [" << url << "]\n";
                            CATCH_REQUIRE(url.substr(0, 8) == "file:///");

                            CATCH_REQUIRE(url.substr(7, g_config_filename.length()) == g_config_filename);

                            std::string::size_type const qm_pos(url.find('?'));
                            if(qm_pos == std::string::npos)
                            {
                                // must have the defaults in this case
                                //
                                CATCH_REQUIRE(static_cast<advgetopt::line_continuation_t>(lc) == advgetopt::line_continuation_t::unix);
                                CATCH_REQUIRE(real_ao == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
                                CATCH_REQUIRE(c  == (advgetopt::COMMENT_INI | advgetopt::COMMENT_SHELL));
                                CATCH_REQUIRE(so == advgetopt::SECTION_OPERATOR_INI_FILE);
                            }
                            else
                            {
                                std::string const qs(url.substr(qm_pos + 1));

                                std::vector<std::string> strings;
                                snap::tokenize_string(strings, qs, "&");

                                bool def_lc(true);
                                bool def_ao(true);
                                bool def_c(true);
                                bool def_so(true);

                                for(auto s : strings)
                                {
                                    std::string::size_type const eq_pos(s.find('='));
                                    CATCH_REQUIRE(eq_pos != std::string::npos);

                                    std::string const var_name(s.substr(0, eq_pos));
                                    std::string const var_value(s.substr(eq_pos + 1));

                                    if(var_name == "line-continuation")
                                    {
                                        def_lc = false;
                                        switch(static_cast<advgetopt::line_continuation_t>(lc))
                                        {
                                        case advgetopt::line_continuation_t::single_line:
                                            CATCH_REQUIRE(var_value == "single-line");
                                            break;

                                        case advgetopt::line_continuation_t::rfc_822:
                                            CATCH_REQUIRE(var_value == "rfc-822");
                                            break;

                                        case advgetopt::line_continuation_t::msdos:
                                            CATCH_REQUIRE(var_value == "msdos");
                                            break;

                                        case advgetopt::line_continuation_t::unix:
                                            CATCH_REQUIRE(var_value == "unix");
                                            break;

                                        case advgetopt::line_continuation_t::fortran:
                                            CATCH_REQUIRE(var_value == "fortran");
                                            break;

                                        case advgetopt::line_continuation_t::semicolon:
                                            CATCH_REQUIRE(var_value == "semi-colon");
                                            break;

                                        default:
                                            CATCH_REQUIRE(("unknown_var_value for \"line-continuation\":" + var_value) == std::string());
                                            break;

                                        }
                                    }
                                    else if(var_name == "assignment-operator")
                                    {
                                        def_ao = false;
                                        std::vector<std::string> operators;
                                        snap::tokenize_string(operators, var_value, ",");

                                        if((real_ao & advgetopt::ASSIGNMENT_OPERATOR_EQUAL) != 0)
                                        {
                                            auto it(std::find(operators.begin(), operators.end(), "equal"));
                                            CATCH_REQUIRE(it != operators.end());
                                            operators.erase(it);
                                        }
                                        if((real_ao & advgetopt::ASSIGNMENT_OPERATOR_COLON) != 0)
                                        {
                                            auto it(std::find(operators.begin(), operators.end(), "colon"));
                                            CATCH_REQUIRE(it != operators.end());
                                            operators.erase(it);
                                        }
                                        if((real_ao & advgetopt::ASSIGNMENT_OPERATOR_SPACE) != 0)
                                        {
                                            auto it(std::find(operators.begin(), operators.end(), "space"));
                                            CATCH_REQUIRE(it != operators.end());
                                            operators.erase(it);
                                        }

                                        CATCH_REQUIRE(operators.empty());
                                    }
                                    else if(var_name == "comment")
                                    {
                                        def_c = false;
                                        std::vector<std::string> comments;
                                        snap::tokenize_string(comments, var_value, ",");

                                        if((c & advgetopt::COMMENT_INI) != 0)
                                        {
                                            auto it(std::find(comments.begin(), comments.end(), "ini"));
                                            CATCH_REQUIRE(it != comments.end());
                                            comments.erase(it);
                                        }
                                        if((c & advgetopt::COMMENT_SHELL) != 0)
                                        {
                                            auto it(std::find(comments.begin(), comments.end(), "shell"));
                                            CATCH_REQUIRE(it != comments.end());
                                            comments.erase(it);
                                        }
                                        if((c & advgetopt::COMMENT_CPP) != 0)
                                        {
                                            auto it(std::find(comments.begin(), comments.end(), "cpp"));
                                            CATCH_REQUIRE(it != comments.end());
                                            comments.erase(it);
                                        }
                                        if(c == 0)
                                        {
                                            auto it(std::find(comments.begin(), comments.end(), "none"));
                                            CATCH_REQUIRE(it != comments.end());
                                            comments.erase(it);
                                        }

                                        CATCH_REQUIRE(comments.empty());
                                    }
                                    else if(var_name == "section-operator")
                                    {
                                        def_so = false;
                                        std::vector<std::string> section_operators;
                                        snap::tokenize_string(section_operators, var_value, ",");

                                        if((so & advgetopt::SECTION_OPERATOR_C) != 0)
                                        {
                                            auto it(std::find(section_operators.begin(), section_operators.end(), "c"));
                                            CATCH_REQUIRE(it != section_operators.end());
                                            section_operators.erase(it);
                                        }
                                        if((so & advgetopt::SECTION_OPERATOR_CPP) != 0)
                                        {
                                            auto it(std::find(section_operators.begin(), section_operators.end(), "cpp"));
                                            CATCH_REQUIRE(it != section_operators.end());
                                            section_operators.erase(it);
                                        }
                                        if((so & advgetopt::SECTION_OPERATOR_BLOCK) != 0)
                                        {
                                            auto it(std::find(section_operators.begin(), section_operators.end(), "block"));
                                            CATCH_REQUIRE(it != section_operators.end());
                                            section_operators.erase(it);
                                        }
                                        if((so & advgetopt::SECTION_OPERATOR_INI_FILE) != 0)
                                        {
                                            auto it(std::find(section_operators.begin(), section_operators.end(), "ini-file"));
                                            CATCH_REQUIRE(it != section_operators.end());
                                            section_operators.erase(it);
                                        }

                                        CATCH_REQUIRE(section_operators.empty());
                                    }
                                    else
                                    {
                                        CATCH_REQUIRE(("unknown var_name = " + var_name) == std::string());
                                    }
                                }

                                if(def_lc)
                                {
                                    CATCH_REQUIRE(static_cast<advgetopt::line_continuation_t>(lc) == advgetopt::line_continuation_t::unix);
                                }
                                if(def_ao)
                                {
                                    CATCH_REQUIRE(real_ao == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
                                }
                                if(def_c)
                                {
                                    CATCH_REQUIRE(c == (advgetopt::COMMENT_INI | advgetopt::COMMENT_SHELL));
                                }
                            }
                        }
                    }
                }
            }
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check <empty> URL")
        advgetopt::conf_file_setup setup(
                      "/etc/advgetopt/unknown-file.conf"
                    , advgetopt::line_continuation_t::fortran
                    , advgetopt::ASSIGNMENT_OPERATOR_COLON
                    , advgetopt::COMMENT_INI
                    , advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE_FALSE(setup.is_valid());
        CATCH_REQUIRE(setup.get_filename() == std::string());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::fortran);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_COLON);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_INI);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE(setup.get_config_url() == "file:///<empty>?line-continuation=fortran&assignment-operator=colon&comment=ini&section-operator=cpp");
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("config_line_continuation_tests")
{
    CATCH_START_SECTION("single_line")
        init_tmp_dir("line-continuation", "single-line");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "normal=param\n"
                "\n"
                "rfc-822=start here\n"
                "  continue=there\n"
                "\n"
                "msdos=end with &\n"
                "  and-continue=on next line\n"
                "\n"
                "unix=end with \\\n"
                "to-continue=like this\n"
                "\n"
                "fortran=fortran is funny\n"
                "&since=it starts with an & on the following line\n"
                "\n"
                "semicolon=this ends with\n"
                "a=semi-colon only;\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 11);

        CATCH_REQUIRE(file->has_parameter("normal"));
        CATCH_REQUIRE(file->has_parameter("rfc-822"));
        CATCH_REQUIRE(file->has_parameter("continue"));
        CATCH_REQUIRE(file->has_parameter("msdos"));
        CATCH_REQUIRE(file->has_parameter("and-continue"));
        CATCH_REQUIRE(file->has_parameter("unix"));
        CATCH_REQUIRE(file->has_parameter("to-continue"));
        CATCH_REQUIRE(file->has_parameter("fortran"));
        CATCH_REQUIRE(file->has_parameter("&since"));
        CATCH_REQUIRE(file->has_parameter("semicolon"));
        CATCH_REQUIRE(file->has_parameter("a"));

        CATCH_REQUIRE(file->get_parameter("normal") == "param");
        CATCH_REQUIRE(file->get_parameter("rfc-822") == "start here");
        CATCH_REQUIRE(file->get_parameter("continue") == "there");
        CATCH_REQUIRE(file->get_parameter("msdos") == "end with &");
        CATCH_REQUIRE(file->get_parameter("and-continue") == "on next line");
        CATCH_REQUIRE(file->get_parameter("unix") == "end with \\");
        CATCH_REQUIRE(file->get_parameter("to-continue") == "like this");
        CATCH_REQUIRE(file->get_parameter("fortran") == "fortran is funny");
        CATCH_REQUIRE(file->get_parameter("&since") == "it starts with an & on the following line");
        CATCH_REQUIRE(file->get_parameter("semicolon") == "this ends with");
        CATCH_REQUIRE(file->get_parameter("a") == "semi-colon only;");

        for(int c(0); c < 0x110000; ++c)
        {
            if(c == '=')
            {
                CATCH_REQUIRE(file->is_assignment_operator(c));
            }
            else
            {
                CATCH_REQUIRE_FALSE(file->is_assignment_operator(c));
            }
        }
    CATCH_END_SECTION()

    CATCH_START_SECTION("rfc822")
        init_tmp_dir("line-continuation", "rfc822");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "normal=param\n"
                "\n"
                "rfc-822=start here\n"
                "  continue=there\n"
                "\n"
                "msdos=end with &\n"
                "  and-continue=on next line\n"
                "\n"
                "unix=end with \\\n"
                "to-continue=like this\n"
                "\n"
                "fortran=fortran is funny\n"
                "&since=it starts with an & on the following line\n"
                "\n"
                "semicolon=this ends with\n"
                "a=semi-colon only;\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::rfc_822
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::rfc_822);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 9);

        CATCH_REQUIRE(file->has_parameter("normal"));
        CATCH_REQUIRE(file->has_parameter("rfc-822"));
        CATCH_REQUIRE(file->has_parameter("msdos"));
        CATCH_REQUIRE_FALSE(file->has_parameter("and-continue"));
        CATCH_REQUIRE(file->has_parameter("unix"));
        CATCH_REQUIRE(file->has_parameter("to-continue"));
        CATCH_REQUIRE(file->has_parameter("fortran"));
        CATCH_REQUIRE(file->has_parameter("&since"));
        CATCH_REQUIRE(file->has_parameter("semicolon"));
        CATCH_REQUIRE(file->has_parameter("a"));

        CATCH_REQUIRE(file->get_parameter("normal") == "param");
        CATCH_REQUIRE(file->get_parameter("rfc-822") == "start herecontinue=there");
        CATCH_REQUIRE(file->get_parameter("msdos") == "end with &and-continue=on next line");
        CATCH_REQUIRE(file->get_parameter("and-continue") == std::string());
        CATCH_REQUIRE(file->get_parameter("unix") == "end with \\");
        CATCH_REQUIRE(file->get_parameter("to-continue") == "like this");
        CATCH_REQUIRE(file->get_parameter("fortran") == "fortran is funny");
        CATCH_REQUIRE(file->get_parameter("&since") == "it starts with an & on the following line");
        CATCH_REQUIRE(file->get_parameter("semicolon") == "this ends with");
        CATCH_REQUIRE(file->get_parameter("a") == "semi-colon only;");
    CATCH_END_SECTION()

    CATCH_START_SECTION("msdos")
        init_tmp_dir("line-continuation", "msdos");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "normal=param\n"
                "\n"
                "rfc-822=start here\n"
                "  continue=there\n"
                "\n"
                "msdos=end with &\n"
                "  and-continue=on next line\n"
                "\n"
                "unix=end with \\\n"
                "to-continue=like this\n"
                "\n"
                "fortran=fortran is funny\n"
                "&since=it starts with an & on the following line\n"
                "\n"
                "semicolon=this ends with\n"
                "a=semi-colon only;\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::msdos
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::msdos);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 10);

        CATCH_REQUIRE(file->has_parameter("normal"));
        CATCH_REQUIRE(file->has_parameter("rfc-822"));
        CATCH_REQUIRE(file->has_parameter("continue"));
        CATCH_REQUIRE(file->has_parameter("msdos"));
        CATCH_REQUIRE_FALSE(file->has_parameter("and-continue"));
        CATCH_REQUIRE(file->has_parameter("unix"));
        CATCH_REQUIRE(file->has_parameter("to-continue"));
        CATCH_REQUIRE(file->has_parameter("fortran"));
        CATCH_REQUIRE(file->has_parameter("&since"));
        CATCH_REQUIRE(file->has_parameter("semicolon"));
        CATCH_REQUIRE(file->has_parameter("a"));

        CATCH_REQUIRE(file->get_parameter("normal") == "param");
        CATCH_REQUIRE(file->get_parameter("rfc-822") == "start here");
        CATCH_REQUIRE(file->get_parameter("continue") == "there");
        CATCH_REQUIRE(file->get_parameter("msdos") == "end with   and-continue=on next line");
        CATCH_REQUIRE(file->get_parameter("and-continue") == std::string());
        CATCH_REQUIRE(file->get_parameter("unix") == "end with \\");
        CATCH_REQUIRE(file->get_parameter("to-continue") == "like this");
        CATCH_REQUIRE(file->get_parameter("fortran") == "fortran is funny");
        CATCH_REQUIRE(file->get_parameter("&since") == "it starts with an & on the following line");
        CATCH_REQUIRE(file->get_parameter("semicolon") == "this ends with");
        CATCH_REQUIRE(file->get_parameter("a") == "semi-colon only;");
    CATCH_END_SECTION()

    CATCH_START_SECTION("unix")
        init_tmp_dir("line-continuation", "unix");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "normal=param\n"
                "\n"
                "rfc-822=start here\n"
                "  continue=there\n"
                "\n"
                "msdos=end with &\n"
                "  and-continue=on next line\n"
                "\n"
                "unix=end with \\\n"
                "to-continue=like this\n"
                "\n"
                "fortran=fortran is funny\n"
                "&since=it starts with an & on the following line\n"
                "\n"
                "semicolon=this ends with\n"
                "a=semi-colon only;\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 10);

        CATCH_REQUIRE(file->has_parameter("normal"));
        CATCH_REQUIRE(file->has_parameter("rfc-822"));
        CATCH_REQUIRE(file->has_parameter("continue"));
        CATCH_REQUIRE(file->has_parameter("msdos"));
        CATCH_REQUIRE(file->has_parameter("and-continue"));
        CATCH_REQUIRE(file->has_parameter("unix"));
        CATCH_REQUIRE_FALSE(file->has_parameter("to-continue"));
        CATCH_REQUIRE(file->has_parameter("fortran"));
        CATCH_REQUIRE(file->has_parameter("&since"));
        CATCH_REQUIRE(file->has_parameter("semicolon"));
        CATCH_REQUIRE(file->has_parameter("a"));

        CATCH_REQUIRE(file->get_parameter("normal") == "param");
        CATCH_REQUIRE(file->get_parameter("rfc-822") == "start here");
        CATCH_REQUIRE(file->get_parameter("continue") == "there");
        CATCH_REQUIRE(file->get_parameter("msdos") == "end with &");
        CATCH_REQUIRE(file->get_parameter("and-continue") == "on next line");
        CATCH_REQUIRE(file->get_parameter("unix") == "end with to-continue=like this");
        CATCH_REQUIRE(file->get_parameter("to-continue") == std::string());
        CATCH_REQUIRE(file->get_parameter("fortran") == "fortran is funny");
        CATCH_REQUIRE(file->get_parameter("&since") == "it starts with an & on the following line");
        CATCH_REQUIRE(file->get_parameter("semicolon") == "this ends with");
        CATCH_REQUIRE(file->get_parameter("a") == "semi-colon only;");
    CATCH_END_SECTION()

    CATCH_START_SECTION("fortran")
        init_tmp_dir("line-continuation", "fortran");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "normal=param\n"
                "\n"
                "rfc-822=start here\n"
                "  continue=there\n"
                "\n"
                "msdos=end with &\n"
                "  and-continue=on next line\n"
                "\n"
                "unix=end with \\\n"
                "to-continue=like this\n"
                "\n"
                "fortran=fortran is funny\n"
                "&since=it starts with an & on the following line\n"
                "\n"
                "semicolon=this ends with\n"
                "a=semi-colon only;\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::fortran
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::fortran);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 10);

        CATCH_REQUIRE(file->has_parameter("normal"));
        CATCH_REQUIRE(file->has_parameter("rfc-822"));
        CATCH_REQUIRE(file->has_parameter("continue"));
        CATCH_REQUIRE(file->has_parameter("msdos"));
        CATCH_REQUIRE(file->has_parameter("and-continue"));
        CATCH_REQUIRE(file->has_parameter("unix"));
        CATCH_REQUIRE(file->has_parameter("to-continue"));
        CATCH_REQUIRE(file->has_parameter("fortran"));
        CATCH_REQUIRE_FALSE(file->has_parameter("&since"));
        CATCH_REQUIRE(file->has_parameter("semicolon"));
        CATCH_REQUIRE(file->has_parameter("a"));

        CATCH_REQUIRE(file->get_parameter("normal") == "param");
        CATCH_REQUIRE(file->get_parameter("rfc-822") == "start here");
        CATCH_REQUIRE(file->get_parameter("continue") == "there");
        CATCH_REQUIRE(file->get_parameter("msdos") == "end with &");
        CATCH_REQUIRE(file->get_parameter("and-continue") == "on next line");
        CATCH_REQUIRE(file->get_parameter("unix") == "end with \\");
        CATCH_REQUIRE(file->get_parameter("to-continue") == "like this");
        CATCH_REQUIRE(file->get_parameter("fortran") == "fortran is funnysince=it starts with an & on the following line");
        CATCH_REQUIRE(file->get_parameter("&since") == std::string());
        CATCH_REQUIRE(file->get_parameter("semicolon") == "this ends with");
        CATCH_REQUIRE(file->get_parameter("a") == "semi-colon only;");
    CATCH_END_SECTION()

    CATCH_START_SECTION("semicolon")
        init_tmp_dir("line-continuation", "semicolon");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\r\n"
                "normal=param\r\n"
                "\r\n"
                "rfc-822=start here\r\n"
                "  continue=there\r\n"
                "\r\n"
                "msdos=end with &\r"
                "  and-continue=on next line\r\n"
                "\r\n"
                "unix=end with \\\r\n"
                "to-continue=like this\r"
                "\r\n"
                "fortran=fortran is funny\r\n"
                "&since=it starts with an & on the following line\r\n"
                "\r"
                "semicolon=this ends with\r\n"
                "a=semi-colon only;\r\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::semicolon
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::semicolon);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 1);

        CATCH_REQUIRE(file->has_parameter("normal"));
        CATCH_REQUIRE_FALSE(file->has_parameter("rfc-822"));
        CATCH_REQUIRE_FALSE(file->has_parameter("continue"));
        CATCH_REQUIRE_FALSE(file->has_parameter("msdos"));
        CATCH_REQUIRE_FALSE(file->has_parameter("and-continue"));
        CATCH_REQUIRE_FALSE(file->has_parameter("unix"));
        CATCH_REQUIRE_FALSE(file->has_parameter("to-continue"));
        CATCH_REQUIRE_FALSE(file->has_parameter("fortran"));
        CATCH_REQUIRE_FALSE(file->has_parameter("&since"));
        CATCH_REQUIRE_FALSE(file->has_parameter("semicolon"));
        CATCH_REQUIRE_FALSE(file->has_parameter("a"));

        CATCH_REQUIRE(file->get_parameter("normal") == std::string("param\n"
"\n"
"rfc-822=start here\n"
"  continue=there\n"
"\n"
"msdos=end with &\n"
"  and-continue=on next line\n"
"\n"
"unix=end with \\\n"
"to-continue=like this\n"
"\n"
"fortran=fortran is funny\n"
"&since=it starts with an & on the following line\n"
"\n"
"semicolon=this ends with\n"
"a=semi-colon only"));
        CATCH_REQUIRE(file->get_parameter("rfc-822") == std::string());
        CATCH_REQUIRE(file->get_parameter("continue") == std::string());
        CATCH_REQUIRE(file->get_parameter("msdos") == std::string());
        CATCH_REQUIRE(file->get_parameter("and-continue") == std::string());
        CATCH_REQUIRE(file->get_parameter("unix") == std::string());
        CATCH_REQUIRE(file->get_parameter("to-continue") == std::string());
        CATCH_REQUIRE(file->get_parameter("fortran") == std::string());
        CATCH_REQUIRE(file->get_parameter("&since") == std::string());
        CATCH_REQUIRE(file->get_parameter("semicolon") == std::string());
        CATCH_REQUIRE(file->get_parameter("a") == std::string());
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("config_assignment_operator_tests")
{
    CATCH_START_SECTION("equal")
        init_tmp_dir("assignment-operator", "equal");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "equal=value\n"
                "\n"
                "name:value=127\n"
                "\n"
                "and=no operator\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("equal"));
        CATCH_REQUIRE(file->has_parameter("name:value"));
        CATCH_REQUIRE(file->has_parameter("and"));

        CATCH_REQUIRE(file->get_parameter("equal") == "value");
        CATCH_REQUIRE(file->get_parameter("name:value") == "127");
        CATCH_REQUIRE(file->get_parameter("and") == "no operator");
    CATCH_END_SECTION()

    CATCH_START_SECTION("colon")
        init_tmp_dir("assignment-operator", "colon");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "equal=value\n"
                "\n"
                "name:value=127\n"
                "\n"
                "and=no-operator\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_COLON
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_COLON);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("equal=value"));
        CATCH_REQUIRE(file->has_parameter("name"));
        CATCH_REQUIRE(file->has_parameter("and=no-operator"));

        CATCH_REQUIRE(file->get_parameter("equal=value") == std::string());
        CATCH_REQUIRE(file->get_parameter("name") == "value=127");
        CATCH_REQUIRE(file->get_parameter("and=no-operator") == std::string());
    CATCH_END_SECTION()

    CATCH_START_SECTION("space")
        init_tmp_dir("assignment-operator", "space");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "equal=value\n"
                "\n"
                "name 127\n"
                "\n"
                "and=no operator\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_SPACE
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_SPACE);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("equal=value"));
        CATCH_REQUIRE(file->has_parameter("name"));
        CATCH_REQUIRE(file->has_parameter("and=no"));

        CATCH_REQUIRE(file->get_parameter("equal=value") == std::string());
        CATCH_REQUIRE(file->get_parameter("name") == "127");
        CATCH_REQUIRE(file->get_parameter("and=no") == "operator");
    CATCH_END_SECTION()

    CATCH_START_SECTION("equal_colon_and_space")
        init_tmp_dir("assignment-operator", "all");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "equal=value\n"
                "\n"
                "name: 127\n"
                "\n"
                "and no operator\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            | advgetopt::ASSIGNMENT_OPERATOR_COLON
                            | advgetopt::ASSIGNMENT_OPERATOR_SPACE
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == (advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                                                       | advgetopt::ASSIGNMENT_OPERATOR_COLON
                                                       | advgetopt::ASSIGNMENT_OPERATOR_SPACE));
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("equal"));
        CATCH_REQUIRE(file->has_parameter("name"));
        CATCH_REQUIRE(file->has_parameter("and"));

        CATCH_REQUIRE(file->get_parameter("equal") == "value");
        CATCH_REQUIRE(file->get_parameter("name") == "127");
        CATCH_REQUIRE(file->get_parameter("and") == "no operator");
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("config_comment_tests")
{
    CATCH_START_SECTION("ini comment")
        init_tmp_dir("comment", "ini");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "; Auto-generated\n"
                "#ini=comment\n"
                ";ignore=this one\n"
                "//is=the semi-colon\n"
                ";continuation=with Unix\\\n"
                "also=works for\\\n"
                "comments\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_INI
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_INI);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("#ini"));
        CATCH_REQUIRE(file->has_parameter("//is"));

        CATCH_REQUIRE(file->get_parameter("#ini") == "comment");
        CATCH_REQUIRE(file->get_parameter("//is") == "the semi-colon");
    CATCH_END_SECTION()

    CATCH_START_SECTION("shell comment")
        init_tmp_dir("comment", "shell");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                ";shell=comment\n"
                "#ignore=this one\n"
                "//is=the hash (`#`) character\n"
                "#continuation=with Unix\\\n"
                "also=works for\\\n"
                "comments\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter(";shell"));
        CATCH_REQUIRE(file->has_parameter("//is"));

        CATCH_REQUIRE(file->get_parameter(";shell") == "comment");
        CATCH_REQUIRE(file->get_parameter("//is") == "the hash (`#`) character");
    CATCH_END_SECTION()

    CATCH_START_SECTION("C++ comment")
        init_tmp_dir("comment", "cpp");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "// Auto-generated\n"
                ";cpp=comment\n"
                "//ignore=this one\n"
                "#is=the double slash (`//`)\n"
                "//continuation=with Unix\\\n"
                "also=works for\\\n"
                "comments\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_CPP
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_CPP);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter(";cpp"));
        CATCH_REQUIRE(file->has_parameter("#is"));

        CATCH_REQUIRE(file->get_parameter(";cpp") == "comment");
        CATCH_REQUIRE(file->get_parameter("#is") == "the double slash (`//`)");
    CATCH_END_SECTION()

    CATCH_START_SECTION("All three comments")
        init_tmp_dir("comment", "all-comments");

        {
            std::ofstream config_file;
            config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "// Auto-generated\n"
                "all=comments\n"
                ";ignore=this one\n"
                "together=for powerful config support\n"
                "#continuation=with Unix\\\n"
                "also=works for\\\n"
                "comments\n"
                "but=maybe\n"
                ";we=should\\\n"
                "test=continuation\n"
                "//with=each\\\n"
                "each=type of comment\n"
            ;
        }

        advgetopt::conf_file_setup setup(g_config_filename
                            , advgetopt::line_continuation_t::unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_INI | advgetopt::COMMENT_SHELL | advgetopt::COMMENT_CPP
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == (advgetopt::COMMENT_INI | advgetopt::COMMENT_SHELL | advgetopt::COMMENT_CPP));
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("all"));
        CATCH_REQUIRE(file->has_parameter("together"));
        CATCH_REQUIRE(file->has_parameter("but"));

        CATCH_REQUIRE(file->get_parameter("all") == "comments");
        CATCH_REQUIRE(file->get_parameter("together") == "for powerful config support");
        CATCH_REQUIRE(file->get_parameter("but") == "maybe");
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("invalid_configuration_setup", "[config][getopt][invalid]")
{
    CATCH_START_SECTION("Empty Filename")
        CATCH_REQUIRE_THROWS_MATCHES(
              advgetopt::conf_file_setup(
                              std::string()
                            , static_cast<advgetopt::line_continuation_t>(rand())
                            , rand()
                            , rand()
                            , rand())
            , advgetopt::getopt_exception_invalid
            , Catch::Matchers::ExceptionMessage(
                          "trying to load a configuration file using an empty filename."));
    CATCH_END_SECTION()

    CATCH_START_SECTION("Invalid Line Continuation")
        for(int count(0); count < 5; ++count)
        {
            advgetopt::line_continuation_t lc(advgetopt::line_continuation_t::unix);
            do
            {
                lc = static_cast<advgetopt::line_continuation_t>(rand());
            }
            while(lc >= advgetopt::line_continuation_t::single_line
               && lc <= advgetopt::line_continuation_t::semicolon);

            advgetopt::conf_file_setup setup(
                          "/etc/advgetopt/system.conf"
                        , lc        // <- this is invalid
                        , rand() & advgetopt::ASSIGNMENT_OPERATOR_MASK
                        , rand() & advgetopt::COMMENT_MASK
                        , rand() & advgetopt::SECTION_OPERATOR_MASK);

            CATCH_REQUIRE_FALSE(setup.is_valid());

            CATCH_REQUIRE_THROWS_MATCHES(
                  setup.get_config_url()
                , advgetopt::getopt_exception_logic
                , Catch::Matchers::ExceptionMessage(
                              "unexpected line continuation."));
        }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("missing_configuration_file", "[config][getopt][invalid]")
{
    CATCH_START_SECTION("Create a conf_file without the file")
        for(int count(0); count < 5; ++count)
        {
            int const id(rand());
            std::string const name("setup-file-" + std::to_string(id));

            init_tmp_dir("delete", name);

            {
                std::ofstream config_file;
                config_file.open(g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "param=optional\n"
                ;
            }

            // create the setup while the file still exists
            //
            advgetopt::conf_file_setup setup(
                          g_config_filename
                        , advgetopt::line_continuation_t::unix
                        , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                        , advgetopt::COMMENT_SHELL
                        , advgetopt::SECTION_OPERATOR_NONE);

            // now unlink() that file
            //
            unlink(g_config_filename.c_str());

            // still valid since we do not check again after the
            // constructor ran
            //
            CATCH_REQUIRE(setup.is_valid());
            CATCH_REQUIRE(setup.get_filename() == g_config_filename);
            CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::unix);
            CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
            CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
            CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

            // so when trying to create the conf_file object it fails
            // opening the file
            //
            advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
            CATCH_REQUIRE(file->get_errno() == ENOENT);
        }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
