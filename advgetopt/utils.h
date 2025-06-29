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
#pragma once

/** \file
 * \brief A few utility functions that are not specific to an object.
 *
 * The following are useful functions offered publicly since they may
 * also be useful to others outside of the library.
 */

// C++
//
#include    <set>
#include    <string>
#include    <vector>



namespace advgetopt
{



typedef std::vector<std::string>                string_list_t;
typedef std::set<std::string>                   string_set_t;

constexpr int const DEFAULT_PRIORITY = 50;


std::string         unquote(std::string const & s, std::string const & pairs = "\"\"''");
std::string         quote(std::string const & s, char open = '"', char close = '\0');
std::string         option_with_dashes(std::string const & s);
std::string         option_with_underscores(std::string const & s);
void                split_string(std::string const & str
                               , string_list_t & result
                               , string_list_t const & separators);
string_list_t       insert_group_name(std::string const & filename
                                    , char const * group_name
                                    , char const * project_name
                                    , bool add_default_on_empty = true);
std::string         default_group_name(std::string const & filename
                                    , char const * group_name
                                    , char const * project_name
                                    , int priority = DEFAULT_PRIORITY);
std::string         handle_user_directory(std::string const & filename);
bool                is_true(std::string s);
bool                is_false(std::string s);
std::string         breakup_line(std::string line
                               , std::size_t const option_width
                               , std::size_t const line_width);
std::string         format_usage_string(std::string const & argument
                                      , std::string const & help
                                      , std::size_t const option_width
                                      , std::size_t const line_width);
std::size_t         get_screen_width();
std::size_t         get_screen_height();
std::string         sanitizer_details();
std::string         escape_shell_argument(std::string const & arg);
void                less(std::basic_ostream<char> & out
                       , std::string const & data);



}   // namespace advgetopt
// vim: ts=4 sw=4 et
