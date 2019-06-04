/*
 * File:
 *    advgetopt/advgetopt_options.cpp -- advanced get option implementation
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
#include    "advgetopt/conf_file.h"
#include    "advgetopt/exception.h"
#include    "advgetopt/log.h"

// libutf8 lib
//
#include    <libutf8/libutf8.h>

//// C++ lib
////
//#include    <iomanip>



namespace advgetopt
{





/** \brief Retrieve the number of arguments.
 *
 * This function returns the number of arguments that were specified after
 * the named option.
 *
 * The function returns zero if the argument was never specified on the
 * command line. If the option accepts exactly one parameter (i.e. not
 * marked as a multiple arguments option: GETOPT_FLAG_MULTIPLE) then
 * the function returns either zero (not specified) or one (specified
 * at least once.)
 *
 * \param[in] name  The name of the option to check.
 *
 * \return The number of arguments specified on the command line or zero.
 */
size_t getopt::size(std::string const & name) const
{
    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->size();
    }
    return 0;
}


/** \brief Get the default value for this option.
 *
 * When an option is not defined, you may use this function to retrieve its
 * default instead. This is actually done automatically when you call the
 * get_string() or get_long() functions.
 *
 * An option without a default has this function returning nullptr.
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if this function is
 * called with an empty \p name.
 *
 * \param[in] name  The name of the parameter of which you want to retrieve
 *                  the default value.
 *
 * \return The default value or an empty string if no value is defined.
 */
std::string getopt::get_default(std::string const & name) const
{
    if(name.empty())
    {
        throw getopt_exception_logic("argument name cannot be empty.");
    }

    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->get_default();
    }

    return std::string();
}


/** \brief Get the content of an option as a string.
 *
 * Get the content of the named parameter as a string. Command line options
 * that accept multiple arguments accept the \p idx parameter to
 * specify which item you are interested in.
 *
 * Note that the option must have been specified on the command line or have
 * a default value. For options that do not have a default value, you want
 * to call the is_defined() function first.
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if \p name was not
 * found on the command line and it has no default, or if \p idx is
 * out of bounds.
 *
 * \param[in] name  The name of the option to read.
 * \param[in] idx  The zero based index of a multi-argument command line option.
 *
 * \return The option argument as a string.
 */
std::string getopt::get_string(std::string const & name, int idx) const
{
    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        throw getopt_exception_logic(
                  "there is no --"
                + name
                + " option defined.");
    }

    if(!opt->is_defined())
    {
        if(opt->has_default())
        {
            return opt->get_default();
        }
        throw getopt_exception_logic(
                  "the --"
                + name
                + " option was not defined on the command line and it has no default.");
    }

    return opt->get_value(idx);
}


/** \brief This function retrieves an argument as a long value.
 *
 * This function reads the specified argument from the named option and
 * transforms it to a long value. It then checks the result against the
 * specified minimum and maximum range.
 *
 * The function name represents an argument that needs to be defined. You
 * can test whether it was defined on the command line with the is_defined()
 * function. The index must be between 0 and 'size() - 1' inclusive. If
 * the item was not defined, then size() returns zero and you cannot call
 * this function.
 *
 * The function does not check the validity of the minimum and maximum
 * parameters. If \p min \> \p max is true then the function will always
 * fail with a call to usage() as no value can be defined between \p min
 * and \p max in that case. The minimum and maximum values are inclusive,
 * so a range of 1 to 9 is defined with exactly 1 and 9 in min and max.
 * For example, the z library compression could be retrieved with:
 *
 * \code
 * int level(6); // default to 6
 * if(opt.is_defined("zlevel"))
 * {
 *   zlevel = opt.get_long("zlevel", 0, 1, 9);
 * }
 * \endcode
 *
 * Note that the function can be used to read unsigned numbers, however
 * at this point getopt does not really support negative numbers (i.e. because
 * -\<number> is viewed as an option.)
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if \p name was not
 * found on the command line and it has no default, or if \p idx is
 * out of bounds.
 *
 * \param[in] name  The name of the option to retrieve.
 * \param[in] idx  The index of the argument to retrieve.
 * \param[in] min  The minimum value that will be returned (inclusive).
 * \param[in] max  The maximum value that will be returned (inclusive).
 *
 * \return The argument as a long.
 */
long getopt::get_long(std::string const & name, int idx, long min, long max)
{
    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        throw getopt_exception_logic(
                  "there is no --"
                + name
                + " option defined.");
    }

    long result(0);
    if(!opt->is_defined())
    {
        std::string const d(opt->get_default());
        if(d.empty())
        {
            throw getopt_exception_logic(
                      "the --"
                    + name
                    + " option was not defined on the command line and it has no or an empty default.");
        }
        char * end;
        char const * str(d.c_str());
        result = strtol(str, &end, 10);
        if(end != str + d.length())
        {
            // here we throw because this default value is defined in the
            // options of the tool and not by the user
            //
            throw getopt_exception_logic(
                      "invalid default number \""
                    + d
                    + "\" for option --"
                    + name);
        }
    }
    else
    {
        result = opt->get_long(idx);
    }

    // TODO: replace with validators
    //
    if(result < min || result > max)
    {
        log << log_level_t::error
            << result
            << " is out of bounds ("
            << min
            << ".."
            << max
            << " inclusive) in parameter --"
            << name
            << "."
            << end;
        result = -1;
    }

    return result;
}





} // namespace advgetopt
// vim: ts=4 sw=4 et
