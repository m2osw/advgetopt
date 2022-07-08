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
 * \brief Declaration of the conf_file class used to read a configuration file.
 *
 * The library offers a way to read a configuration file parameters without
 * doing anything more than that. The getopt class uses it to read and verify
 * the parameters. It also uses it to read files of configuration options.
 */

// self
//
#include    <advgetopt/variables.h>


// C++
//
#include    <fstream>
#include    <functional>
#include    <map>
#include    <memory>
#include    <set>



namespace advgetopt
{



enum class callback_action_t
{
    created,
    updated,
    erased
};


enum class line_continuation_t
{
    line_continuation_single_line,    // no continuation support
    line_continuation_rfc_822,        // like email/HTTP, whitespace at start of next line
    line_continuation_msdos,          // '&' at end of line
    line_continuation_unix,           // '\' at end of line
    line_continuation_fortran,        // '&' at start of next line
    line_continuation_semicolon       // ';' ends the _line_
};


typedef std::uint_fast16_t      assignment_operator_t;

constexpr assignment_operator_t ASSIGNMENT_OPERATOR_EQUAL       = 0x0001;       // a = b
constexpr assignment_operator_t ASSIGNMENT_OPERATOR_COLON       = 0x0002;       // a: b
constexpr assignment_operator_t ASSIGNMENT_OPERATOR_SPACE       = 0x0004;       // a b

constexpr assignment_operator_t ASSIGNMENT_OPERATOR_MASK        = 0x0007;


typedef std::uint_fast16_t      comment_t;

constexpr comment_t             COMMENT_NONE                    = 0x0000;       // no support for comments
constexpr comment_t             COMMENT_INI                     = 0x0001;       // ; comment
constexpr comment_t             COMMENT_SHELL                   = 0x0002;       // # comment
constexpr comment_t             COMMENT_CPP                     = 0x0004;       // // comment

constexpr comment_t             COMMENT_SAVE                    = 0x8000;       // save comments along parameters

constexpr comment_t             COMMENT_MASK                    = 0x0007;


typedef std::uint_fast16_t      section_operator_t;

constexpr section_operator_t    SECTION_OPERATOR_NONE           = 0x0000;       // no support
constexpr section_operator_t    SECTION_OPERATOR_C              = 0x0001;       // a.b
constexpr section_operator_t    SECTION_OPERATOR_CPP            = 0x0002;       // a::b
constexpr section_operator_t    SECTION_OPERATOR_BLOCK          = 0x0004;       // a { ... }
constexpr section_operator_t    SECTION_OPERATOR_INI_FILE       = 0x0008;       // [a]

constexpr section_operator_t    SECTION_OPERATOR_ONE_SECTION    = 0x8000;       // accept at most 1 section

constexpr section_operator_t    SECTION_OPERATOR_MASK           = 0x000F;


typedef std::uint_fast16_t      name_separator_t;

constexpr name_separator_t      NAME_SEPARATOR_UNDERSCORES      = 0x0001;       // output underscore ('_') instead of dashes
constexpr name_separator_t      NAME_SEPARATOR_DASHES           = 0x0002;       // output dashes ('-')


class conf_file_setup
{
public:
                                conf_file_setup(
                                              std::string const & filename
                                            , line_continuation_t line_continuation = line_continuation_t::line_continuation_unix
                                            , assignment_operator_t assignment_operator = ASSIGNMENT_OPERATOR_EQUAL
                                            , comment_t comment = COMMENT_INI | COMMENT_SHELL
                                            , section_operator_t section_operator = SECTION_OPERATOR_INI_FILE
                                            , name_separator_t name_separator = NAME_SEPARATOR_UNDERSCORES);

    bool                        is_valid() const;
    std::string const &         get_original_filename() const;
    std::string const &         get_filename() const;
    line_continuation_t         get_line_continuation() const;
    assignment_operator_t       get_assignment_operator() const;
    comment_t                   get_comment() const;
    section_operator_t          get_section_operator() const;
    std::string                 get_config_url() const;
    name_separator_t            get_name_separator() const;

private:
    std::string                 f_original_filename = std::string();
    std::string                 f_filename = std::string();
    line_continuation_t         f_line_continuation = line_continuation_t::line_continuation_unix;
    assignment_operator_t       f_assignment_operator = ASSIGNMENT_OPERATOR_EQUAL;
    comment_t                   f_comment = COMMENT_INI | COMMENT_SHELL;
    section_operator_t          f_section_operator = SECTION_OPERATOR_INI_FILE;
    mutable std::string         f_url = std::string();
    name_separator_t            f_name_separator = NAME_SEPARATOR_UNDERSCORES;
};


class parameter_value
{
public:
                                parameter_value();
                                parameter_value(parameter_value const & rhs);
                                parameter_value(std::string const & value);

    parameter_value &           operator = (parameter_value const & rhs);
    parameter_value &           operator = (std::string const & value);
                                operator std::string () const;

    void                        set_value(std::string const & value);
    void                        set_comment(std::string const & comment);
    void                        set_line(int line);

    std::string const &         get_value() const;
    std::string                 get_comment(bool ensure_newline = false) const;
    int                         get_line() const;

private:
    std::string                 f_value = std::string();
    std::string                 f_comment = std::string();
    int                         f_line = 0;
};


class conf_file
    : public std::enable_shared_from_this<conf_file>
{
public:
    typedef std::shared_ptr<conf_file>              pointer_t;
    typedef string_set_t                            sections_t;
    typedef std::map<std::string, parameter_value>  parameters_t;
    typedef std::function<void(
                  pointer_t conf_file
                , callback_action_t action
                , std::string const & parameter_name
                , std::string const & value)>       callback_t;
    typedef int                                     callback_id_t;

    static pointer_t            get_conf_file(conf_file_setup const & setup);

    bool                        save_configuration(
                                      std::string backup_extension = std::string(".bak")
                                    , bool replace_backup = false
                                    , bool prepend_warning = true
                                    , std::string output_filename = std::string());

    conf_file_setup const &     get_setup() const;
    callback_id_t               add_callback(
                                      callback_t const & c
                                    , std::string const & parameter_name = std::string());
    void                        remove_callback(callback_id_t id);

    bool                        exists() const;
    int                         get_errno() const;

    int                         section_to_variables(
                                      std::string const & section_name
                                    , variables::pointer_t var);
    void                        set_variables(variables::pointer_t variables);
    variables::pointer_t        get_variables() const;
    sections_t                  get_sections() const;
    parameters_t                get_parameters() const;
    bool                        has_parameter(std::string name) const;
    std::string                 get_parameter(std::string name) const;
    bool                        set_parameter(
                                      std::string section
                                    , std::string name
                                    , std::string const & value
                                    , std::string const & comment = std::string());
    bool                        erase_parameter(std::string name);
    void                        erase_all_parameters();
    bool                        was_modified() const;

    bool                        is_assignment_operator(int c) const;
    bool                        is_comment(char const * s) const;

private:
    struct callback_entry_t
    {
        callback_entry_t(
                    callback_id_t id
                  , callback_t const & c
                  , std::string const & name)
            : f_id(id)
            , f_callback(c)
            , f_parameter_name(name)
        {
        }

        callback_id_t           f_id = 0;
        callback_t              f_callback = callback_t();
        std::string             f_parameter_name = std::string();
    };
    typedef std::vector<callback_entry_t>
                                callback_vector_t;

                                conf_file(conf_file_setup const & setup);

    int                         getc(std::ifstream & stream);
    void                        ungetc(int c);
    bool                        get_line(std::ifstream & stream, std::string & line);
    void                        read_configuration();
    void                        value_changed(
                                      callback_action_t action
                                    , std::string const & parameter_name
                                    , std::string const & value);

    conf_file_setup const       f_setup;

    int                         f_unget_char = '\0';
    int                         f_line = 0;
    int                         f_errno = 0;
    bool                        f_reading = false;
    bool                        f_exists = false;

    bool                        f_modified = false;
    sections_t                  f_sections = sections_t();
    variables::pointer_t        f_variables = variables::pointer_t();
    parameters_t                f_parameters = parameters_t();
    callback_vector_t           f_callbacks = callback_vector_t();
    callback_id_t               f_next_callback_id = 0;
};


bool iswspace(int c);


}   // namespace advgetopt
// vim: ts=4 sw=4 et
