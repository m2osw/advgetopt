// Copyright (c) 2006-2020  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/
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


/** \file
 * \brief Implementation of utility functions.
 *
 * This file includes various utility functions that are not specifically
 * attached to a class.
 */

// self
//
#include    "advgetopt/utils.h"


// snapdev lib
//
#include    <snapdev/glob_to_list.h>
#include    <snapdev/not_used.h>


// cppthread lib
//
#include    <cppthread/guard.h>
#include    <cppthread/mutex.h>


// boost lib
//
#include    <boost/algorithm/string/trim.hpp>


// C++ lib
//
#include    <set>


// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{



namespace
{



/** \brief The configuration file mutex.
 *
 * This options are generally viewed as read-only global variables. They
 * get setup once early on and then used and reused as many times as
 * required.
 *
 * This mutex makes sure that access between multiple thread happens in
 * a safe manner.
 */
cppthread::mutex *      g_mutex;



}
// no name namespace



/** \brief Get a global mutex.
 *
 * This function returns a global mutex we can use to lock the advgetopt
 * whenever multithread functionality is required (i.e. a global is used.)
 *
 * It is safe to call this function early (i.e. before main was ever
 * called.)
 *
 * Usage:
 *
 * \code
 *    cppthread::guard lock(get_global_mutex());
 * \endcode
 *
 * \return A reference to our global mutex.
 */
cppthread::mutex & get_global_mutex()
{
    {
        cppthread::guard lock(*cppthread::g_system_mutex);

        if(g_mutex == nullptr)
        {
            g_mutex = new cppthread::mutex();
        }
    }

    return *g_mutex;
}



/** \brief Remove single (') or double (") quotes from a string.
 *
 * If a string starts and ends with the same quotation mark, then it
 * gets removed.
 *
 * If no quotes appear, then the function returns a copy of the input as is.
 *
 * The \p pairs parameter must have an even size (or the last character
 * gets ignored). By default, it is set to the double and single quotes:
 *
 * \code
 *     "\"\"''"
 * \endcode
 *
 * To remove square, angle, curly brackets:
 *
 * \code
 *     "[]<>{}"
 * \endcode
 *
 * \todo
 * Add support for UTF-8 quotes. Right now only quotes of 1 byte will
 * work.
 *
 * \param[in] s  The string to unquote.
 * \param[in] pairs  A list of accepted quotes.
 *
 * \return The unquoted string.
 */
std::string unquote(std::string const & s, std::string const & pairs)
{
    if(s.length() >= 2)
    {
        std::string::size_type const max(pairs.length() - 1);
        for(std::string::size_type pos(0); pos < max; pos += 2)
        {
            if(s.front() == pairs[pos + 0]
            && s.back()  == pairs[pos + 1])
            {
                return s.substr(1, s.length() - 2);
            }
        }
    }

    return s;
}


/** \brief Split a string in sub-strings separated by \p separators.
 *
 * This function searches for any of the \p separators in \p str and
 * split at those locations.
 *
 * For example, to split a comma separated list of strings, use the
 * following:
 *
 * \code
 *     string_list_t result;
 *     option_info::split_string(string_to_split, result, {","});
 * \endcode
 *
 * If `string_to_split` is set to "a, b, c", then the `result` vector
 * will have three strings as a result: `a`, `b`, and `c`. Note that
 * the function automatically trims all strings and it never keeps
 * empty strings. So two separators one after another is accepted and
 * no empty string results.
 *
 * The trimming happens after the split occurs. This allows for the
 * list of separators to include spaces as separators.
 *
 * The function does not clear the result vector. This allows you to
 * call this function multiple times with various strings and the
 * results will be cumulated.
 *
 * \note
 * This function is a static so it can be used from anywhere to split
 * strings as required. You do not need to have an option_info instance.
 *
 * \todo
 * See to fix the fact that `a"b"c` becomes `{"a", "b", "c"}` when
 * there are not separators between `a`, `"b"`, and `c`. To the minimum
 * we may want to generate an error when such is found (i.e. when a
 * quote is found and `start < pos` is true.
 *
 * \param[in] str  The string to split.
 * \param[in] result  The vector where the split strings are saved.
 * \param[in] separators  The vector of strings used as separators.
 */
void split_string(std::string const & str
                , string_list_t & result
                , string_list_t const & separators)
{
    std::string::size_type pos(0);
    std::string::size_type start(0);
    while(pos < str.length())
    {
        if(str[pos] == '\'' || str[pos] == '"')
        {
            if(start < pos)
            {
                std::string v(str.substr(start, pos - start));
                boost::trim(v);
                if(!v.empty())
                {
                    result.push_back(v);
                }
                start = pos;
            }

            // quoted parameters are handled without the separators
            //
            char const quote(str[pos]);
            for(++pos; pos < str.length() && str[pos] != quote; ++pos);

            std::string const v(str.substr(start + 1, pos - (start + 1)));
            if(!v.empty())
            {
                result.push_back(v);
            }
            if(pos < str.length())
            {
                // skip the closing quote
                //
                ++pos;
            }
            start = pos;
        }
        else
        {
            bool found(false);
            for(auto const & sep : separators)
            {
                if(str.length() - pos >= sep.length())
                {
                    if(str.compare(pos, sep.length(), sep) == 0)
                    {
                        // match! cut here
                        //
                        if(start < pos)
                        {
                            std::string v(str.substr(start, pos - start));
                            boost::trim(v);
                            if(!v.empty())
                            {
                                result.push_back(v);
                            }
                        }
                        pos += sep.length();
                        start = pos;
                        found = true;
                        break;
                    }
                }
            }

            if(!found)
            {
                ++pos;
            }
        }
    }

    if(start < pos)
    {
        std::string v(str.substr(start, pos - start));
        boost::trim(v);
        if(!v.empty())
        {
            result.push_back(v);
        }
    }
}


/** \brief Insert the group (or project) name in the filename.
 *
 * This function inserts the name of the group in the specified full path
 * filename. It gets added right before the basename. So for example you
 * have a path such as:
 *
 *     /etc/snapwebsites/advgetopt.conf
 *
 * and a group name such as:
 *
 *     adventure
 *
 * The resulting path is:
 *
 *     /etc/snapwebsites/adventure.d/advgetopt.conf
 *
 * Notice that the function adds a ".d" as well.
 *
 * If the group name is empty or null, then the project name is used. If
 * both are empty, then nothing happens (the function returns an empty list).
 *
 * \param[in] filename  The filename where the project name gets injected.
 * \param[in] group_name  The name of the group to inject in the filename.
 * \param[in] project_name  The name of the project to inject in the filename.
 *
 * \return The list of filenames or an empty list if no group or project name
 *         or filename are specified.
 */
string_list_t insert_group_name(
          std::string const & filename
        , char const * group_name
        , char const * project_name)
{
    if(filename.empty())
    {
        return string_list_t();
    }

    std::string name;
    if(group_name == nullptr
    || *group_name == '\0')
    {
        if(project_name == nullptr
        || *project_name == '\0')
        {
            return string_list_t();
        }
        name = project_name;
    }
    else
    {
        name = group_name;
    }

    std::string pattern;
    std::string::size_type const pos(filename.find_last_of('/'));
    if(pos != std::string::npos
    && pos > 0)
    {
        pattern = filename.substr(0, pos + 1)
                + name
                + ".d/[0-9][0-9]-"
                + filename.substr(pos + 1);
    }
    else
    {
        pattern = name
                + (".d/[0-9][0-9]-" + filename);
    }

    snap::glob_to_list<std::set<std::string>> glob;

    // the glob() function is not thread safe
    {
        cppthread::guard lock(get_global_mutex());
        snap::NOTUSED(glob.read_path<snap::glob_to_list_flag_t::GLOB_FLAG_IGNORE_ERRORS>(pattern));
    }

    // we add the default name if none other exists
    //
    if(glob.empty())
    {
        if(pos != std::string::npos
        && pos > 0)
        {
            glob.insert(filename.substr(0, pos + 1)
                    + name
                    + ".d/50-"
                    + filename.substr(pos + 1));
        }
        else
        {
            glob.insert(name
                    + (".d/50-" + filename));
        }
    }

    return string_list_t(glob.begin(), glob.end());
}


/** \brief Replace a starting `~/...` with the contents of the \$HOME variable.
 *
 * This function checks the beginning of \p filename. If it starts with `'~/'`
 * then it replaces the `'~'` character with the contents of the \$HOME
 * environment variable.
 *
 * If \p filename is just `"~"`, then the function returns the contents of
 * the \$HOME environment variable by itself.
 *
 * If somehow the \$HOME environment variable is empty, the function does
 * nothing.
 *
 * \param[in] filename  The filename to check for a tilde (~).
 *
 * \return The input as is unless the \$HOME path can be prepended to replace
 *         the tilde (~) character.
 */
std::string handle_user_directory(std::string const & filename)
{
    char const * const home(getenv("HOME"));
    if(home != nullptr
    && *home != '\0')
    {
        if(!filename.empty()
        && filename[0] == '~'
        && (filename.length() == 1 || filename[1] == '/'))
        {
            return home + filename.substr(1);
        }
    }

    return filename;
}


/** \brief Check whether a value represents "true".
 *
 * This function checks a string to see whether it is one of:
 *
 * * "true"
 * * "on"
 * * "yes"
 * * "1"
 *
 * If so, then the function returns true.
 *
 * \param[in] s  The string to be checked.
 *
 * \return true if the string represents "true".
 */
bool is_true(std::string s)
{
    return s == "true" || s == "on" || s == "yes" | s == "1";
}


/** \brief Check whether a value represents "false".
 *
 * This function checks a string to see whether it is one of:
 *
 * * "false"
 * * "off"
 * * "no"
 * * "0"
 *
 * If so, then the function returns true.
 *
 * \param[in] s  The string to be checked.
 *
 * \return true if the string represents "false".
 */
bool is_false(std::string s)
{
    return s == "false" || s == "off" || s == "no" || s == "0";
}



}   // namespace advgetopt
// vim: ts=4 sw=4 et
