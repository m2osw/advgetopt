// Copyright (c) 2006-2023  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief Implementation of the options_to_strings() command.
 *
 * The getopt object holds all the command line options your program was
 * started with. Here we transforms these options back to shell command
 * line options so one can start the command again with the same options.
 */

// self
//
#include    "advgetopt/advgetopt.h"


// cppthread
//
#include    <cppthread/log.h>


// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{


namespace
{

constexpr char const g_space = ' ';


} // no name namespace

/** \brief Transform all the defined options back in a string.
 *
 * This function creates a string which system() can use to start the
 * command again with the same options. You may, of course, tweak the
 * options first.
 *
 * \param[in] include_progname  Whether the program name should be included
 * in the output string. In some cases, you may want to start a different
 * program with similar command line options. This gives you that option.
 * \param[in] keep_defaults  If the value is equal to the default value, it
 * gets ignored unless this parameter is set to true.
 *
 * \return The string representing the command line options.
 */
std::string getopt::options_to_string(bool include_progname, bool keep_defaults) const
{
    std::string result;

    if(include_progname)
    {
        result += escape_shell_argument(get_program_fullname());
    }

    advgetopt::option_info::pointer_t default_option;
    for(auto const & opt : f_options_by_name)
    {
        if(!opt.second->is_defined())
        {
            continue;
        }
        if(opt.second->is_default_option())
        {
            default_option = opt.second;
            continue;
        }

        if(!keep_defaults
        && !opt.second->has_flag(advgetopt::GETOPT_FLAG_FLAG)
        && opt.second->get_default() == opt.second->get_value())
        {
            // same as default, no need to add that parameter
            //
            continue;
        }

        if(!result.empty())
        {
            result += g_space;
        }

        result += "--";
        result += opt.second->get_name();

        if(!opt.second->has_flag(advgetopt::GETOPT_FLAG_FLAG))
        {
            result += g_space;
            result += escape_shell_argument(opt.second->get_value());
            std::size_t const max(opt.second->size());
            for(std::size_t idx(1); idx < max; ++idx)
            {
                result += g_space;
                result += escape_shell_argument(opt.second->get_value(idx));
            }
        }
    }

    if(default_option != nullptr)
    {
        result += " -- ";

        result += escape_shell_argument(default_option->get_value());
        std::size_t const max(default_option->size());
        for(std::size_t idx(1); idx < max; ++idx)
        {
            result += g_space;
            result += escape_shell_argument(default_option->get_value(idx));
        }
    }

    return result;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
