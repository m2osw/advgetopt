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
#include "advgetopt/validator.h"
#include "advgetopt/utils.h"

// C++ lib
//
#include <map>
#include <memory>


namespace advgetopt
{


typedef uint32_t            short_name_t;

constexpr short_name_t      NO_SHORT_NAME = L'\0';


typedef std::uint32_t       flag_t;

static constexpr flag_t     GETOPT_FLAG_NONE                 = static_cast<flag_t>(0x00000000);

static constexpr flag_t     GETOPT_FLAG_COMMAND_LINE         = static_cast<flag_t>(0x00000001);  // acceptable on the command line
static constexpr flag_t     GETOPT_FLAG_ENVIRONMENT_VARIABLE = static_cast<flag_t>(0x00000002);  // acceptable in environment variable
static constexpr flag_t     GETOPT_FLAG_CONFIGURATION_FILE   = static_cast<flag_t>(0x00000004);  // acceptable in configuration files

static constexpr flag_t     GETOPT_FLAG_ALIAS                = static_cast<flag_t>(0x00000010);  // alias, result in another option defined in "help" string
static constexpr flag_t     GETOPT_FLAG_FLAG                 = static_cast<flag_t>(0x00000020);  // no parameter allowed (-h)
static constexpr flag_t     GETOPT_FLAG_REQUIRED             = static_cast<flag_t>(0x00000040);  // required (--host 127.0.0.1)
static constexpr flag_t     GETOPT_FLAG_MULTIPLE             = static_cast<flag_t>(0x00000080);  // any number of parameter is allowed (--files a b c d ...)
static constexpr flag_t     GETOPT_FLAG_HAS_DEFAULT          = static_cast<flag_t>(0x00000100);  // default value is defined

static constexpr flag_t     GETOPT_FLAG_SHOW_USAGE_ON_ERROR  = static_cast<flag_t>(0x00001000);  // show in usage() when an error occurs
static constexpr flag_t     GETOPT_FLAG_SHOW_ALL             = static_cast<flag_t>(0x00002000);  // show in usage() when --long-help is used
static constexpr flag_t     GETOPT_FLAG_SHOW_GROUP1          = static_cast<flag_t>(0x00004000);  // show in usage() when --<group1>-help is used (app dependent)
static constexpr flag_t     GETOPT_FLAG_SHOW_GROUP2          = static_cast<flag_t>(0x00008000);  // show in usage() when --<group2>-help is used (app dependent)

static constexpr flag_t     GETOPT_FLAG_DYNAMIC              = static_cast<flag_t>(0x20000000);  // this value was found in a configuration file and dynamic parameters are allowed (i.e. no definition for this option was found)
static constexpr flag_t     GETOPT_FLAG_LOCK                 = static_cast<flag_t>(0x40000000);  // this value is currently locked (can't be modified)

static constexpr flag_t     GETOPT_FLAG_END                  = static_cast<flag_t>(0x80000000);  // mark the end of the list





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
    void                        set_minimum(std::string const & value);
    void                        set_maximum(std::string const & value);

    void                        set_range(std::string const & min, std::string const & max);
    std::string const &         get_min() const;
    std::string const &         get_max() const;

    void                        add_child(option_info::pointer_t child);
    map_by_name_t const &       get_children() const;
    pointer_t                   get_child(std::string const & name) const;
    pointer_t                   get_child(short_name_t short_name) const;
    void                        set_alias(option_info::pointer_t alias);
    option_info::pointer_t      get_alias() const;

    void                        set_multiple_separators(string_list_t const & separators);
    void                        set_multiple_separators(char const ** separators);
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
    std::string                 f_minimum_value = std::string();
    std::string                 f_maximum_value = std::string();
    std::string                 f_help = std::string();
    validator::pointer_t        f_validator = validator::pointer_t();
    pointer_t                   f_alias = pointer_t();
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
