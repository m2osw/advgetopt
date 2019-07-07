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
#include "advgetopt/utils.h"


// snapdev lib
//
#include <snapdev/tokenize_string.h>


// boost lib
//
#include <boost/algorithm/string/join.hpp>


// C++ lib
//
#include <algorithm>
#include <fstream>


// last include
//
#include <snapdev/poison.h>



namespace advgetopt
{



namespace
{



typedef std::map<std::string, conf_file::pointer_t>     conf_file_map_t;

conf_file_map_t     g_conf_files = conf_file_map_t();


class conf_mutex
{
public:
    conf_mutex()
    {
        pthread_mutexattr_t mattr;
        int err(pthread_mutexattr_init(&mattr));
        if(err != 0)
        {
            throw getopt_exception_initialization("pthread_muteattr_init() failed"); // LCOV_EXCL_LINE
        }
        err = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
        if(err != 0)
        {
            pthread_mutexattr_destroy(&mattr);                                          // LCOV_EXCL_LINE
            throw getopt_exception_initialization("pthread_muteattr_settype() failed"); // LCOV_EXCL_LINE
        }
        err = pthread_mutex_init(&f_mutex, &mattr);
        if(err != 0)
        {
            pthread_mutexattr_destroy(&mattr);                                    // LCOV_EXCL_LINE
            throw getopt_exception_initialization("pthread_mutex_init() failed"); // LCOV_EXCL_LINE
        }
        err = pthread_mutexattr_destroy(&mattr);
        if(err != 0)
        {
            throw getopt_exception_initialization("pthread_mutexattr_destroy() failed"); // LCOV_EXCL_LINE
        }
    }

    ~conf_mutex()
    {
        pthread_mutex_destroy(&f_mutex);
    }

    void lock()
    {
        int const err(pthread_mutex_lock(&f_mutex));
        if(err != 0)
        {
            throw getopt_exception_invalid("pthread_mutex_lock() failed"); // LCOV_EXCL_LINE
        }
    }

    void unlock()
    {
        int const err(pthread_mutex_unlock(&f_mutex));
        if(err != 0)
        {
            throw getopt_exception_invalid("pthread_mutex_unlock() failed"); // LCOV_EXCL_LINE
        }
    }

private:
    pthread_mutex_t     f_mutex = pthread_mutex_t();
};

conf_mutex g_mutex;

class safe_lock
{
public:
    safe_lock(conf_mutex & m)
        : f_mutex(m)
    {
        f_mutex.lock();
    }

    ~safe_lock()
    {
        f_mutex.unlock();
    }

private:
    conf_mutex &        f_mutex;
};



} // no name namespace



conf_file_setup::conf_file_setup(
          std::string const & filename
        , line_continuation_t line_continuation
        , assignment_operator_t assignment_operator
        , comment_t comment
        , section_operator_t section_operator)
    : f_line_continuation(line_continuation)
    , f_assignment_operator(assignment_operator == 0
                ? ASSIGNMENT_OPERATOR_EQUAL
                : assignment_operator)
    , f_comment(comment)
    , f_section_operator(section_operator)
{
    if(filename.empty())
    {
        throw getopt_exception_invalid("trying to load a configuration file using an empty filename.");
    }

    std::unique_ptr<char, decltype(&::free)> fn(realpath(filename.c_str(), nullptr), &::free);
    if(fn != nullptr)
    {
        f_filename = fn.get();
    }
}


bool conf_file_setup::is_valid() const
{
    return !f_filename.empty();
}


std::string const & conf_file_setup::get_filename() const
{
    return f_filename;
}


line_continuation_t conf_file_setup::get_line_continuation() const
{
    return f_line_continuation;
}


assignment_operator_t conf_file_setup::get_assignment_operator() const
{
    return f_assignment_operator;
}


comment_t conf_file_setup::get_comment() const
{
    return f_comment;
}


section_operator_t conf_file_setup::get_section_operator() const
{
    return f_section_operator;
}


std::string conf_file_setup::get_config_url() const
{
    if(f_url.empty())
    {
        std::stringstream ss;

        ss << "file://"
           << (f_filename.empty()
                    ? "/<empty>"
                    : f_filename);

        std::vector<std::string> params;
        if(f_line_continuation != line_continuation_t::unix)
        {
            std::string name;
            switch(f_line_continuation)
            {
            case line_continuation_t::single_line:
                name = "single-line";
                break;

            case line_continuation_t::rfc_822:
                name = "rfc-822";
                break;

            case line_continuation_t::msdos:
                name = "msdos";
                break;

            // we should not ever receive this one since we don't enter
            // this block when the value is "unix"
            //
            //case line_continuation_t::unix:
            //    name = "unix";
            //    break;

            case line_continuation_t::fortran:
                name = "fortran";
                break;

            case line_continuation_t::semicolon:
                name = "semi-colon";
                break;

            default:
                throw getopt_exception_logic("unexpected line continuation.");

            }
            params.push_back("line-continuation=" + name);
        }

        if(f_assignment_operator != ASSIGNMENT_OPERATOR_EQUAL)
        {
            std::vector<std::string> assignments;
            if((f_assignment_operator & ASSIGNMENT_OPERATOR_EQUAL) != 0)
            {
                assignments.push_back("equal");
            }
            if((f_assignment_operator & ASSIGNMENT_OPERATOR_COLON) != 0)
            {
                assignments.push_back("colon");
            }
            if((f_assignment_operator & ASSIGNMENT_OPERATOR_SPACE) != 0)
            {
                assignments.push_back("space");
            }
            if(!assignments.empty())
            {
                params.push_back("assignment-operator=" + boost::algorithm::join(assignments, ","));
            }
        }

        if(f_comment != COMMENT_INI | COMMENT_SHELL)
        {
            std::vector<std::string> comment;
            if((f_comment & COMMENT_INI) != 0)
            {
                comment.push_back("ini");
            }
            if((f_comment & COMMENT_SHELL) != 0)
            {
                comment.push_back("shell");
            }
            if((f_comment & COMMENT_CPP) != 0)
            {
                comment.push_back("cpp");
            }
            if(comment.empty())
            {
                params.push_back("comment=none");
            }
            else
            {
                params.push_back("comment=" + boost::algorithm::join(comment, ","));
            }
        }

        if(f_section_operator != SECTION_OPERATOR_INI_FILE)
        {
            std::vector<std::string> section_operator;
            if((f_section_operator & SECTION_OPERATOR_C) != 0)
            {
                section_operator.push_back("c");
            }
            if((f_section_operator & SECTION_OPERATOR_CPP) != 0)
            {
                section_operator.push_back("cpp");
            }
            if((f_section_operator & SECTION_OPERATOR_BLOCK) != 0)
            {
                section_operator.push_back("block");
            }
            if((f_section_operator & SECTION_OPERATOR_INI_FILE) != 0)
            {
                section_operator.push_back("ini-file");
            }
            if(!section_operator.empty())
            {
                params.push_back("section-operator=" + boost::algorithm::join(section_operator, ","));
            }
        }

        std::string const query_string(boost::algorithm::join(params, "&"));
        if(!query_string.empty())
        {
            ss << '?'
               << query_string;
        }

        f_url = ss.str();
    }

    return f_url;
}




/** \brief Create and read a conf_file.
 *
 * This function creates a new conf_file object unless one with the same
 * filename already exists.
 *
 * If the configuration file was already loaded, then that pointer gets
 * returned instead of reloading the file. There is currently no API to
 * allow for the removal because another thread or function may have
 * the existing pointer cached and we want all instances of a configuration
 * file to be the same (i.e. if you update the value of a parameter then
 * that new value should be visible by all the users of that configuration
 * file.) Therefore, you can think of a configuration file as a global
 * variable.
 */
conf_file::pointer_t conf_file::get_conf_file(conf_file_setup const & setup)
{
    safe_lock lock(g_mutex);

    auto it(g_conf_files.find(setup.get_filename()));
    if(it != g_conf_files.end())
    {
        if(it->second->get_setup().get_config_url() != setup.get_config_url())
        {
            throw getopt_exception_logic("trying to load configuration file \""
                                       + setup.get_config_url()
                                       + "\" but an existing configuration file with the same name was loaded with URL: \""
                                       + it->second->get_setup().get_config_url()
                                       + "\".");
        }
        return it->second;
    }
    conf_file::pointer_t cf(new conf_file(setup));
    g_conf_files[setup.get_filename()] = cf;
    return cf;
}


/** \brief Initialize and read a configuration file.
 *
 * This constructor initializes this conf_file object and then reads the
 * corresponding configuration file.
 *
 * Note that you have to use the create_conf_file() function for you
 * to be able to create a configuration file. It is done that way became
 * a file can be read only once. Once loaded, it gets cached until your
 * application quits.
 *
 * \param[in] filename  The path and name of the configuration file to be read.
 * \param[in] line_continuation  How lines end in this file.
 * \param[in] assignment_operator  What appears between the name and value.
 * \param[in] comment  The supported comment introducer(s).
 */
conf_file::conf_file(conf_file_setup const & setup)
    : f_setup(setup)
{
    read_configuration();
}


/** \brief Get the configuration file setup.
 *
 * This function returns a copy of the setup used to load this
 * configuration file.
 *
 * \note
 * This function has no mutex protection because the setup can't
 * change so there is no multi-thread protection necessary (the
 * fact that you hold a shared pointer to the conf_file object
 * is enough protection in this case.)
 *
 * \return A reference to this configuration file setup.
 */
conf_file_setup const & conf_file::get_setup() const
{
    return f_setup;
}


int conf_file::get_errno() const
{
    safe_lock lock(g_mutex);

    return f_errno;
}


conf_file::sections_t conf_file::get_sections() const
{
    safe_lock lock(g_mutex);

    return f_sections;
}


conf_file::parameters_t conf_file::get_parameters() const
{
    safe_lock lock(g_mutex);

    return f_parameters;
}


bool conf_file::has_parameter(std::string const & name) const
{
    safe_lock lock(g_mutex);

    auto it(f_parameters.find(name));
    return it != f_parameters.end();
}


std::string conf_file::get_parameter(std::string const & name) const
{
    safe_lock lock(g_mutex);

    auto it(f_parameters.find(name));
    if(it != f_parameters.end())
    {
        return it->second;
    }
    return std::string();
}


bool conf_file::set_parameter(std::string const & section, std::string const & name, std::string const & value)
{
    // use the tokenize_string() function because we do not want to support
    // quoted strings in this list of sections which our split_string()
    // does automatically
    //
    string_list_t section_list;
    snap::tokenize_string(section_list
                        , section
                        , "::"
                        , true
                        , std::string()
                        , &snap::string_predicate<string_list_t>);

    char const * n(name.c_str());

    // global scope? if so ignore the section_list (clear it)
    //
    if((f_setup.get_section_operator() & SECTION_OPERATOR_CPP) != 0
    && n[0] == ':'
    && n[1] == ':')
    {
        section_list.clear();
        do
        {
            ++n;
        }
        while(*n == ':');
    }

    char const * s(n);
    while(*n != '\0')
    {
        if((f_setup.get_section_operator() & SECTION_OPERATOR_C) != 0
        && *n == '.')
        {
            if(s == n)
            {
                log << log_level_t::error
                    << "option name \""
                    << name
                    << "\" cannot start with a period (.)."
                    << end;
                return false;
            }
            section_list.push_back(std::string(s, n - s));
            do
            {
                ++n;
            }
            while(*n == '.');
            s = n;
        }
        else if((f_setup.get_section_operator() & SECTION_OPERATOR_CPP) != 0
             && n[0] == ':'
             && n[1] == ':')
        {
            if(s == n)
            {
                log << log_level_t::error
                    << "option name \""
                    << name
                    << "\" cannot start with a scope operator (::)."
                    << end;
                return false;
            }
            section_list.push_back(std::string(s, n - s));
            do
            {
                ++n;
            }
            while(*n == ':');
            s = n;
        }
        else
        {
            ++n;
        }
    }
    if(s == n)
    {
        log << log_level_t::error
            << "option name \""
            << name
            << "\" cannot end with a section operator or be empty."
            << end;
        return false;
    }
    std::string param_name(s, n - s);

    std::string const section_name(boost::algorithm::join(section_list, "::"));

    if(f_setup.get_section_operator() == SECTION_OPERATOR_NONE
    && !section_list.empty())
    {
        log << log_level_t::error
            << "option name \""
            << name
            << "\" cannot be added to section \""
            << section_name
            << "\" because there is no section support for this configuration file."
            << end;
        return false;
    }
    if((f_setup.get_section_operator() & SECTION_OPERATOR_ONE_SECTION) != 0
    && section_list.size() > 1)
    {
        log << log_level_t::error
            << "option name \""
            << name
            << "\" cannot be added to section \""
            << section_name
            << "\" because this configuration only accepts one section level."
            << end;
        return false;
    }

    // add the section to the list of sections
    //
    // TODO: should we have a list of all the parent sections? Someone can
    //       write "a::b::c::d = 123" and we currently only get section
    //       "a::b::c", no section "a" and no section "a::b".
    //
    if(!section_name.empty())
    {
        f_sections.insert(section_name);
    }

    section_list.push_back(param_name);
    std::string const full_name(boost::algorithm::join(section_list, "::"));

    safe_lock lock(g_mutex);

    f_parameters[full_name] = value;

    return true;
}


int conf_file::getc(std::ifstream & in)
{
    if(f_unget_char != '\0')
    {
        int const r(f_unget_char);
        f_unget_char = '\0';
        return r;
    }

    char c;
    in.get(c);

    if(in.eof())
    {
        return EOF;
    }

    return static_cast<std::uint8_t>(c);
}


void conf_file::ungetc(int c)
{
    if(f_unget_char != '\0')
    {
        throw getopt_exception_logic("conf_file::ungetc() called when the f_unget_char variable member is not '\\0'."); // LCOV_EXCL_LINE
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
        && f_setup.get_line_continuation() == line_continuation_t::semicolon)
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
            switch(f_setup.get_line_continuation())
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
                c = getc(in);
                break;

            case line_continuation_t::unix:
                if(line.empty()
                || line.back() != '\\')
                {
                    return true;
                }
                line.pop_back();
                c = getc(in);
                break;

            case line_continuation_t::fortran:
                c = getc(in);
                if(c != '&')
                {
                    ungetc(c);
                    return true;
                }
                c = getc(in);
                break;

            case line_continuation_t::semicolon:
                // if we have a comment, we want to return immediately;
                // at this time, the comments are not multi-line so
                // the call can return true only if we were reading the
                // very first line
                //
                if(is_comment(line.c_str()))
                {
                    return true;
                }
                // the semicolon is checked earlier, just keep the newline
                // in this case
                //
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
    std::ifstream conf(f_setup.get_filename());
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
        if((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) != 0
        && *s == '}')
        {
            current_section = sections.back();
            sections.pop_back();
            continue;
        }
        char const * str_name(s);
        char const * e(nullptr);
        while(!is_assignment_operator(*s)
           && ((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) == 0 || (*s != '{' && *s != '}'))
           && ((f_setup.get_section_operator() & SECTION_OPERATOR_INI_FILE) == 0 || *s != ']')
           && *s != '\0'
           && !iswspace(*s))
        {
            ++s;
        }
        if(iswspace(*s))
        {
            e = s;
            while(iswspace(*s))
            {
                ++s;
            }
            if(*s != '\0'
            && !is_assignment_operator(*s)
            && (f_setup.get_assignment_operator() & ASSIGNMENT_OPERATOR_SPACE) == 0
            && ((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) == 0 || (*s != '{' && *s != '}')))
            {
                log << log_level_t::error
                    << "option name from \""
                    << str
                    << "\" on line "
                    << f_line
                    << " in configuration file \""
                    << f_setup.get_filename()
                    << "\" cannot include a space, missing assignment operator?"
                    << end;
                continue;
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
                << f_setup.get_filename()
                << "\", missing name before the assignment operator?"
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
                << f_setup.get_filename()
                << "\"."
                << end;
            continue;
        }
        if((f_setup.get_section_operator() & SECTION_OPERATOR_INI_FILE) != 0
        && name.length() >= 1
        && name[0] == '['
        && *s == ']')
        {
            ++s;
            if(!sections.empty())
            {
                log << log_level_t::error
                    << "`[...]` sections can't be used within a `section { ... }` on line "
                    << f_line
                    << " from configuration file \""
                    << f_setup.get_filename()
                    << "\"."
                    << end;
                continue;
            }
            while(iswspace(*s))
            {
                ++s;
            }
            if(*s != '\0'
            && !is_comment(s))
            {
                log << log_level_t::error
                    << "section names in configuration files cannot be followed by anything other than spaces in \""
                    << str
                    << "\" on line "
                    << f_line
                    << " from configuration file \""
                    << f_setup.get_filename()
                    << "\"."
                    << end;
                continue;
            }
            if(name.length() == 1)
            {
                // "[]" removes the section
                //
                current_section.clear();
            }
            else
            {
                current_section = name.substr(1);
                current_section += "::";
            }
        }
        else if((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) != 0
             && *s == '{')
        {
            sections.push_back(current_section);
            current_section += name;
            current_section += "::";
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
            set_parameter(current_section, name, std::string(s, len));
        }
    }
    if(!sections.empty())
    {
        log << log_level_t::error
            << "unterminated `section { ... }`, the `}` is missing in configuration file \""
            << f_setup.get_filename()
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
    assignment_operator_t const assignment_operator(f_setup.get_assignment_operator());
    return ((assignment_operator & ASSIGNMENT_OPERATOR_EQUAL) != 0 && c == '=')
        || ((assignment_operator & ASSIGNMENT_OPERATOR_COLON) != 0 && c == ':')
        || ((assignment_operator & ASSIGNMENT_OPERATOR_SPACE) != 0 && std::iswspace(c));
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
    comment_t const comment(f_setup.get_comment());
    if((comment & COMMENT_INI) != 0
    && *s == ';')
    {
        return true;
    }

    if((comment & COMMENT_SHELL) != 0
    && *s == '#')
    {
        return true;
    }

    if((comment & COMMENT_CPP) != 0
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
