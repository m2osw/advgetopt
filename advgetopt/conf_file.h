/*
 * File:
 *    advgetopt/conf_file.h -- a replacement to the Unix getopt() implementation
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
 * \brief Declaration of the conf_file class used to read a configuration file.
 *
 * The library offers a way to read a configuration file parameters without
 * doing anything more than that. The getopt class uses it to read and verify
 * the parameters. It also uses it to read files of configuration options.
 */

// C++ lib
//
#include    <fstream>
#include    <map>
#include    <memory>


namespace advgetopt
{



enum class line_continuation_t
{
    single_line,
    rfc_822,        // like email/HTTP, whitespace on next line
    msdos,          // '&' at end of line
    unix,           // '\' at end of line
    fortran,        // '&' at start of next line
    semicolon       // ';' ends the _line_
};


typedef std::uint_fast16_t      comment_t;

constexpr comment_t             COMMENT_INI                 = 0x0001;       // ; comment
constexpr comment_t             COMMENT_SHELL               = 0x0002;       // # comment
constexpr comment_t             COMMENT_CPP                 = 0x0004;       // // comment


typedef std::uint_fast16_t      assignment_operator_t;

constexpr assignment_operator_t ASSIGNMENT_OPERATOR_EQUAL   = 0x0001;       // a = b
constexpr assignment_operator_t ASSIGNMENT_OPERATOR_COLON   = 0x0002;       // a: b
constexpr assignment_operator_t ASSIGNMENT_OPERATOR_SPACE   = 0x0004;       // a b


typedef std::uint_fast16_t      section_operator_t;

constexpr section_operator_t    SECTION_OPERATOR_C          = 0x0001;       // a.b
constexpr section_operator_t    SECTION_OPERATOR_CPP        = 0x0002;       // a::b
constexpr section_operator_t    SECTION_OPERATOR_BLOCK      = 0x0004;       // a { ... }
constexpr section_operator_t    SECTION_OPERATOR_INI_FILE   = 0x0008;       // [a]



class conf_file
{
public:
    typedef std::shared_ptr<conf_file>              pointer_t;
    typedef std::map<std::string, bool>             sections_t;
    typedef std::map<std::string, std::string>      parameters_t;

                                conf_file(std::string const & filename
                                        , line_continuation_t line_continuation = line_continuation_t::unix
                                        , assignment_operator_t assignment_operator = ASSIGNMENT_OPERATOR_EQUAL
                                        , comment_t comment = COMMENT_INI | COMMENT_SHELL);

    std::string const &         get_filename() const;
    int                         get_errno() const;
    sections_t const &          get_sections() const;
    parameters_t const &        get_parameters() const;
    bool                        has_parameter(std::string const & name) const;
    std::string                 get_parameter(std::string const & name) const;

    bool                        is_assignment_operator(int c) const;
    bool                        is_comment(char const * s) const;

private:
    int                         getc(std::ifstream & stream);
    void                        ungetc(int c);
    bool                        get_line(std::ifstream & stream, std::string & line);
    void                        read_configuration();

    std::string                 f_filename = std::string();
    int                         f_unget_char = '\0';
    int                         f_line = 0;
    line_continuation_t         f_line_continuation = line_continuation_t::unix;
    assignment_operator_t       f_assignment_operator = ASSIGNMENT_OPERATOR_EQUAL;
    comment_t                   f_comment = COMMENT_INI | COMMENT_SHELL;
    int                         f_errno = 0;
    sections_t                  f_sections = sections_t();
    parameters_t                f_parameters = parameters_t();
};


bool iswspace(int c);


}   // namespace advgetopt
// vim: ts=4 sw=4 et
