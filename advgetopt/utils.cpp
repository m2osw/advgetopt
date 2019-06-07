/*
 * File:
 *    advgetopt/utils.cpp -- a replacement to the Unix getopt() implementation
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
 * \brief Implementation of utility functions.
 *
 * This file includes various utility functions that are not specifically
 * attached to a class.
 */

// self
//
#include "advgetopt/utils.h"


// boost lib
//
#include <boost/algorithm/string/trim.hpp>


// last include
//
#include <snapdev/poison.h>



namespace advgetopt
{



/** \brief Remove single (') or double (") quotes from a string.
 *
 * If a string starts and ends with the same quotation mark, then it
 * gets removed.
 *
 * If no quotes appear, then the function returns a copy of the input as is.
 *
 * \param[in] s  The string to unquote.
 *
 * \return The unquoted string.
 */
std::string unquote(std::string const & s)
{
    if(s.length() >= 2)
    {
        if((s[0] == '"' && s.back() == '"')
        || (s[0] == '\'' && s.back() == '\''))
        {
            return s.substr(1, s.length() - 2);
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
 *     std::vector<std::string> result;
 *     option_info::split_value(string_to_split, result, {","});
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
 * \param[in] str  The string to split.
 * \param[in] result  The vector where the split strings are saved.
 * \param[in] separators  The vector of strings used as separators.
 */
void split_string(std::string const & str
                , string_list_t & result
                , string_list_t const & separators)
{
    std::string::size_type pos(0);
    std::string::size_type start(pos);
    while(pos < str.length())
    {
        if(pos == start
        && (str[pos] == '\'' || str[pos] == '"'))
        {
            // quoted parameters are handled without the separators
            //
            char const quote(str[pos]);
            for(++pos; pos < str.length(); ++pos)
            {
                if(str[pos] == quote)
                {
                    break;
                }
            }

            std::string v(str.substr(start + 1, pos - (start + 1)));
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






}   // namespace advgetopt
// vim: ts=4 sw=4 et
