/*
 * License:
 *    Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Advanced getopt data access implementation.
 *
 * The advgetopt class has many function used to access the data in the
 * class. These functions are gathered here.
 */

// self
//
#include    "advgetopt/advgetopt.h"

// advgetopt lib
//
#include    "advgetopt/exception.h"


// last include
//
#include <snapdev/poison.h>



namespace advgetopt
{


/** \brief Transform the argv[0] parameter in the program name.
 *
 * This function is transforms the first command line argument in a program
 * name. It will define two versions, the basename and the fullname which
 * you can access with the get_program_name() and get_program_fullname()
 * functions.
 *
 * \note
 * The %p and %*p options of the process_help_string() function make use
 * of this parameter. If you never call this function, they both use an
 * empty string as the program name.
 *
 * \exception getopt_exception_logic
 * If you call this function with a null pointer, then it raises this
 * exception.
 *
 * \param[in] argv  The arguments vector.
 *
 * \sa get_program_name()
 * \sa get_program_fullname()
 * \sa process_help_string()
 */
void getopt::parse_program_name(char * argv[])
{
    if(argv == nullptr)
    {
        throw getopt_logic_error("argv pointer cannot be nullptr");
    }
    if(argv[0] != nullptr)
    {
        f_program_fullname = argv[0];
    }

    std::string::size_type p(f_program_fullname.find_last_of('/'));
    if(p == std::string::npos)
    {
        // MS-Windows path uses \ instead of /
        //
        p = f_program_fullname.find_last_of('\\');
    }
    if(p != std::string::npos)
    {
        // remove the path
        //
        f_program_name = f_program_fullname.substr(p + 1);
    }
    else
    {
        f_program_name = f_program_fullname;
    }
}


/** \brief Get the full name of the program.
 *
 * This function return the name of the program exactly as it was passed to
 * the program via argv[0].
 *
 * The reset() function will reset this parameter. If you are creating
 * internal lists of parameters that you want to parse with the same
 * getopt object and your main getopt object, then you may want to
 * consider using this function to define argv[0] of your new list:
 *
 * \code
 * std::vector<std::string> args;
 * args.push_back(my_opts.get_program_fullname());
 * args.push_back("--test");
 * [...]
 * // the following probably require some const_cast<>(), but that's the idea
 * my_opts.reset(args.size(), &args[0], ...);
 * \endcode
 *
 * \return The contents of the argv[0] parameter as defined on construction.
 *
 * \sa parse_program_name()
 */
std::string getopt::get_program_fullname() const
{
    return f_program_fullname;
}


/** \brief Get the basename of the program.
 *
 * This function retrieves the basename, the name of the program with its
 * path trimmed, from this getopt object.
 *
 * This is defined from the argv[0] parameter passed to the constructor or
 * the last reset() call.
 *
 * \return The basename of the program.
 *
 * \sa parse_program_name()
 */
std::string getopt::get_program_name() const
{
    return f_program_name;
}


/** \brief Retrieve the project name if one is defined.
 *
 * This function returns the name of the project as defined in the
 * options_environment structure passed to the constructor.
 *
 * For example, the snapwebsites project makes use of "snapwebsites"
 * name as its common project name. Many of the configuration files are
 * found under that sub-folder. This ensures that the configuration files
 * are searched for under the indicated folders and again under:
 *
 * \code
 *      <existing path>/<project name>.d/<basename>
 * \endcode
 *
 * So if you have a configuration file named "snapserver.conf" with
 * a path such as "/etc/snapwebsites", you end up with:
 *
 * \code
 *      "/etc/snapwebsites/snapserver.conf"
 *      "/etc/snapwebsites/snapwebsites.d/snapserver.conf"
 * \endcode
 *
 * Notice that the loader adds a ".d" at the end of the project name.
 * Also, if the user were to specify a different filename with the
 * --config command line option, it could end up like this:
 *
 * \code
 *      ... --config /home/alexis/.config/iplock/iplock.conf ...
 *
 *      # First we read this file:
 *      "/home/alexis/.config/iplock/iplock.conf"
 *
 *      # Second we read this file (assuming the same project name
 *      # of "snapwebsites"):
 *      "/home/alexis/.config/iplock/snapwebsites.d/iplock.conf"
 * \endcode
 *
 * The order is important as well. We first load the direct path, then
 * the path with the sub-folder. Finally, we move forward to the next
 * configuration file. We ignore errors when a file can't be loaded or
 * is missing.
 *
 * \return The name of the project, maybe empty if undefined.
 */
std::string getopt::get_project_name() const
{
    if(f_options_environment.f_project_name == nullptr)
    {
        return std::string();
    }
    return f_options_environment.f_project_name;
}




/** \brief Retrieve the group name if one is defined.
 *
 * This function returns the name of the group as defined in the
 * options_environment structure passed to the constructor. This is
 * the main group name (TODO: fix the name).
 *
 * The group name is used for the sub-folder because at times many projects
 * are going to use the same sub-folder.
 *
 * \return The name of the group, maybe empty if undefined.
 */
std::string getopt::get_group_name() const
{
    if(f_options_environment.f_group_name == nullptr)
    {
        return std::string();
    }
    return f_options_environment.f_group_name;
}




} // namespace advgetopt
// vim: ts=4 sw=4 et
