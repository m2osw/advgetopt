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
#pragma once

/** \file
 * \brief Declaration of the variable class.
 *
 * The advgetopt library supports variables in its configuration files.
 *
 * This class handles the variable data store and replacement in strings.
 */

// self
//
#include    <advgetopt/utils.h>


// C++
//
#include    <map>
#include    <memory>
#include    <string>



namespace advgetopt
{



enum class assignment_t
{
    ASSIGNMENT_NONE,

    ASSIGNMENT_SET,
    ASSIGNMENT_OPTIONAL,
    ASSIGNMENT_APPEND,
    ASSIGNMENT_NEW,
};


class variables
{
public:
    typedef std::shared_ptr<variables>          pointer_t;
    typedef std::map<std::string, std::string>  variable_t;

    bool                    has_variable(std::string const & name) const;
    std::string             get_variable(std::string const & name) const;
    variable_t const &      get_variables() const;
    void                    set_variable(
                                  std::string const & name
                                , std::string const & value
                                , assignment_t assignment = assignment_t::ASSIGNMENT_SET);

    std::string             process_value(std::string const & value) const;

    static std::string      canonicalize_variable_name(std::string const & name);

private:
    typedef string_set_t    variable_names_t;

    std::string             recursive_process_value(
                                  std::string const & value
                                , variable_names_t & names) const;

    variable_t              f_variables = variable_t();
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
