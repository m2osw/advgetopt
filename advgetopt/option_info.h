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
#include    "advgetopt/flags.h"
#include    "advgetopt/validator.h"
#include    "advgetopt/variables.h"


// C++ lib
//
#include    <functional>
#include    <map>



namespace advgetopt
{


typedef char32_t            short_name_t;

constexpr short_name_t      NO_SHORT_NAME = U'\0';


short_name_t                string_to_short_name(std::string const & name);
std::string                 short_name_to_string(short_name_t short_name);




enum class option_source_t
{
    SOURCE_COMMAND_LINE,            // set on command line
    SOURCE_CONFIGURATION,           // read from config file
    SOURCE_DIRECT,                  // set by programmer directly
    SOURCE_DYNAMIC,                 // set dynamically
    SOURCE_ENVIRONMENT_VARIABLE,    // found in environment variable

    SOURCE_UNDEFINED,               // the option object exists, but the value is still undefined
};


// the `option_info` can be used instead or on top of the `struct option`
// it is especially used to read an external getopt declaration file
//
class option_info
{
public:
    typedef std::shared_ptr<option_info>                    pointer_t;
    typedef std::vector<pointer_t>                          vector_t;
    typedef std::map<std::string, pointer_t>                map_by_name_t;
    typedef std::map<short_name_t, pointer_t>               map_by_short_name_t;
    typedef std::function<void (option_info const & opt)>   callback_t;
    typedef int                                             callback_id_t;

                                option_info(std::string const & name, short_name_t short_name = NO_SHORT_NAME);

    std::string const &         get_name() const;
    void                        set_short_name(short_name_t short_name);
    short_name_t                get_short_name() const;
    std::string                 get_basename() const;
    std::string                 get_section_name() const;
    string_list_t               get_section_name_list() const;
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

    bool                        set_validator(std::string const & name_and_params);
    bool                        set_validator(validator::pointer_t validator);
    bool                        set_validator(std::nullptr_t);
    validator::pointer_t        get_validator() const;

    void                        set_alias_destination(option_info::pointer_t destination);
    option_info::pointer_t      get_alias_destination() const;

    void                        set_multiple_separators(string_list_t const & separators);
    void                        set_multiple_separators(char const * const * separators);
    string_list_t const &       get_multiple_separators() const;

    void                        set_variables(variables::pointer_t vars);
    variables::pointer_t        get_variables() const;
    bool                        has_value(std::string const & value) const;
    bool                        add_value(std::string const & value, option_source_t source = option_source_t::SOURCE_DIRECT);
    bool                        set_value(int idx, std::string const & value, option_source_t source = option_source_t::SOURCE_DIRECT);
    bool                        set_multiple_values(std::string const & value, option_source_t source = option_source_t::SOURCE_DIRECT);
    bool                        is_defined() const;
    option_source_t             source() const;
    static void                 set_trace_sources(bool trace);
    string_list_t const &       trace_sources() const;
    static void                 set_configuration_filename(std::string const & filename);
    size_t                      size() const;
    std::string                 get_value(int idx = 0, bool raw = false) const;
    long                        get_long(int idx = 0) const;
    double                      get_double(int idx = 0) const;
    void                        lock(bool always = true);
    void                        unlock();
    void                        reset();

    callback_id_t               add_callback(callback_t const & c);
    void                        remove_callback(callback_id_t id);

private:
    struct callback_entry_t
    {
        callback_entry_t(callback_id_t id, callback_t const & c)
            : f_id(id)
            , f_callback(c)
        {
        }

        callback_id_t           f_id = 0;
        callback_t              f_callback = callback_t();
    };
    typedef std::vector<callback_entry_t>
                                callback_vector_t;

    bool                        validate_all_values();
    bool                        validates(int idx = 0);
    void                        value_changed(int idx);
    void                        trace_source(int idx);

    // definitions
    //
    std::string                 f_name = std::string();
    short_name_t                f_short_name = NO_SHORT_NAME;
    flag_t                      f_flags = GETOPT_FLAG_NONE;
    std::string                 f_default_value = std::string();
    std::string                 f_help = std::string();
    validator::pointer_t        f_validator = validator::pointer_t();
    pointer_t                   f_alias_destination = pointer_t();
    string_list_t               f_multiple_separators = string_list_t();
    callback_vector_t           f_callbacks = callback_vector_t();
    id_t                        f_next_callback_id = 0;
    string_list_t               f_trace_sources = string_list_t();
    variables::pointer_t        f_variables = variables::pointer_t();

    // value read from command line, environment, .conf file
    //
    option_source_t             f_source = option_source_t::SOURCE_UNDEFINED;
    string_list_t               f_value = string_list_t();
    mutable std::vector<long>   f_integer = std::vector<long>();
    mutable std::vector<double> f_double = std::vector<double>();
};


class option_info_ref
{
public:
                                option_info_ref(option_info::pointer_t opt);

    bool                        empty() const;
    size_t                      length() const;
    size_t                      size() const;
    long                        get_long() const;
    double                      get_double() const;

                                operator std::string () const;

    option_info_ref &           operator = (char value);
    option_info_ref &           operator = (char32_t value);
    option_info_ref &           operator = (char const * value);
    option_info_ref &           operator = (std::string const & value);
    option_info_ref &           operator = (option_info_ref const & value);

    option_info_ref &           operator += (char value);
    option_info_ref &           operator += (char32_t value);
    option_info_ref &           operator += (char const * value);
    option_info_ref &           operator += (std::string const & value);
    option_info_ref &           operator += (option_info_ref const & value);

    std::string                 operator + (char value) const;
    std::string                 operator + (char32_t value) const;
    std::string                 operator + (char const * value) const;
    std::string                 operator + (std::string const & value) const;
    std::string                 operator + (option_info_ref const & value) const;

    friend std::string          operator + (char value, option_info_ref const & rhs);
    friend std::string          operator + (char32_t value, option_info_ref const & rhs);
    friend std::string          operator + (char const * value, option_info_ref const & rhs);
    friend std::string          operator + (std::string const & value, option_info_ref const & rhs);

                                operator bool () const;
    bool                        operator ! () const;

    bool                        operator == (char const * value) const;
    bool                        operator == (std::string const & value) const;
    bool                        operator == (option_info_ref const & value) const;

    bool                        operator != (char const * value) const;
    bool                        operator != (std::string const & value) const;
    bool                        operator != (option_info_ref const & value) const;

    bool                        operator < (char const * value) const;
    bool                        operator < (std::string const & value) const;
    bool                        operator < (option_info_ref const & value) const;

    bool                        operator <= (char const * value) const;
    bool                        operator <= (std::string const & value) const;
    bool                        operator <= (option_info_ref const & value) const;

    bool                        operator > (char const * value) const;
    bool                        operator > (std::string const & value) const;
    bool                        operator > (option_info_ref const & value) const;

    bool                        operator >= (char const * value) const;
    bool                        operator >= (std::string const & value) const;
    bool                        operator >= (option_info_ref const & value) const;

    friend bool                 operator == (char const * value, option_info_ref const & rhs);
    friend bool                 operator == (std::string const & value, option_info_ref const & rhs);

    friend bool                 operator != (char const * value, option_info_ref const & rhs);
    friend bool                 operator != (std::string const & value, option_info_ref const & rhs);

    friend bool                 operator < (char const * value, option_info_ref const & rhs);
    friend bool                 operator < (std::string const & value, option_info_ref const & rhs);

    friend bool                 operator <= (char const * value, option_info_ref const & rhs);
    friend bool                 operator <= (std::string const & value, option_info_ref const & rhs);

    friend bool                 operator > (char const * value, option_info_ref const & rhs);
    friend bool                 operator > (std::string const & value, option_info_ref const & rhs);

    friend bool                 operator >= (char const * value, option_info_ref const & rhs);
    friend bool                 operator >= (std::string const & value, option_info_ref const & rhs);

private:
    option_info::pointer_t      f_opt = option_info::pointer_t();
};


}   // namespace advgetopt


std::string operator + (char32_t value, std::string const & rhs);
std::string operator + (std::string const & lhs, char32_t value);


// vim: ts=4 sw=4 et
