/*
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
 *    Doug Barbieri  doug@m2osw.com
 */
#pragma once

/** \file
 * \brief A few utility functions that are not specific to an object.
 *
 * The following are useful functions offered publicly since they may
 * also be useful to others outside of the library.
 */

// C++ lib
//
#include    <string>
#include    <vector>



namespace advgetopt
{



typedef std::vector<std::string>                string_list_t;


std::string         unquote(std::string const & s, std::string const & pairs = "\"\"''");
void                split_string(std::string const & str
                               , string_list_t & result
                               , string_list_t const & separators);
string_list_t       insert_group_name(std::string const & filename
                                    , char const * group_name
                                    , char const * project_name);
std::string         handle_user_directory(std::string const & filename);
bool                is_true(std::string s);
bool                is_false(std::string s);



}   // namespace advgetopt
// vim: ts=4 sw=4 et
