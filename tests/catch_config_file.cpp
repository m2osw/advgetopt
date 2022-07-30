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

// advgetopt
//
#include    <advgetopt/conf_file.h>

#include    <advgetopt/exception.h>


// self
//
#include    "catch_main.h"


// snapdev
//
#include    <snapdev/safe_setenv.h>
#include    <snapdev/tokenize_string.h>


// C++
//
#include    <fstream>
#include    <iomanip>


// C
//
#include    <unistd.h>


// last include
//
#include    <snapdev/poison.h>







CATCH_TEST_CASE("configuration_spaces", "[config][getopt][valid]")
{
    CATCH_START_SECTION("Verify Configuration Spaces")
    {
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
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("configuration_setup", "[config][getopt][valid]")
{
    CATCH_START_SECTION("Check All Setups")
    {
        // 5 * 6 * 8 * 8 * 16 = 30720
        for(int count(0); count < 5; ++count)
        {
            int const id(rand());
            std::string const name("setup-file-" + std::to_string(id));

            SNAP_CATCH2_NAMESPACE::init_tmp_dir("setup", name);

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "param=optional\n"
                ;
            }

            for(int lc(static_cast<int>(advgetopt::line_continuation_t::line_continuation_single_line));
                lc <= static_cast<int>(advgetopt::line_continuation_t::line_continuation_semicolon);
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
                            advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                                                , static_cast<advgetopt::line_continuation_t>(lc)
                                                , ao
                                                , c
                                                , so);

                            advgetopt::assignment_operator_t real_ao(ao == 0 ? advgetopt::ASSIGNMENT_OPERATOR_EQUAL : ao);

                            CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

                            CATCH_REQUIRE(setup.is_valid());
                            std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
                            CATCH_REQUIRE(setup.get_filename() == fn.get());
                            CATCH_REQUIRE(setup.get_line_continuation() == static_cast<advgetopt::line_continuation_t>(lc));
                            CATCH_REQUIRE(setup.get_assignment_operator() == real_ao);
                            CATCH_REQUIRE(setup.get_comment() == c);
                            CATCH_REQUIRE(setup.get_section_operator() == so);

                            std::string const url(setup.get_config_url());
//std::cerr << "+++ " << lc << " / " << ao << " / " << c << " / " << so << " URL [" << url << "]\n";
                            CATCH_REQUIRE(url.substr(0, 8) == "file:///");

                            CATCH_REQUIRE(url.substr(7, strlen(fn.get())) == fn.get());

                            std::string::size_type const qm_pos(url.find('?'));
                            if(qm_pos == std::string::npos)
                            {
                                // must have the defaults in this case
                                //
                                CATCH_REQUIRE(static_cast<advgetopt::line_continuation_t>(lc) == advgetopt::line_continuation_t::line_continuation_unix);
                                CATCH_REQUIRE(real_ao == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
                                CATCH_REQUIRE(c  == (advgetopt::COMMENT_INI | advgetopt::COMMENT_SHELL));
                                CATCH_REQUIRE(so == advgetopt::SECTION_OPERATOR_INI_FILE);
                            }
                            else
                            {
                                std::string const qs(url.substr(qm_pos + 1));

                                std::vector<std::string> strings;
                                snapdev::tokenize_string(strings, qs, "&");

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
                                        case advgetopt::line_continuation_t::line_continuation_single_line:
                                            CATCH_REQUIRE(var_value == "single-line");
                                            break;

                                        case advgetopt::line_continuation_t::line_continuation_rfc_822:
                                            CATCH_REQUIRE(var_value == "rfc-822");
                                            break;

                                        case advgetopt::line_continuation_t::line_continuation_msdos:
                                            CATCH_REQUIRE(var_value == "msdos");
                                            break;

                                        case advgetopt::line_continuation_t::line_continuation_unix:
                                            CATCH_REQUIRE(var_value == "unix");
                                            break;

                                        case advgetopt::line_continuation_t::line_continuation_fortran:
                                            CATCH_REQUIRE(var_value == "fortran");
                                            break;

                                        case advgetopt::line_continuation_t::line_continuation_semicolon:
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
                                        snapdev::tokenize_string(operators, var_value, ",");

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
                                        snapdev::tokenize_string(comments, var_value, ",");

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
                                        if(c == advgetopt::COMMENT_NONE)
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
                                        snapdev::tokenize_string(section_operators, var_value, ",");

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
                                    CATCH_REQUIRE(static_cast<advgetopt::line_continuation_t>(lc) == advgetopt::line_continuation_t::line_continuation_unix);
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Check non-existant filename")
    {
        advgetopt::conf_file_setup setup(
                      "/etc/advgetopt/unknown-file.conf"
                    , advgetopt::line_continuation_t::line_continuation_fortran
                    , advgetopt::ASSIGNMENT_OPERATOR_COLON
                    , advgetopt::COMMENT_INI
                    , advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE(setup.get_original_filename() == "/etc/advgetopt/unknown-file.conf");

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_filename() == "/etc/advgetopt/unknown-file.conf");
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_fortran);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_COLON);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_INI);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE(setup.get_config_url() == "file:///etc/advgetopt/unknown-file.conf?line-continuation=fortran&assignment-operator=colon&comment=ini&section-operator=cpp");
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("config_reload_tests", "[config][getopt][valid]")
{
    CATCH_START_SECTION("Load a file, update it, verify it does not get reloaded")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("reload", "load-twice");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "param=value\n"
                "changing=without reloading is useless\n"
                "test=1009\n"
            ;
        }

        advgetopt::conf_file::pointer_t file1;
        {
            advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                                , advgetopt::line_continuation_t::line_continuation_single_line
                                , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                                , advgetopt::COMMENT_SHELL
                                , advgetopt::SECTION_OPERATOR_NONE);

            CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

            CATCH_REQUIRE(setup.is_valid());
            CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
            CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
            CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
            CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

            file1 = advgetopt::conf_file::get_conf_file(setup);

            CATCH_REQUIRE(file1->get_setup().get_config_url() == setup.get_config_url());
            CATCH_REQUIRE(file1->get_errno() == 0);
            CATCH_REQUIRE(file1->get_sections().empty());
            CATCH_REQUIRE(file1->get_parameters().size() == 3);

            CATCH_REQUIRE(file1->has_parameter("param"));
            CATCH_REQUIRE(file1->has_parameter("changing"));
            CATCH_REQUIRE(file1->has_parameter("test"));

            CATCH_REQUIRE(file1->get_parameter("param") == "value");
            CATCH_REQUIRE(file1->get_parameter("changing") == "without reloading is useless");
            CATCH_REQUIRE(file1->get_parameter("test") == "1009");
        }

        // change all the values now
        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "param=new data\n"
                "new=this is not even acknowledge\n"
                "changing=special value\n"
                "test=9010\n"
                "level=three\n"
            ;
        }

        // "reloading" that very same file has the old data
        {
            advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                                , advgetopt::line_continuation_t::line_continuation_single_line
                                , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                                , advgetopt::COMMENT_SHELL
                                , advgetopt::SECTION_OPERATOR_NONE);

            CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

            CATCH_REQUIRE(setup.is_valid());
            CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
            CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
            CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
            CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

            advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

            // exact same pointer
            //
            CATCH_REQUIRE(file == file1);

            CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
            CATCH_REQUIRE(file->get_errno() == 0);
            CATCH_REQUIRE(file->get_sections().empty());
            CATCH_REQUIRE(file->get_parameters().size() == 3);

            CATCH_REQUIRE(file->has_parameter("param"));
            CATCH_REQUIRE(file->has_parameter("changing"));
            CATCH_REQUIRE(file->has_parameter("test"));

            CATCH_REQUIRE(file->get_parameter("param") == "value");
            CATCH_REQUIRE(file->get_parameter("changing") == "without reloading is useless");
            CATCH_REQUIRE(file->get_parameter("test") == "1009");
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("config_duplicated_variables", "[config][getopt][valid]")
{
    CATCH_START_SECTION("file with the same variable defined multiple times")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("duplicated-variable", "multiple");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "unique    = perfect  \n"
                "multiple  = defintions\n"
                "another   = just fine \t\n"
                "multiple  = value\r\n"
                "good      = variable \n"
                "multiple  = set\n"
                "more      = data\t \n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "warning: parameter \"multiple\" on line 5 in"
                      " configuration file \""
                    + std::string(fn.get())
                    + "\" was found twice in the same configuration file.");
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "warning: parameter \"multiple\" on line 7 in"
                      " configuration file \""
                    + std::string(fn.get())
                    + "\" was found twice in the same configuration file.");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 5);

        CATCH_REQUIRE(file->has_parameter("unique"));
        CATCH_REQUIRE(file->has_parameter("another"));
        CATCH_REQUIRE(file->has_parameter("good"));
        CATCH_REQUIRE(file->has_parameter("more"));
        CATCH_REQUIRE(file->has_parameter("multiple"));

        CATCH_REQUIRE(file->get_parameter("unique") == "perfect");
        CATCH_REQUIRE(file->get_parameter("another") == "just fine");
        CATCH_REQUIRE(file->get_parameter("good") == "variable");
        CATCH_REQUIRE(file->get_parameter("more") == "data");
        CATCH_REQUIRE(file->get_parameter("multiple") == "set");

        // we get a warning while reading; but not when directly
        // accessing the file object
        //
        CATCH_REQUIRE(file->set_parameter(std::string(), "multiple", "new value"));
        CATCH_REQUIRE(file->get_parameter("multiple") == "new value");
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("config_callback_calls", "[config][getopt][valid]")
{
    CATCH_START_SECTION("setup a callback and test the set_parameter()/erase() functions")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("callback-variable", "callback");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "unique     = perfect  \n"
                "definition = long value here\n"
                "another    = just fine \t\n"
                "multiple   = value\r\n"
                "good       = variable \n"
                "organized  = set\n"
                "more       = data\t \n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        struct conf_data
        {
            advgetopt::conf_file::pointer_t f_conf_file = advgetopt::conf_file::pointer_t();
            advgetopt::callback_action_t    f_expected_action = advgetopt::callback_action_t::created;
            std::string                     f_expected_variable = std::string();
            std::string                     f_expected_value = std::string();
        };
        conf_data cf_data;
        cf_data.f_conf_file = file;

        struct conf_callback
        {
            void operator () (advgetopt::conf_file::pointer_t conf_file
                            , advgetopt::callback_action_t action
                            , std::string const & variable_name
                            , std::string const & value)
            {
                CATCH_REQUIRE(conf_file == f_data->f_conf_file);
                CATCH_REQUIRE(action == f_data->f_expected_action);
                CATCH_REQUIRE(variable_name == f_data->f_expected_variable);
                CATCH_REQUIRE(value == f_data->f_expected_value);
                CATCH_REQUIRE(conf_file->get_parameter(variable_name) == f_data->f_expected_value);
            }

            conf_data * f_data = nullptr;
        };
        conf_callback cf;
        cf.f_data = &cf_data;

        advgetopt::conf_file::callback_id_t callback_id(file->add_callback(cf));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE_FALSE(file->was_modified());
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 7);

        CATCH_REQUIRE(file->has_parameter("unique"));
        CATCH_REQUIRE(file->has_parameter("definition"));
        CATCH_REQUIRE(file->has_parameter("another"));
        CATCH_REQUIRE(file->has_parameter("multiple"));
        CATCH_REQUIRE(file->has_parameter("good"));
        CATCH_REQUIRE(file->has_parameter("organized"));
        CATCH_REQUIRE(file->has_parameter("more"));

        CATCH_REQUIRE(file->get_parameter("unique") == "perfect");
        CATCH_REQUIRE(file->get_parameter("definition") == "long value here");
        CATCH_REQUIRE(file->get_parameter("another") == "just fine");
        CATCH_REQUIRE(file->get_parameter("multiple") == "value");
        CATCH_REQUIRE(file->get_parameter("good") == "variable");
        CATCH_REQUIRE(file->get_parameter("organized") == "set");
        CATCH_REQUIRE(file->get_parameter("more") == "data");

        // updated action
        //
        cf_data.f_expected_action = advgetopt::callback_action_t::updated;
        cf_data.f_expected_variable = "multiple";
        cf_data.f_expected_value = "new value";
        CATCH_REQUIRE(file->set_parameter(std::string(), "multiple", "new value"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 7);
        CATCH_REQUIRE(file->get_parameter("multiple") == "new value");

        // created action
        //
        cf_data.f_expected_action = advgetopt::callback_action_t::created;
        cf_data.f_expected_variable = "new-param";
        cf_data.f_expected_value = "with this value";
        CATCH_REQUIRE(file->set_parameter(std::string(), "new_param", "with this value"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 8);
        CATCH_REQUIRE(file->has_parameter("new-param"));
        CATCH_REQUIRE(file->get_parameter("new-param") == "with this value");
        CATCH_REQUIRE(file->has_parameter("new_param"));
        CATCH_REQUIRE(file->get_parameter("new_param") == "with this value");

        // updated action when modifying
        //
        cf_data.f_expected_action = advgetopt::callback_action_t::updated;
        cf_data.f_expected_variable = "new-param";
        cf_data.f_expected_value = "change completely";
        CATCH_REQUIRE(file->set_parameter(std::string(), "new_param", "change completely"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 8);
        CATCH_REQUIRE(file->has_parameter("new-param"));
        CATCH_REQUIRE(file->get_parameter("new-param") == "change completely");
        CATCH_REQUIRE(file->has_parameter("new_param"));
        CATCH_REQUIRE(file->get_parameter("new_param") == "change completely");

        // erased action
        //
        cf_data.f_expected_action = advgetopt::callback_action_t::erased;
        cf_data.f_expected_variable = "new-param";
        cf_data.f_expected_value = std::string();
        CATCH_REQUIRE(file->erase_parameter("new_param"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 7);
        CATCH_REQUIRE_FALSE(file->has_parameter("new-param"));
        CATCH_REQUIRE(file->get_parameter("new-param") == std::string());
        CATCH_REQUIRE_FALSE(file->has_parameter("new_param"));
        CATCH_REQUIRE(file->get_parameter("new_param") == std::string());
        CATCH_REQUIRE_FALSE(file->erase_parameter("new_param"));

        // created action again (because it was erased)
        //
        cf_data.f_expected_action = advgetopt::callback_action_t::created;
        cf_data.f_expected_variable = "new-param";
        cf_data.f_expected_value = "with this value";
        CATCH_REQUIRE(file->set_parameter(std::string(), "new_param", "with this value"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 8);
        CATCH_REQUIRE(file->has_parameter("new-param"));
        CATCH_REQUIRE(file->get_parameter("new-param") == "with this value");
        CATCH_REQUIRE(file->has_parameter("new_param"));
        CATCH_REQUIRE(file->get_parameter("new_param") == "with this value");

        file->remove_callback(callback_id);
        cf_data.f_expected_action = advgetopt::callback_action_t::created;
        cf_data.f_expected_variable = "ignored";
        cf_data.f_expected_value = "ignored";
        CATCH_REQUIRE(file->set_parameter(std::string(), "new_param", "unnoticed change"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 8);
        CATCH_REQUIRE(file->has_parameter("new-param"));
        CATCH_REQUIRE(file->get_parameter("new-param") == "unnoticed change");
        CATCH_REQUIRE(file->has_parameter("new_param"));
        CATCH_REQUIRE(file->get_parameter("new_param") == "unnoticed change");

        // further calls do nothing more
        //
        file->remove_callback(callback_id);
        CATCH_REQUIRE(file->set_parameter(std::string(), "new_param", "still unnoticed"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 8);
        CATCH_REQUIRE(file->has_parameter("new-param"));
        CATCH_REQUIRE(file->get_parameter("new-param") == "still unnoticed");
        CATCH_REQUIRE(file->has_parameter("new_param"));
        CATCH_REQUIRE(file->get_parameter("new_param") == "still unnoticed");

        // and we can always re-add it
        //
        CATCH_REQUIRE(callback_id != file->add_callback(cf));
        cf_data.f_expected_action = advgetopt::callback_action_t::updated;
        cf_data.f_expected_variable = "new-param";
        cf_data.f_expected_value = "we're back";
        CATCH_REQUIRE(file->set_parameter(std::string(), "new_param", "we're back"));
        CATCH_REQUIRE(file->was_modified());
        CATCH_REQUIRE(file->get_parameters().size() == 8);
        CATCH_REQUIRE(file->has_parameter("new-param"));
        CATCH_REQUIRE(file->get_parameter("new-param") == "we're back");
        CATCH_REQUIRE(file->has_parameter("new_param"));
        CATCH_REQUIRE(file->get_parameter("new_param") == "we're back");

        // until you save it remains true even if you were to restore the
        // state to "normal" (we do not keep a copy of the original value
        // as found in the file.)
        //
        CATCH_REQUIRE(file->was_modified());
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("config_line_continuation_tests", "[config][getopt][valid]")
{
    CATCH_START_SECTION("single_line")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("line-continuation", "single-line");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("rfc822")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("line-continuation", "rfc822");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_rfc_822
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_rfc_822);
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("msdos")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("line-continuation", "msdos");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_msdos
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_msdos);
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("unix")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("line-continuation", "unix");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("fortran")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("line-continuation", "fortran");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_fortran
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_fortran);
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("semicolon")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("line-continuation", "semicolon");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_semicolon
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_semicolon);
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
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("config_assignment_operator_tests", "[config][getopt][valid]")
{
    CATCH_START_SECTION("equal")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("assignment-operator", "equal");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "equal=value\n"
                "\n"
                "name_value=127\n"
                "\n"
                "and=no operator\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("equal"));
        CATCH_REQUIRE(file->has_parameter("name-value"));
        CATCH_REQUIRE(file->has_parameter("and"));

        CATCH_REQUIRE(file->get_parameter("equal") == "value");
        CATCH_REQUIRE(file->get_parameter("name-value") == "127");
        CATCH_REQUIRE(file->get_parameter("and") == "no operator");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("colon")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("assignment-operator", "colon");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "equal_value\n"
                "\n"
                "name:value=127\n"
                "\n"
                "and_no-operator\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_COLON
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_COLON);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("equal-value"));
        CATCH_REQUIRE(file->has_parameter("name"));
        CATCH_REQUIRE(file->has_parameter("and-no-operator"));

        CATCH_REQUIRE(file->get_parameter("equal-value") == std::string());
        CATCH_REQUIRE(file->get_parameter("name") == "value=127");
        CATCH_REQUIRE(file->get_parameter("and-no-operator") == std::string());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("space")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("assignment-operator", "space");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "equal-value\n"
                "\n"
                "name 127\n"
                "\n"
                "and-no operator\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_SPACE
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_SPACE);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("equal-value"));
        CATCH_REQUIRE(file->has_parameter("name"));
        CATCH_REQUIRE(file->has_parameter("and-no"));

        CATCH_REQUIRE(file->get_parameter("equal-value") == std::string());
        CATCH_REQUIRE(file->get_parameter("name") == "127");
        CATCH_REQUIRE(file->get_parameter("and-no") == "operator");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("equal_colon_and_space")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("assignment-operator", "all");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            | advgetopt::ASSIGNMENT_OPERATOR_COLON
                            | advgetopt::ASSIGNMENT_OPERATOR_SPACE
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
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
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("config_comment_tests", "[config][getopt][valid]")
{
    CATCH_START_SECTION("ini comment")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("comment", "ini");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "; Auto-generated\n"
                "ini=comment\n"
                ";ignore=this one\n"
                "is=the semi-colon\n"
                ";continuation=with Unix\\\n"
                "also=works for\\\n"
                "comments\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_INI
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_INI);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("ini"));
        CATCH_REQUIRE(file->has_parameter("is"));

        CATCH_REQUIRE(file->get_parameter("ini") == "comment");
        CATCH_REQUIRE(file->get_parameter("is") == "the semi-colon");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("shell comment")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("comment", "shell");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "shell=comment\n"
                "#ignore=this one\n"
                "is=the hash (`#`) character\n"
                "#continuation=with Unix\\\n"
                "also=works for\\\n"
                "comments\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("shell"));
        CATCH_REQUIRE(file->has_parameter("is"));

        CATCH_REQUIRE(file->get_parameter("shell") == "comment");
        CATCH_REQUIRE(file->get_parameter("is") == "the hash (`#`) character");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("C++ comment")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("comment", "cpp");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "// Auto-generated\n"
                "cpp=comment\n"
                "//ignore=this one\n"
                "is=the double slash (`//`)\n"
                "//continuation=with Unix\\\n"
                "also=works for\\\n"
                "comments\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_CPP
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_CPP);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("cpp"));
        CATCH_REQUIRE(file->has_parameter("is"));

        CATCH_REQUIRE(file->get_parameter("cpp") == "comment");
        CATCH_REQUIRE(file->get_parameter("is") == "the double slash (`//`)");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("All three comments")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("comment", "all-comments");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_INI | advgetopt::COMMENT_SHELL | advgetopt::COMMENT_CPP
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
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
    }
    CATCH_END_SECTION()
}





CATCH_TEST_CASE("config_section_tests", "[config][getopt][valid]")
{
    CATCH_START_SECTION("section operator c (.)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("section-operator", "section-c");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a.b=red\n"
                "a.b.c=122\n"
                "m=size\n"
                "z=edge\n"
                "z.b=line\n"
                "z.b.c=12.72\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_C);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 4);
        CATCH_REQUIRE(sections.find("a")    != sections.end());
        CATCH_REQUIRE(sections.find("a::b") != sections.end());
        CATCH_REQUIRE(sections.find("z")    != sections.end());
        CATCH_REQUIRE(sections.find("z::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 7);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("a::b::c"));
        CATCH_REQUIRE(file->has_parameter("m"));
        CATCH_REQUIRE(file->has_parameter("z"));
        CATCH_REQUIRE(file->has_parameter("z::b"));
        CATCH_REQUIRE(file->has_parameter("z::b::c"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("a::b::c") == "122");
        CATCH_REQUIRE(file->get_parameter("m") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
        CATCH_REQUIRE(file->get_parameter("z::b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::b::c") == "12.72");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("section operator c++ (::)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("section-operator", "section-cpp");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a::b=red\n"
                "a::b::c=122\n"
                "m=size\n"
                "z=edge\n"
                "z::b=line\n"
                "z::b::c=12.72\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_CPP);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 4);
        CATCH_REQUIRE(sections.find("a")    != sections.end());
        CATCH_REQUIRE(sections.find("a::b") != sections.end());
        CATCH_REQUIRE(sections.find("z")    != sections.end());
        CATCH_REQUIRE(sections.find("z::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 7);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("a::b::c"));
        CATCH_REQUIRE(file->has_parameter("m"));
        CATCH_REQUIRE(file->has_parameter("z"));
        CATCH_REQUIRE(file->has_parameter("z::b"));
        CATCH_REQUIRE(file->has_parameter("z::b::c"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("a::b::c") == "122");
        CATCH_REQUIRE(file->get_parameter("m") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
        CATCH_REQUIRE(file->get_parameter("z::b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::b::c") == "12.72");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("section operator block ({ ... })")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("section-operator", "section-block");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a {\n"
                "  b=red\n"
                "  b {\n"
                "    c=122\n"
                "  }\n"
                "}\n"
                "m=size\n"
                "z=edge\n"
                "z {\n"
                "  b {\n"
                "    c=12.72\n"
                "  }\n"
                "  b=line\n"
                "}\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_BLOCK);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_BLOCK);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 4);
        CATCH_REQUIRE(sections.find("a")    != sections.end());
        CATCH_REQUIRE(sections.find("a::b") != sections.end());
        CATCH_REQUIRE(sections.find("z")    != sections.end());
        CATCH_REQUIRE(sections.find("z::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 7);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("a::b::c"));
        CATCH_REQUIRE(file->has_parameter("m"));
        CATCH_REQUIRE(file->has_parameter("z"));
        CATCH_REQUIRE(file->has_parameter("z::b"));
        CATCH_REQUIRE(file->has_parameter("z::b::c"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("a::b::c") == "122");
        CATCH_REQUIRE(file->get_parameter("m") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
        CATCH_REQUIRE(file->get_parameter("z::b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::b::c") == "12.72");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("section operator ini file ([...])")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("section-operator", "section-ini-file");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "[a]\n"
                "b=red\n"
                "b-c=122\n"
                "[]\n"
                "m=size\n"
                "z=edge\n"
                "[z] # we allow comments here\n"
                "b=line\n"
                "b-c=12.72\n"
                "[p]#nospacenecessary\n"
                "b=comment\n"
                "b-c=allowed\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_INI_FILE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_INI_FILE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 3);
        CATCH_REQUIRE(sections.find("a") != sections.end());
        CATCH_REQUIRE(sections.find("z") != sections.end());
        CATCH_REQUIRE(sections.find("p") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 9);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("a::b-c"));
        CATCH_REQUIRE(file->has_parameter("m"));
        CATCH_REQUIRE(file->has_parameter("z"));
        CATCH_REQUIRE(file->has_parameter("z::b"));
        CATCH_REQUIRE(file->has_parameter("z::b-c"));
        CATCH_REQUIRE(file->has_parameter("p::b"));
        CATCH_REQUIRE(file->has_parameter("p::b-c"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("a::b-c") == "122");
        CATCH_REQUIRE(file->get_parameter("m") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
        CATCH_REQUIRE(file->get_parameter("z::b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::b-c") == "12.72");
        CATCH_REQUIRE(file->get_parameter("p::b") == "comment");
        CATCH_REQUIRE(file->get_parameter("p::b-c") == "allowed");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("section operator ini-file & c++")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("section-operator", "section-double");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "[a]\n"
                "b=red\n"
                "b::c=209\n"
                "::h=high\n"
                "m=size\n"
                "[z]\n"
                "z=edge\n"
                "::b=line\n"
                "z::b::c=17.92\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_INI_FILE | advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == (advgetopt::SECTION_OPERATOR_INI_FILE | advgetopt::SECTION_OPERATOR_CPP));

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 4);
        CATCH_REQUIRE(sections.find("a")       != sections.end());
        CATCH_REQUIRE(sections.find("a::b")    != sections.end());
        CATCH_REQUIRE(sections.find("z")       != sections.end());
        CATCH_REQUIRE(sections.find("z::z::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 7);

        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("a::b::c"));
        CATCH_REQUIRE(file->has_parameter("h"));
        CATCH_REQUIRE(file->has_parameter("a::m"));
        CATCH_REQUIRE(file->has_parameter("z::z"));
        CATCH_REQUIRE(file->has_parameter("b"));
        CATCH_REQUIRE(file->has_parameter("z::z::b::c"));

        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("a::b::c") == "209");
        CATCH_REQUIRE(file->get_parameter("h") == "high");
        CATCH_REQUIRE(file->get_parameter("a::m") == "size");
        CATCH_REQUIRE(file->get_parameter("z::z") == "edge");
        CATCH_REQUIRE(file->get_parameter("b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::z::b::c") == "17.92");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("section of variables ([variables])")
    {
        // in a config file variables are not auto-managed
        //
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("section-variables", "section-with-variables");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "edge=${blue}\n"
                "background=${white}\n"
                "foreground=${black}\n"
                "[error]\n"
                "edge=${red}\n"
                "background=${gray}\n"
                "[variables]\n"
                "red=\"#ff0000\"\n"
                "green=\"#00ff00\"\n"
                "blue=\"#0000ff\"\n"
                "no_color=\"#000000\"\n"
                "black=${no_color}\n"
                "white=\"#ffffff\"\n"
                "gray=\"#aaaaaa\"\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_INI_FILE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_INI_FILE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 2);
        CATCH_REQUIRE(sections.find("error") != sections.end());
        CATCH_REQUIRE(sections.find("variables") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 12);

        CATCH_REQUIRE(file->has_parameter("edge"));
        CATCH_REQUIRE(file->has_parameter("background"));
        CATCH_REQUIRE(file->has_parameter("foreground"));
        CATCH_REQUIRE(file->has_parameter("error::edge"));
        CATCH_REQUIRE(file->has_parameter("error::background"));
        CATCH_REQUIRE(file->has_parameter("variables::red"));
        CATCH_REQUIRE(file->has_parameter("variables::green"));
        CATCH_REQUIRE(file->has_parameter("variables::blue"));
        CATCH_REQUIRE(file->has_parameter("variables::no_color"));
        CATCH_REQUIRE(file->has_parameter("variables::black"));
        CATCH_REQUIRE(file->has_parameter("variables::white"));
        CATCH_REQUIRE(file->has_parameter("variables::gray"));

        // without a variables attached, we get the raw (original) data back
        //
        CATCH_REQUIRE(file->get_parameter("edge") == "${blue}");
        CATCH_REQUIRE(file->get_parameter("background") == "${white}");
        CATCH_REQUIRE(file->get_parameter("foreground") == "${black}");
        CATCH_REQUIRE(file->get_parameter("error::edge") == "${red}");
        CATCH_REQUIRE(file->get_parameter("error::background") == "${gray}");
        CATCH_REQUIRE(file->get_parameter("variables::red") == "#ff0000");
        CATCH_REQUIRE(file->get_parameter("variables::green") == "#00ff00");
        CATCH_REQUIRE(file->get_parameter("variables::blue") == "#0000ff");
        CATCH_REQUIRE(file->get_parameter("variables::no_color") == "#000000");
        CATCH_REQUIRE(file->get_parameter("variables::black") == "${no_color}");
        CATCH_REQUIRE(file->get_parameter("variables::white") == "#ffffff");
        CATCH_REQUIRE(file->get_parameter("variables::gray") == "#aaaaaa");

        // transform the "[variables]" section to variables
        //
        advgetopt::variables::pointer_t vars(std::make_shared<advgetopt::variables>());
        CATCH_REQUIRE(file->section_to_variables("variables", vars) == 7);
        file->set_variables(vars);
        CATCH_REQUIRE(file->get_variables() == vars);

        sections = file->get_sections();
        CATCH_REQUIRE(sections.size() == 1);
        CATCH_REQUIRE(sections.find("error") != sections.end());
        CATCH_REQUIRE(sections.find("variables") == sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 5);

        CATCH_REQUIRE(file->has_parameter("edge"));
        CATCH_REQUIRE(file->has_parameter("background"));
        CATCH_REQUIRE(file->has_parameter("foreground"));
        CATCH_REQUIRE(file->has_parameter("error::edge"));
        CATCH_REQUIRE(file->has_parameter("error::background"));
        CATCH_REQUIRE_FALSE(file->has_parameter("variables::red"));
        CATCH_REQUIRE_FALSE(file->has_parameter("variables::green"));
        CATCH_REQUIRE_FALSE(file->has_parameter("variables::blue"));
        CATCH_REQUIRE_FALSE(file->has_parameter("variables::no_color"));
        CATCH_REQUIRE_FALSE(file->has_parameter("variables::black"));
        CATCH_REQUIRE_FALSE(file->has_parameter("variables::white"));
        CATCH_REQUIRE_FALSE(file->has_parameter("variables::gray"));

        CATCH_REQUIRE(file->get_parameter("edge") == "#0000ff");
        CATCH_REQUIRE(file->get_parameter("background") == "#ffffff");
        CATCH_REQUIRE(file->get_parameter("foreground") == "#000000");
        CATCH_REQUIRE(file->get_parameter("error::edge") == "#ff0000");
        CATCH_REQUIRE(file->get_parameter("error::background") == "#aaaaaa");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("command line with .conf including section of variables ([variables])")
    {
        // in a config file variables are not auto-managed
        //
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("command-line-and-section-variables", "command-section-with-variables");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "edge=${blue}\n"
                "background=${white}\n"
                "foreground=${black}\n"
                "[error]\n"
                "edge=${red}\n"
                "background=${gray}\n"
                "[variables]\n"
                "red=\"#ff0000\"\n"
                "green=\"#00ff00\"\n"
                "blue=\"#0000ff\"\n"
                "no_color=\"#000000\"\n"
                "black=${no_color}\n"
                "orange=\"#80ff00\"\n"
                "white=\"#ffffff\"\n"
                "gray=\"#aaaaaa\"\n"
            ;
        }

        const advgetopt::option options[] =
        {
            advgetopt::define_option(
                  advgetopt::Name("edge")
                , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_PROCESS_VARIABLES>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("background")
                , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_PROCESS_VARIABLES>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("foreground")
                , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_PROCESS_VARIABLES>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("error::edge")
                , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_PROCESS_VARIABLES>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("error::background")
                , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_PROCESS_VARIABLES>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("error::foreground")
                , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_PROCESS_VARIABLES>())
            ),
            advgetopt::define_option(
                  advgetopt::Name("see-config")
                , advgetopt::Flags(advgetopt::standalone_command_flags<>())
            ),
            advgetopt::end_options()
        };

        char const * const configuration_files[] =
        {
            SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(),
            nullptr
        };

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "unittest";
        environment_options.f_options = options;
        environment_options.f_help_header = "Usage: configuration with variables through environment.";
        environment_options.f_section_variables_name = "variables";
        environment_options.f_configuration_files = configuration_files;

        char const * cargv[] =
        {
            "/usr/bin/cmd-n-config",
            "--see-config",
            "--error::foreground",
            "${orange}",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::getopt::pointer_t opts(std::make_shared<advgetopt::getopt>(environment_options, argc, argv));
        CATCH_REQUIRE(opts != nullptr);

        advgetopt::variables::pointer_t variables(opts->get_variables());
        CATCH_REQUIRE(variables != nullptr);

        CATCH_REQUIRE(opts->is_defined("see-config"));
        CATCH_REQUIRE(opts->is_defined("edge"));
        CATCH_REQUIRE(opts->is_defined("background"));
        CATCH_REQUIRE(opts->is_defined("foreground"));
        CATCH_REQUIRE(opts->is_defined("error::edge"));
        CATCH_REQUIRE(opts->is_defined("error::background"));
        CATCH_REQUIRE(opts->is_defined("error::foreground"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::red"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::green"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::blue"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::no_color"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::black"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::orange"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::white"));
        CATCH_REQUIRE_FALSE(opts->is_defined("variables::gray"));

        CATCH_REQUIRE(opts->get_string("edge") == "#0000ff");
        CATCH_REQUIRE(opts->get_string("background") == "#ffffff");
        CATCH_REQUIRE(opts->get_string("foreground") == "#000000");
        CATCH_REQUIRE(opts->get_string("error::edge") == "#ff0000");
        CATCH_REQUIRE(opts->get_string("error::background") == "#aaaaaa");
        CATCH_REQUIRE(opts->get_string("error::foreground") == "#80ff00");

        CATCH_REQUIRE(opts->get_option("edge")->get_variables() == variables);
        CATCH_REQUIRE(opts->get_option("background")->get_variables() == variables);
        CATCH_REQUIRE(opts->get_option("foreground")->get_variables() == variables);
        CATCH_REQUIRE(opts->get_option("error::edge")->get_variables() == variables);
        CATCH_REQUIRE(opts->get_option("error::background")->get_variables() == variables);
        CATCH_REQUIRE(opts->get_option("error::foreground")->get_variables() == variables);
    }
    CATCH_END_SECTION()
}




CATCH_TEST_CASE("save_config_file", "[config][getopt][valid]")
{
    CATCH_START_SECTION("load update save (=)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("save-operation", "configuration-equal");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "b=red\n"
                "call-flag=122\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->exists());
        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.empty());

        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("b"));
        CATCH_REQUIRE(file->has_parameter("call-flag"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("b") == "red");
        CATCH_REQUIRE(file->get_parameter("call-flag") == "122");

        CATCH_REQUIRE(file->save_configuration());

        // no backup since there was no modification so the save did nothing
        //
        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".bak").c_str(), F_OK) != 0);

        file->set_parameter(std::string(), "a", "size");
        file->set_parameter(std::string(), "b", "tall");
        file->set_parameter(std::string(), "call-flag", "1920");

        CATCH_REQUIRE(file->save_configuration());

        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".bak").c_str(), F_OK) == 0);

        file->set_parameter(std::string(), "a", "pace");
        file->set_parameter(std::string(), "b", "fall");
        file->set_parameter(std::string(), "call-flag", "2019");

        CATCH_REQUIRE(file->save_configuration("save"));

        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".save").c_str(), F_OK) == 0);

        std::string const new_name(SNAP_CATCH2_NAMESPACE::g_config_filename + ".conf2");
        rename(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), new_name.c_str());

        advgetopt::conf_file_setup setup2(new_name
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup2.is_valid());
        CATCH_REQUIRE(setup2.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup2.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup2.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup2.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file2(advgetopt::conf_file::get_conf_file(setup2));

        CATCH_REQUIRE(file2->exists());
        CATCH_REQUIRE(file2->get_setup().get_config_url() == setup2.get_config_url());
        CATCH_REQUIRE(file2->get_errno() == 0);

        CATCH_REQUIRE(file->get_sections().empty());

        CATCH_REQUIRE(file2->get_parameters().size() == 3);

        CATCH_REQUIRE(file2->has_parameter("a"));
        CATCH_REQUIRE(file2->has_parameter("b"));
        CATCH_REQUIRE(file2->has_parameter("call-flag"));

        CATCH_REQUIRE(file2->get_parameter("a") == "pace");
        CATCH_REQUIRE(file2->get_parameter("b") == "fall");
        CATCH_REQUIRE(file2->get_parameter("call-flag") == "2019");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load update save (:)")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("save-operation", "configuration-colon");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a: color\n"
                "b: red\n"
                "call-flag: 122\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_COLON
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE
                            , advgetopt::NAME_SEPARATOR_DASHES);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_COLON);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->exists());
        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.empty());

        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("b"));
        CATCH_REQUIRE(file->has_parameter("call-flag"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("b") == "red");
        CATCH_REQUIRE(file->get_parameter("call-flag") == "122");

        CATCH_REQUIRE(file->save_configuration());

        // no backup since there was no modification so the save did nothing
        //
        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".bak").c_str(), F_OK) != 0);

        file->set_parameter(std::string(), "a", "size");
        file->set_parameter(std::string(), "b", "tall");
        file->set_parameter(std::string(), "call-flag", "1920");

        CATCH_REQUIRE(file->save_configuration());

        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".bak").c_str(), F_OK) == 0);

        file->set_parameter(std::string(), "a", "pace");
        file->set_parameter(std::string(), "b", "fall");
        file->set_parameter(std::string(), "call-flag", "2019");

        CATCH_REQUIRE(file->save_configuration("save"));

        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".save").c_str(), F_OK) == 0);

        std::string const new_name(SNAP_CATCH2_NAMESPACE::g_config_filename + ".conf2");
        rename(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), new_name.c_str());

        advgetopt::conf_file_setup setup2(new_name
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_COLON
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE
                            , advgetopt::NAME_SEPARATOR_DASHES);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup2.is_valid());
        CATCH_REQUIRE(setup2.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup2.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_COLON);
        CATCH_REQUIRE(setup2.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup2.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file2(advgetopt::conf_file::get_conf_file(setup2));

        CATCH_REQUIRE(file2->exists());
        CATCH_REQUIRE(file2->get_setup().get_config_url() == setup2.get_config_url());
        CATCH_REQUIRE(file2->get_errno() == 0);

        CATCH_REQUIRE(file->get_sections().empty());

        CATCH_REQUIRE(file2->get_parameters().size() == 3);

        CATCH_REQUIRE(file2->has_parameter("a"));
        CATCH_REQUIRE(file2->has_parameter("b"));
        CATCH_REQUIRE(file2->has_parameter("call-flag"));

        CATCH_REQUIRE(file2->get_parameter("a") == "pace");
        CATCH_REQUIRE(file2->get_parameter("b") == "fall");
        CATCH_REQUIRE(file2->get_parameter("call-flag") == "2019");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("load update save ( )")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("save-operation", "configuration-space");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# This comment is kept along the a= variable\n"
                "a color\n"
                "b red\n"
                "call-flag 122\n"
            ;
        }

        advgetopt::comment_t const comment(advgetopt::COMMENT_SHELL | advgetopt::COMMENT_SAVE);
        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_SPACE
                            , comment
                            , advgetopt::SECTION_OPERATOR_NONE
                            , advgetopt::NAME_SEPARATOR_DASHES);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_SPACE);
        CATCH_REQUIRE(setup.get_comment() == comment);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->exists());
        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.empty());

        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("b"));
        CATCH_REQUIRE(file->has_parameter("call-flag"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("b") == "red");
        CATCH_REQUIRE(file->get_parameter("call-flag") == "122");

        advgetopt::conf_file::parameters_t params(file->get_parameters());
        auto it(params.find("a"));
        CATCH_REQUIRE(it != params.end());
        CATCH_REQUIRE(it->second.get_comment() == "# This comment is kept along the a= variable\n");
        CATCH_REQUIRE(it->second.get_comment(true) == "# This comment is kept along the a= variable\n");

        it->second.set_comment("# Changing the comment");
        CATCH_REQUIRE(it->second.get_comment() == "# Changing the comment");
        CATCH_REQUIRE(it->second.get_comment(true) == "# Changing the comment\n");

        CATCH_REQUIRE(file->save_configuration());

        // no backup since there was no modification so the save did nothing
        //
        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".bak").c_str(), F_OK) != 0);

        file->set_parameter(std::string(), "a", "size");
        file->set_parameter(std::string(), "b", "tall");
        file->set_parameter(std::string(), "call-flag", "1920");

        CATCH_REQUIRE(file->save_configuration());

        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".bak").c_str(), F_OK) == 0);

        file->set_parameter(std::string(), "a", "pace");
        file->set_parameter(std::string(), "b", "fall");
        file->set_parameter(std::string(), "call-flag", "2019");

        it->second.set_value("warning"); // WARNING: the parameters_t is a copy so you can't change the parameters that way
        CATCH_REQUIRE(it->second.get_value() == "warning");
        CATCH_REQUIRE(file->get_parameter("a") == "pace");

        // the following constructor and assignment are defined although not
        // used within the library at the moment
        //
        advgetopt::parameter_value const value("other value");
        it->second = value;
        CATCH_REQUIRE(it->second.get_value() == "other value");
        CATCH_REQUIRE(file->get_parameter("a") == "pace");

        CATCH_REQUIRE(file->save_configuration("save"));

        CATCH_REQUIRE(access((SNAP_CATCH2_NAMESPACE::g_config_filename + ".save").c_str(), F_OK) == 0);

        std::string const new_name(SNAP_CATCH2_NAMESPACE::g_config_filename + ".conf2");
        rename(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), new_name.c_str());

        advgetopt::conf_file_setup setup2(new_name
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_SPACE
                            , comment
                            , advgetopt::SECTION_OPERATOR_NONE
                            , advgetopt::NAME_SEPARATOR_DASHES);

        CATCH_REQUIRE(setup.get_original_filename() == SNAP_CATCH2_NAMESPACE::g_config_filename);

        CATCH_REQUIRE(setup2.is_valid());
        CATCH_REQUIRE(setup2.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup2.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_SPACE);
        CATCH_REQUIRE(setup2.get_comment() == comment);
        CATCH_REQUIRE(setup2.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file2(advgetopt::conf_file::get_conf_file(setup2));

        CATCH_REQUIRE(file2->exists());
        CATCH_REQUIRE(file2->get_setup().get_config_url() == setup2.get_config_url());
        CATCH_REQUIRE(file2->get_errno() == 0);

        CATCH_REQUIRE(file->get_sections().empty());

        CATCH_REQUIRE(file2->get_parameters().size() == 3);

        CATCH_REQUIRE(file2->has_parameter("a"));
        CATCH_REQUIRE(file2->has_parameter("b"));
        CATCH_REQUIRE(file2->has_parameter("call-flag"));

        CATCH_REQUIRE(file2->get_parameter("a") == "pace");
        CATCH_REQUIRE(file2->get_parameter("b") == "fall");
        CATCH_REQUIRE(file2->get_parameter("call-flag") == "2019");

        file2->erase_all_parameters();
        CATCH_REQUIRE_FALSE(file2->has_parameter("a"));
        CATCH_REQUIRE_FALSE(file2->has_parameter("b"));
        CATCH_REQUIRE_FALSE(file2->has_parameter("call-flag"));
    }
    CATCH_END_SECTION()
}







CATCH_TEST_CASE("invalid_configuration_setup", "[config][getopt][invalid]")
{
    CATCH_START_SECTION("Empty Filename")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
              advgetopt::conf_file_setup(
                              std::string()
                            , static_cast<advgetopt::line_continuation_t>(rand())
                            , rand()
                            , rand()
                            , rand())
            , advgetopt::getopt_invalid
            , Catch::Matchers::ExceptionMessage(
                          "getopt_exception: trying to load a configuration file using an empty filename."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Invalid Line Continuation")
    {
        for(int count(0); count < 5; ++count)
        {
            advgetopt::line_continuation_t lc(advgetopt::line_continuation_t::line_continuation_unix);
            do
            {
                lc = static_cast<advgetopt::line_continuation_t>(rand());
            }
            while(lc >= advgetopt::line_continuation_t::line_continuation_single_line
               && lc <= advgetopt::line_continuation_t::line_continuation_semicolon);

            advgetopt::conf_file_setup setup(
                          "/etc/advgetopt/system.conf"
                        , lc        // <- this is invalid
                        , rand() & advgetopt::ASSIGNMENT_OPERATOR_MASK
                        , rand() & advgetopt::COMMENT_MASK
                        , rand() & advgetopt::SECTION_OPERATOR_MASK);

            CATCH_REQUIRE(setup.is_valid());

            CATCH_REQUIRE_THROWS_MATCHES(
                  setup.get_config_url()
                , advgetopt::getopt_logic_error
                , Catch::Matchers::ExceptionMessage(
                              "getopt_logic_error: unexpected line continuation."));
        }
    }
    CATCH_END_SECTION()
}







CATCH_TEST_CASE("config_reload_invalid_setup", "[config][getopt][invalid]")
{
    CATCH_START_SECTION("Load a file, update it, verify it does not get reloaded")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-reload", "load-twice-wrong-parameters");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "duplicates=work\n"
                "varying=parameters\n"
                "however=is\n"
                "not=valid\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_single_line
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_single_line);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);
        CATCH_REQUIRE(file->get_sections().empty());
        CATCH_REQUIRE(file->get_parameters().size() == 4);

        CATCH_REQUIRE(file->has_parameter("duplicates"));
        CATCH_REQUIRE(file->has_parameter("varying"));
        CATCH_REQUIRE(file->has_parameter("however"));
        CATCH_REQUIRE(file->has_parameter("not"));

        CATCH_REQUIRE(file->get_parameter("duplicates") == "work");
        CATCH_REQUIRE(file->get_parameter("varying") == "parameters");
        CATCH_REQUIRE(file->get_parameter("however") == "is");
        CATCH_REQUIRE(file->get_parameter("not") == "valid");

        // "reloading" that very same file but with the "wrong" parameters
        // fails
        //
        for(int lc(static_cast<int>(advgetopt::line_continuation_t::line_continuation_single_line));
            lc <= static_cast<int>(advgetopt::line_continuation_t::line_continuation_semicolon);
            ++lc)
        {
            if(static_cast<advgetopt::line_continuation_t>(lc) == advgetopt::line_continuation_t::line_continuation_single_line)
            {
                continue;
            }

            for(advgetopt::assignment_operator_t ao(0);
                ao <= advgetopt::ASSIGNMENT_OPERATOR_MASK;
                ++ao)
            {
                if(ao == advgetopt::ASSIGNMENT_OPERATOR_EQUAL)
                {
                    continue;
                }

                for(advgetopt::comment_t c(0);
                    c < advgetopt::COMMENT_MASK;
                    ++c)
                {
                    if(c == advgetopt::COMMENT_SHELL)
                    {
                        continue;
                    }

                    for(advgetopt::section_operator_t so(0);
                        so < advgetopt::SECTION_OPERATOR_MASK;
                        ++so)
                    {
                        if(c == advgetopt::SECTION_OPERATOR_NONE)
                        {
                            continue;
                        }

                        advgetopt::conf_file_setup different_setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                                        , static_cast<advgetopt::line_continuation_t>(lc)
                                        , ao
                                        , c
                                        , so);

                        CATCH_REQUIRE_THROWS_MATCHES(
                              advgetopt::conf_file::get_conf_file(different_setup)
                            , advgetopt::getopt_logic_error
                            , Catch::Matchers::ExceptionMessage(
                                          "getopt_logic_error: trying to load configuration file \""
                                        + different_setup.get_config_url()
                                        + "\" but an existing configuration file with the same name was loaded with URL: \""
                                        + setup.get_config_url()
                                        + "\"."));
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("missing_configuration_file", "[config][getopt][invalid]")
{
    CATCH_START_SECTION("Create a conf_file without the file")
    {
        for(int count(0); count < 5; ++count)
        {
            int const id(rand());
            std::string const name("delete-file-" + std::to_string(id));

            SNAP_CATCH2_NAMESPACE::init_tmp_dir("delete", name);

            {
                std::ofstream config_file;
                config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                CATCH_REQUIRE(config_file.good());
                config_file <<
                    "# Auto-generated\n"
                    "param=optional\n"
                ;
            }

            // create the setup while the file still exists
            //
            advgetopt::conf_file_setup setup(
                          SNAP_CATCH2_NAMESPACE::g_config_filename
                        , advgetopt::line_continuation_t::line_continuation_unix
                        , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                        , advgetopt::COMMENT_SHELL
                        , advgetopt::SECTION_OPERATOR_NONE);

            // get the full name before the unlink()
            std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);

            // now unlink() that file
            //
            unlink(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str());

            // still valid since we do not check again after the
            // constructor ran
            //
            CATCH_REQUIRE(setup.is_valid());
            CATCH_REQUIRE(setup.get_filename() == fn.get());
            CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
            CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
            CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
            CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_NONE);

            // so when trying to create the conf_file object it fails
            // opening the file
            //
            advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
            CATCH_REQUIRE(file->get_errno() == ENOENT);
            CATCH_REQUIRE_FALSE(file->exists());
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_sections", "[config][getopt][invalid]")
{
    CATCH_START_SECTION("variable name cannot start with a period when C operator is active")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "period-name");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a..b=red\n"
                ".a.b.c=122\n"
                "m=size\n"
                "z=edge\n"
                "z.b=line\n"
                "z..b.c=12.72\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_C);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \".a.b.c\" cannot start with"
                    " a period (.).");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 3);
        CATCH_REQUIRE(sections.find("a")    != sections.end());
        CATCH_REQUIRE(sections.find("z")    != sections.end());
        CATCH_REQUIRE(sections.find("z::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 6);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("m"));
        CATCH_REQUIRE(file->has_parameter("z"));
        CATCH_REQUIRE(file->has_parameter("z::b"));
        CATCH_REQUIRE(file->has_parameter("z::b::c"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("m") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
        CATCH_REQUIRE(file->get_parameter("z::b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::b::c") == "12.72");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("two section operators one after another can cause trouble")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "name-period-cpp-name");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a..b=red\n"
                "a.::b.c=122\n"
                "m=size\n"
                "z=edge\n"
                "z.b=line\n"
                "z..b.c=12.72\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C | advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == (advgetopt::SECTION_OPERATOR_C | advgetopt::SECTION_OPERATOR_CPP));

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"a.::b.c\" cannot start with"
                    " a scope operator (::).");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 3);
        CATCH_REQUIRE(sections.find("a")    != sections.end());
        CATCH_REQUIRE(sections.find("z")    != sections.end());
        CATCH_REQUIRE(sections.find("z::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 6);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("m"));
        CATCH_REQUIRE(file->has_parameter("z"));
        CATCH_REQUIRE(file->has_parameter("z::b"));
        CATCH_REQUIRE(file->has_parameter("z::b::c"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("m") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
        CATCH_REQUIRE(file->get_parameter("z::b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::b::c") == "12.72");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("section operator cannot appear at the end")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "name-period-name-cpp");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a..b=red\n"
                "a.b.c::=122\n"
                "m=size\n"
                "z=edge\n"
                "z.b=line\n"
                "z..b.c=12.72\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C | advgetopt::SECTION_OPERATOR_CPP);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == (advgetopt::SECTION_OPERATOR_C | advgetopt::SECTION_OPERATOR_CPP));

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"a.b.c::\" cannot end with a"
                    " section operator or be empty.");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 3);
        CATCH_REQUIRE(sections.find("a")    != sections.end());
        CATCH_REQUIRE(sections.find("z")    != sections.end());
        CATCH_REQUIRE(sections.find("z::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 6);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("m"));
        CATCH_REQUIRE(file->has_parameter("z"));
        CATCH_REQUIRE(file->has_parameter("z::b"));
        CATCH_REQUIRE(file->has_parameter("z::b::c"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("m") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
        CATCH_REQUIRE(file->get_parameter("z::b") == "line");
        CATCH_REQUIRE(file->get_parameter("z::b::c") == "12.72");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("sections not allowed")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "section-not-allowed");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a::b=red\n"
                "m.n=size\n"
                "z=edge\n"
            ;
        }

        // no errors here since we do not detect the sections in this case
        //
        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_NONE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == (advgetopt::SECTION_OPERATOR_NONE));

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                  "error: section \"a::b\" from parameter \"a::b\" on line 3 in configuration file \""
                + std::string(fn.get())
                + "\" includes a character (\\072) not acceptable for a section or"
                  " parameter name (controls, space, quotes, and \";#/=:?+\\\").");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.empty());

        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE_FALSE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("m.n"));
        CATCH_REQUIRE(file->has_parameter("z"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == std::string());
        CATCH_REQUIRE(file->get_parameter("m.n") == "size");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"blue::shepard\" cannot be added to"
                    " section \"j::k\" because there is no section support"
                    " for this configuration file.");
        file->set_parameter("j::k", "blue::shepard", "2001");
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid characters in names")
    {
        std::string const bad_chars(
                    "\x01\x02\x03\x04\x05\x06\x07"
                "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                "\x10\x11\x12\x13\x14\x15\x16\x17"
                "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
                "\x20"
                "'\";#/=:?+\\"
            );
        for(auto c : bad_chars)
        {
            // white spaces get removed from the line so we cannot test
            // them in this way
            //
            if(std::iswspace(c))
            {
                continue;
            }
            std::string bc;
            bc += c;

            for(int pos(0); pos < 3; ++pos)
            {
                std::string spos("undefined");
                std::string bad_char("undefined");
                switch(pos)
                {
                case 0:
                    spos = "start";
                    bad_char = bc + "bad-char";
                    break;

                case 1:
                    spos = "middle";
                    bad_char = "bad" + bc + "char";
                    break;

                case 2:
                    spos = "end";
                    bad_char = "bad-char" + bc;
                    break;

                }
                SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-characters", "bad-character-" + std::to_string(static_cast<int>(c)) + "-" + spos);

                {
                    std::ofstream config_file;
                    config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
                    CATCH_REQUIRE(config_file.good());
                    char op(c == '=' ? ':' : '=');
                    config_file <<
                           "good" << op << "red\n"
                        << bad_char << op << "color\n"       // <-- bad char
                           "fine" << op << "param\n";
                    ;
                }

                // no errors here since we do not detect the sections in this case
                //
                advgetopt::assignment_operator_t as(c == '='
                                        ? advgetopt::ASSIGNMENT_OPERATOR_COLON
                                        : advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
                advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                                    , advgetopt::line_continuation_t::line_continuation_unix
                                    , as
                                    , advgetopt::COMMENT_NONE
                                    , advgetopt::SECTION_OPERATOR_NONE);

                CATCH_REQUIRE(setup.is_valid());
                CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
                CATCH_REQUIRE(setup.get_assignment_operator() == as);
                CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_NONE);
                CATCH_REQUIRE(setup.get_section_operator() == (advgetopt::SECTION_OPERATOR_NONE));

                std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
                std::stringstream octal_char;
                octal_char << std::oct << std::setw(3) << std::setfill('0') << static_cast<int>(c);
                SNAP_CATCH2_NAMESPACE::push_expected_log(
                          "error: section \""
                        + bad_char
                        + "\" from parameter \""
                        + bad_char
                        + "\" on line 2 in configuration file \""
                        + fn.get()
                        + "\" includes a character (\\"
                        + octal_char.str()
                        + ") not acceptable for a section or"
                          " parameter name (controls, space, quotes, and \";#/=:?+\\\").");
                advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));

                CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
                CATCH_REQUIRE(file->get_errno() == 0);

                advgetopt::conf_file::sections_t sections(file->get_sections());
                CATCH_REQUIRE(sections.empty());

                CATCH_REQUIRE(file->get_parameters().size() == 2);

                CATCH_REQUIRE(file->has_parameter("good"));
                CATCH_REQUIRE_FALSE(file->has_parameter(bad_char));
                CATCH_REQUIRE(file->has_parameter("fine"));

                CATCH_REQUIRE(file->get_parameter("good") == "red");
                CATCH_REQUIRE(file->get_parameter(bad_char) == std::string());
                CATCH_REQUIRE(file->get_parameter("fine") == "param");
            }
        }
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("too many sections")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "too-many-sections");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "a::b=red\n"
                "m.n.o=size\n"
                "z=edge\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_CPP | advgetopt::SECTION_OPERATOR_C | advgetopt::SECTION_OPERATOR_ONE_SECTION);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == (advgetopt::SECTION_OPERATOR_CPP | advgetopt::SECTION_OPERATOR_C | advgetopt::SECTION_OPERATOR_ONE_SECTION));

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: option name \"m.n.o\" cannot be added to section"
                    " \"m::n\" because this configuration only accepts one"
                    " section level.");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 1);
        CATCH_REQUIRE(sections.find("a") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 3);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("z"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("z") == "edge");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("all '{' were not closed")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "unclosed-brackets");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "colors {\n"
                "  b=red\n"
                "  c=blue\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_BLOCK);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_BLOCK);

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                    "error: unterminated `section { ... }`, the `}` is missing"
                    " in configuration file "
                    "\""
                  + std::string(fn.get())
                  + "\".");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 1);
        CATCH_REQUIRE(sections.find("colors") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("colors::b"));
        CATCH_REQUIRE(file->has_parameter("colors::c"));

        CATCH_REQUIRE(file->get_parameter("colors::b") == "red");
        CATCH_REQUIRE(file->get_parameter("colors::c") == "blue");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("data after ']' in INI file")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "additional-data");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "[colors]\n"
                "b=red\n"
                "c=blue\n"
                "\n"
                "[sizes] comment\n"     // <- missing the comment introducer
                "q=1000\n"
                "r=9999\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_INI_FILE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_INI_FILE);

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: section names in configuration files cannot be followed by anything other than spaces in"
                      " \"[sizes] comment\" on line 6 from configuration file \""
                    + std::string(fn.get())
                    + "\".");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 1);
        CATCH_REQUIRE(sections.find("colors") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 4);

        CATCH_REQUIRE(file->has_parameter("colors::b"));
        CATCH_REQUIRE(file->has_parameter("colors::c"));
        CATCH_REQUIRE(file->has_parameter("colors::q"));
        CATCH_REQUIRE(file->has_parameter("colors::r"));

        CATCH_REQUIRE(file->get_parameter("colors::b") == "red");
        CATCH_REQUIRE(file->get_parameter("colors::c") == "blue");
        CATCH_REQUIRE(file->get_parameter("colors::q") == "1000");
        CATCH_REQUIRE(file->get_parameter("colors::r") == "9999");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("INI file section inside a block is not allowed")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-section-operator", "ini-inside-block");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "[colors]\n"
                "b=red\n"
                "c=blue\n"
                "\n"
                "block {\n"
                "  b = block data\n"
                "  f = filename\n"
                "  [sizes]\n"       // <-- INI section inside a block not allowed
                "  q=1000\n"
                "  r=9999\n"
                "}\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_BLOCK | advgetopt::SECTION_OPERATOR_INI_FILE);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == (advgetopt::SECTION_OPERATOR_BLOCK | advgetopt::SECTION_OPERATOR_INI_FILE));

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: `[...]` sections can't be used within a `section"
                      " { ... }` on line 9 from configuration file \""
                    + std::string(fn.get())
                    + "\".");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 2);
        CATCH_REQUIRE(sections.find("colors") != sections.end());
        CATCH_REQUIRE(sections.find("colors::block") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 6);

        CATCH_REQUIRE(file->has_parameter("colors::b"));
        CATCH_REQUIRE(file->has_parameter("colors::c"));
        CATCH_REQUIRE(file->has_parameter("colors::block::b"));
        CATCH_REQUIRE(file->has_parameter("colors::block::f"));
        CATCH_REQUIRE(file->has_parameter("colors::block::q"));
        CATCH_REQUIRE(file->has_parameter("colors::block::r"));

        CATCH_REQUIRE(file->get_parameter("colors::b") == "red");
        CATCH_REQUIRE(file->get_parameter("colors::c") == "blue");
        CATCH_REQUIRE(file->get_parameter("colors::block::b") == "block data");
        CATCH_REQUIRE(file->get_parameter("colors::block::f") == "filename");
        CATCH_REQUIRE(file->get_parameter("colors::block::q") == "1000");
        CATCH_REQUIRE(file->get_parameter("colors::block::r") == "9999");
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("invalid_variable_name", "[config][getopt][invalid]")
{
    CATCH_START_SECTION("empty variable name")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-variable-name", "name-missing");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "=color\n"                  // <-- name missing
                "a..b=red\n"
                "a.b.c=142\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_C);

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: no option name in \"=color\""
                      " on line 2 from configuration file \""
                    + std::string(fn.get())
                    + "\", missing name before the assignment operator?");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 2);
        CATCH_REQUIRE(sections.find("a")    != sections.end());
        CATCH_REQUIRE(sections.find("a::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("a::b::c"));

        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("a::b::c") == "142");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("empty variable name after section name")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-variable-name", "section-and-name-missing");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a..b=red\n"
                "a.b.=color\n"                  // <-- name missing after section name
                "a.b.c=142\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_C);

        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: option name \"a.b.\" cannot end with a section operator or be empty.");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        advgetopt::conf_file::sections_t sections(file->get_sections());
        CATCH_REQUIRE(sections.size() == 2);
        CATCH_REQUIRE(sections.find("a") != sections.end());
        CATCH_REQUIRE(sections.find("a::b") != sections.end());

        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("a::b"));
        CATCH_REQUIRE(file->has_parameter("a::b::c"));

        CATCH_REQUIRE(file->get_parameter("a::b") == "red");
        CATCH_REQUIRE(file->get_parameter("a::b::c") == "142");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable name starts with a dash")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-variable-name", "dash-name");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a=color\n"
                "-bad-dash=reddish\n"            // <-- name starts with '-'
                "size=412\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_C);

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: option names in configuration files cannot"
                      " start with a dash or an underscore in"
                      " \"-bad-dash=reddish\" on line 3 from configuration file \""
                    + std::string(fn.get())
                    + "\".");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        CATCH_REQUIRE(file->get_sections().empty());

        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("a"));
        CATCH_REQUIRE(file->has_parameter("size"));

        CATCH_REQUIRE(file->get_parameter("a") == "color");
        CATCH_REQUIRE(file->get_parameter("size") == "412");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable name starts with an underscore")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-variable-name", "underscore-name");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a_variable=color\n"
                "_bad_underscore=reddish\n"        // <-- name starts with '_'
                "pos_and_size=412x33+32-18\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_C);

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: option names in configuration files cannot"
                      " start with a dash or an underscore in"
                      " \"_bad_underscore=reddish\" on line 3 from configuration file \""
                    + std::string(fn.get())
                    + "\".");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        CATCH_REQUIRE(file->get_sections().empty());

        CATCH_REQUIRE(file->get_parameters().size() == 2);

        CATCH_REQUIRE(file->has_parameter("a-variable"));
        CATCH_REQUIRE(file->has_parameter("pos-and-size"));

        CATCH_REQUIRE(file->get_parameter("a-variable") == "color");
        CATCH_REQUIRE(file->get_parameter("pos-and-size") == "412x33+32-18");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable name with spaces")
    {
        SNAP_CATCH2_NAMESPACE::init_tmp_dir("invalid-variable-name", "name-space-more-name");

        {
            std::ofstream config_file;
            config_file.open(SNAP_CATCH2_NAMESPACE::g_config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "a variable=color\n"
                "bad space=reddish\n"
                "pos and size=412x33+32-18\n"
            ;
        }

        advgetopt::conf_file_setup setup(SNAP_CATCH2_NAMESPACE::g_config_filename
                            , advgetopt::line_continuation_t::line_continuation_unix
                            , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
                            , advgetopt::COMMENT_SHELL
                            , advgetopt::SECTION_OPERATOR_C);

        CATCH_REQUIRE(setup.is_valid());
        CATCH_REQUIRE(setup.get_line_continuation() == advgetopt::line_continuation_t::line_continuation_unix);
        CATCH_REQUIRE(setup.get_assignment_operator() == advgetopt::ASSIGNMENT_OPERATOR_EQUAL);
        CATCH_REQUIRE(setup.get_comment() == advgetopt::COMMENT_SHELL);
        CATCH_REQUIRE(setup.get_section_operator() == advgetopt::SECTION_OPERATOR_C);

        std::unique_ptr<char, decltype(&::free)> fn(realpath(SNAP_CATCH2_NAMESPACE::g_config_filename.c_str(), nullptr), &::free);
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: option name from \"a variable=color\" on line"
                      " 2 in configuration file \""
                    + std::string(fn.get())
                    + "\" cannot include a space, missing assignment operator?");
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: option name from \"bad space=reddish\" on line"
                      " 3 in configuration file \""
                    + std::string(fn.get())
                    + "\" cannot include a space, missing assignment operator?");
        SNAP_CATCH2_NAMESPACE::push_expected_log(
                      "error: option name from \"pos and size=412x33+32-18\" on line"
                      " 4 in configuration file \""
                    + std::string(fn.get())
                    + "\" cannot include a space, missing assignment operator?");
        advgetopt::conf_file::pointer_t file(advgetopt::conf_file::get_conf_file(setup));
        SNAP_CATCH2_NAMESPACE::expected_logs_stack_is_empty();

        CATCH_REQUIRE(file->get_setup().get_config_url() == setup.get_config_url());
        CATCH_REQUIRE(file->get_errno() == 0);

        CATCH_REQUIRE(file->get_sections().empty());

        CATCH_REQUIRE(file->get_parameters().empty());
    }
    CATCH_END_SECTION()
}





// vim: ts=4 sw=4 et
