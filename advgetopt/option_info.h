/*
 * File:
 *    advgetopt/option_info.h -- a replacement to the Unix getopt() implementation
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
 *    Doug Barbieri  doug@m2osw.com
 */
#pragma once

/** \file
 * \brief Declaration of the option_info class used to record available options.
 *
 * The library offers a way to verify your command line and other options
 * with features such as validators and reading of various types of
 * configuration files.
 *
 * The class defined in this file is used to describe an option.
 */

// advgetopt lib
//
#include "advgetopt/flags.h"
#include "advgetopt/utils.h"
#include "advgetopt/validator.h"


// C++ lib
//
#include <map>
#include <memory>



namespace advgetopt
{


typedef char32_t            short_name_t;

constexpr short_name_t      NO_SHORT_NAME = L'\0';






// the `option_info` can be used instead or on top of the `struct option`
// it is especially used to read an external getopt declaration file
//
class option_info
{
public:
    typedef std::shared_ptr<option_info>            pointer_t;
    typedef std::vector<pointer_t>                  vector_t;
    typedef std::map<std::string, pointer_t>        map_by_name_t;
    typedef std::map<short_name_t, pointer_t>       map_by_short_name_t;

                                option_info(std::string const & name, short_name_t short_name = NO_SHORT_NAME);

    std::string const &         get_name() const;
    short_name_t                get_short_name() const;
    bool                        is_default_option() const;

    void                        set_flags(flag_t flags);
    void                        add_flag(flag_t flag);
    void                        remove_flag(flag_t flag);
    flag_t                      get_flags() const;
    bool                        has_flag(flag_t flag) const;

    bool                        has_default() const;
    void                        set_default(std::string const & default_value);
    void                        set_default(char const * default_value);
    void                        remove_default();
    std::string const &         get_default() const;

    void                        set_help(std::string const & help);
    void                        set_help(char const * help);
    std::string const &         get_help() const;

    void                        set_validator(validator::pointer_t validator);
    bool                        validates(int idx = 0) const;
    validator::pointer_t        get_validator() const;

    void                        add_child(option_info::pointer_t child);
    map_by_name_t const &       get_children() const;
    pointer_t                   get_child(std::string const & name) const;
    pointer_t                   get_child(short_name_t short_name) const;
    void                        set_alias_destination(option_info::pointer_t destination);
    option_info::pointer_t      get_alias_destination() const;

    void                        set_multiple_separators(string_list_t const & separators);
    void                        set_multiple_separators(char const * const * separators);
    string_list_t const &       get_multiple_separators() const;
    void                        add_value(std::string const & value);
    void                        set_value(int idx, std::string const & value);
    void                        set_multiple_value(std::string const & value);
    bool                        is_defined() const;
    size_t                      size() const;
    std::string const &         get_value(int idx = 0) const;
    long                        get_long(int idx = 0) const;
    void                        lock(bool always = true);
    void                        unlock();
    void                        reset();

private:
    // definitions
    //
    std::string                 f_name = std::string();
    short_name_t                f_short_name = NO_SHORT_NAME;
    flag_t                      f_flags = GETOPT_FLAG_NONE;
    std::string                 f_default_value = std::string();
    std::string                 f_help = std::string();
    validator::pointer_t        f_validator = validator::pointer_t();
    pointer_t                   f_alias_destination = pointer_t();
    map_by_name_t               f_children_by_long_name = map_by_name_t();
    map_by_short_name_t         f_children_by_short_name = map_by_short_name_t();
    string_list_t               f_multiple_separators = string_list_t();

    // value read from command line, environment, .conf file
    //
    std::vector<std::string>    f_value = std::vector<std::string>();
    mutable std::vector<long>   f_integer = std::vector<long>();
};


}   // namespace advgetopt
// vim: ts=4 sw=4 et
