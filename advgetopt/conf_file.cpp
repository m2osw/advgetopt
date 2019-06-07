/*
 * File:
 *    advgetopt/conf_file.cpp -- a replacement to the Unix getopt() implementation
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


/** \file
 * \brief Implementation of the option_info class.
 *
 * This is the implementation of the class used to define one command
 * line option.
 */

// self
//
#include "advgetopt/conf_file.h"


// advgetopt lib
//
#include "advgetopt/exception.h"
#include "advgetopt/log.h"


// C++ lib
//
#include <algorithm>
#include <fstream>


// last include
//
#include <snapdev/poison.h>



namespace advgetopt
{



conf_file::conf_file(std::string const & filename
                   , line_continuation_t line_continuation
                   , assignment_operator_t assignment_operator
                   , comment_t comment)
    : f_filename(filename)
    , f_line_continuation(line_continuation)
    , f_assignment_operator(assignment_operator)
    , f_comment(comment)
{
    read_configuration();
}


std::string const & conf_file::get_filename() const
{
    return f_filename;
}


int conf_file::get_errno() const
{
    return f_errno;
}


conf_file::sections_t const & conf_file::get_sections() const
{
    return f_sections;
}


conf_file::parameters_t const & conf_file::get_parameters() const
{
    return f_parameters;
}


bool conf_file::has_parameter(std::string const & name) const
{
    auto it(f_parameters.find(name));
    return it != f_parameters.end();
}


std::string conf_file::get_parameter(std::string const & name) const
{
    auto it(f_parameters.find(name));
    if(it != f_parameters.end())
    {
        return it->second;
    }
    return std::string();
}


int conf_file::getc(std::ifstream & in)
{
    if(f_unget_char != '\0')
    {
        f_unget_char = '\0';
        return f_unget_char;
    }

    char c;
    in.get(c);

    if(in.eof())
    {
        return EOF;
    }

    return c;
}


void conf_file::ungetc(int c)
{
    if(f_unget_char == '\0')
    {
        throw getopt_exception_logic("conf_file::ungetc() called when the f_unget_char variable member is not '\\0'.");
    }
    f_unget_char = c;
}


bool conf_file::get_line(std::ifstream & in, std::string & line)
{
    line.clear();

    for(;;)
    {
        int c(getc(in));
        if(c == EOF)
        {
            return false;
        }
        if(c == ';'
        && f_line_continuation == line_continuation_t::semicolon)
        {
            return true;
        }

        while(c == '\n' || c == '\r')
        {
            if(c == '\r')
            {
                c = getc(in);
                if(c != '\n')
                {
                    ungetc(c);
                }
                c = '\n';
            }

            ++f_line;
            switch(f_line_continuation)
            {
            case line_continuation_t::single_line:
                // continuation support
                return true;

            case line_continuation_t::rfc_822:
                c = getc(in);
                if(!iswspace(c))
                {
                    ungetc(c);
                    return true;
                }
                do
                {
                    c = getc(in);
                }
                while(iswspace(c));
                break;

            case line_continuation_t::msdos:
                if(line.empty()
                || line.back() != '&')
                {
                    return true;
                }
                line.pop_back();
                break;

            case line_continuation_t::unix:
                if(line.empty()
                || line.back() != '\\')
                {
                    return true;
                }
                line.pop_back();
                break;

            case line_continuation_t::fortran:
                c = getc(in);
                if(c != '&')
                {
                    ungetc(c);
                    return true;
                }
                break;

            case line_continuation_t::semicolon:
                line += c;
                c = getc(in);
                break;

            }
        }

        line += c;
    }
}


/** \brief Read a configuration file.
 *
 * This function reads a configuration file and saves all the parameters it
 * finds in a map which can later be checked against an option table for
 * validation.
 *
 * \todo
 * Add support for quotes in configuration files as parameters are otherwise
 * saved as a separated list of parameters losing the number of spaces between
 * each entry.
 */
void conf_file::read_configuration()
{
    std::ifstream conf(f_filename);
    if(!conf)
    {
        f_errno = errno;
        return;
    }

    std::string current_section;
    std::vector<std::string> sections;
    std::string str;
    f_line = 0;
    while(get_line(conf, str))
    {
        char const * s(str.c_str());
        while(iswspace(*s))
        {
            ++s;
        }
        if(*s == '\0'
        || is_comment(s))
        {
            // skip empty lines and comments
            continue;
        }
        if(*s == '}')
        {
            current_section = sections.back();
            sections.pop_back();
            continue;
        }
        char const * str_name(s);
        char const * e(nullptr);
        while(!is_assignment_operator(*s) && *s != '{' && *s != '\0')
        {
            if(iswspace(*s))
            {
                e = s;
                while(iswspace(*s))
                {
                    ++s;
                }
                if(*s != '\0' && !is_assignment_operator(*s))
                {
                    log << log_level_t::error
                        << "option name from \""
                        << str
                        << "\" on line "
                        << f_line
                        << " in configuration file \""
                        << f_filename
                        << "\" cannot include a space, missing assignment operator?"
                        << end;
                }
                break;
            }
            else
            {
                ++s;
            }
        }
        if(e == nullptr)
        {
            e = s;
        }
        if(e - str_name == 0)
        {
            log << log_level_t::error
                << "no option name in \""
                << str
                << "\" on line "
                << f_line
                << " from configuration file \""
                << f_filename
                << "\", missing name before = sign?"
                << end;
            continue;
        }
        std::string name(str_name, e - str_name);
        std::replace(name.begin(), name.end(), '_', '-');
        if(name[0] == '-')
        {
            log << log_level_t::error
                << "option names in configuration files cannot start with a dash or an underscore in \""
                << str
                << "\" on line "
                << f_line
                << " from configuration file \""
                << f_filename
                << "\"."
                << end;
            continue;
        }
        if(name.length() >= 2
        && name[0] == '['
        && name.back() == ']')
        {
            if(!sections.empty())
            {
                log << log_level_t::error
                    << "`[...]` sections can't be used within a `section { ... }` is not supported on line "
                    << f_line
                    << " from configuration file \""
                    << f_filename
                    << "\"."
                    << end;
                continue;
            }
            while(iswspace(*s))
            {
                ++s;
            }
            if(*s != '\0')
            {
                log << log_level_t::error
                    << "section names in configuration files cannot be followed by anything other than spaces in \""
                    << str
                    << "\" on line "
                    << f_line
                    << " from configuration file \""
                    << f_filename
                    << "\"."
                    << end;
                continue;
            }
            if(name.length() == 2)
            {
                // "[]" removes the section
                //
                current_section.clear();
            }
            else
            {
                current_section = name.substr(1, name.length() - 2);
                f_sections[current_section] = true;
                current_section += "::";
            }
        }
        else if(*s == '{')
        {
            sections.push_back(current_section);
            f_sections[current_section + name] = true;
            current_section = name + "::";
        }
        else
        {
            if(is_assignment_operator(*s))
            {
                ++s;
            }
            while(iswspace(*s))
            {
                ++s;
            }
            for(e = str.c_str() + str.length(); e > s; --e)
            {
                if(!iswspace(e[-1]))
                {
                    break;
                }
            }
            size_t const len(e - s);

            // TODO: add a set_parameter() which verifies that the name is
            //       considered valid
            //
            f_parameters[current_section + name] = std::string(s, len);
        }
    }
    if(!sections.empty())
    {
        log << log_level_t::error
            << "unterminated `section { ... }`, the `}` is missing in configuration file \""
            << f_filename
            << "\"."
            << end;
    }
}


/** \brief Check whether `c` is an assignment operator.
 *
 * This function checks the \p c parameter to know whether it matches
 * one of the character allowed as an assignment character.
 *
 * \param[in] c  The character to be checked.
 *
 * \return true if c is considered to represent an assignment character.
 */
bool conf_file::is_assignment_operator(int c) const
{
    return ((f_assignment_operator & ASSIGNMENT_OPERATOR_EQUAL) != 0 && c == '=')
        || ((f_assignment_operator & ASSIGNMENT_OPERATOR_COLON) != 0 && c == ':')
        || ((f_assignment_operator & ASSIGNMENT_OPERATOR_SPACE) != 0 && std::iswspace(c));
}


/** \brief Check whether the string starts with a comment introducer.
 *
 * This function checks whether the \p s string starts with a comment.
 *
 * We support different types of comment introducers. This function
 * checks the flags as defined in the constructor and returns true
 * if the type of character introducer defines a comment.
 *
 * We currently support:
 *
 * \li .ini file comments, introduced by a semi-colon (;)
 *
 * \li Shell file comments, introduced by a hash character (#)
 *
 * \li C++ comment, introduced by two slashes (//)
 *
 * \param[in] s  The string to check for a comment.
 *
 * \return `true` if the string represents a comment.
 */
bool conf_file::is_comment(char const * s) const
{
    if((f_comment & COMMENT_INI) != 0
    && *s == ';')
    {
        return true;
    }

    if((f_comment & COMMENT_SHELL) != 0
    && *s == '#')
    {
        return true;
    }

    if((f_comment & COMMENT_CPP) != 0
    && s[0] == '/'
    && s[1] == '/')
    {
        return true;
    }

    return false;
}


/** \brief Returns true if \p c is considered to be a whitespace.
 *
 * Our iswspace() function is equivalent to the std::iswspace() function
 * except that `'\\r'` and `'\\n'` are never viewed as white spaces.
 *
 * \return true if c is considered to be a white space character.
 */
bool iswspace(int c)
{
    return c != '\n'
        && c != '\r'
        && std::iswspace(c);
}


}   // namespace advgetopt
// vim: ts=4 sw=4 et
