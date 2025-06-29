// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Advanced getopt implementation.
 *
 * The advgetopt class and implementation is an advanced library to parse
 * command line parameters from static definitions specified by the caller.
 *
 * The class supports the command line options, options found in a default
 * configuration file or in a user defined configuration file.
 *
 * The class also includes support for displaying error messages and help
 * information about all the command line arguments.
 */

// self
//
#include    "advgetopt/variables.h"

#include    "advgetopt/exception.h"


// C++
//
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{



/** \brief Canonicalize the variable name.
 *
 * This function canonicalizes the name of a variable.
 *
 * This means:
 *
 * * Replace a sequence of ':' with '::'.
 * * Replace a sequence of '.' with '::'.
 * * Replace a '_' with '-'.
 *
 * \note
 * Variable names that come from a configuration file will already have
 * been canonicalized, but the user can directly call the get_variable()
 * and set_variable() functions which will also benefit from this
 * conversion.
 *
 * \exception getopt_invalid
 * If the variable or one of the section names start with a digit, this
 * exception is raised. The exception is also raised if we detect an
 * empty section name (as in "::test" or "double::::scope" or "not..allowed").
 *
 * \param[in] name  The name of the variable.
 */
std::string variables::canonicalize_variable_name(std::string const & name)
{
    std::string result;

    bool first(true);
    for(char const * n(name.c_str()); *n != '\0'; ++n)
    {
        if(*n == ':' || *n == '.')
        {
            if(first)
            {
                throw getopt_invalid(
                      "found an empty section name in \""
                    + name
                    + "\".");
            }
            while(n[1] == ':' || n[1] == '.')
            {
                ++n;
            }
            result += "::";
            first = true;
        }
        else
        {
            if(first && *n >= '0' && *n <= '9')
            {
                throw getopt_invalid(
                      "a variable name or section name in \""
                    + name
                    + "\" starts with a digit, which is not allowed.");
            }
            first = false;
            if(*n == '_')
            {
                result += '-';
            }
            else
            {
                result += *n;
            }
        }
    }

    return result;
}


/** \brief Check whether a variable is defined.
 *
 * If you want to verify that a variable is defined before retrieving
 * it, you can use this function. A variable can be set to the empty
 * string so checking the returned value of the get_variable() is
 * not sufficient to know whether the variable is defined or log.
 *
 * \param[in] name  The name of the variable to check.
 *
 * \return true if the variable is defined.
 */
bool variables::has_variable(std::string const & name) const
{
    auto it(f_variables.find(canonicalize_variable_name(name)));
    return it != f_variables.end();
}


/** \brief Return the value of the named variable.
 *
 * This function searches for the named variable and returns its value
 * if defined.
 *
 * \param[in] name  The name of the variable to retrieve.
 *
 * \return The variable value of an empty string.
 */
std::string variables::get_variable(std::string const & name) const
{
    auto it(f_variables.find(canonicalize_variable_name(name)));
    if(it != f_variables.end())
    {
        return it->second;
    }

    return std::string();
}


/** \brief Return a reference to the map of variables.
 *
 * This function returns a reference to the whole map of variables.
 *
 * The map is composed of named values. The first string is the name of
 * variables and the second string is the value.
 *
 * \note
 * It is not multi-thread safe since the variable make can be updated at any
 * time.
 *
 * \return The reference to the map of variables.
 */
variables::variable_t const & variables::get_variables() const
{
    return f_variables;
}


/** \brief Set a variable.
 *
 * This function sets a variable in the getopt object.
 *
 * The value of variables can be used to replace `${...}` entries in
 * parameters found on the command line or in configuration files.
 *
 * By default, if that variable already existed, then its value gets
 * replaced (assignment_t::ASSIGNMENT_SET).
 *
 * You can use this function to define a default after loading data with:
 *
 * \code
 * vars->set_variable("foo", "default value", assignment_t::ASSIGNMENT_OPTIONAL);
 * \endcode
 *
 * \note
 * The value of a variable can itself include `${...}` references.
 * When parsing a parameter for variables, such are replaced recursively.
 * See process_value() for details.
 *
 * \param[in] name  The name of the variable.
 * \param[in] value  The value of the variable.
 * \param[in] assignment  The operator to use to set this variable.
 *
 * \sa process_value()
 */
void variables::set_variable(
      std::string const & name
    , std::string const & value
    , assignment_t assignment)
{
    std::string const var(canonicalize_variable_name(name));
    auto it(f_variables.find(var));
    switch(assignment)
    {
    case assignment_t::ASSIGNMENT_OPTIONAL:
        if(it == f_variables.end())
        {
            f_variables[var] = value;
        }
        break;

    case assignment_t::ASSIGNMENT_APPEND:
        if(it == f_variables.end())
        {
            f_variables[var] = value;
        }
        else
        {
            f_variables[var] = it->second + value;
        }
        break;

    case assignment_t::ASSIGNMENT_NEW:
        if(it == f_variables.end())
        {
            f_variables[var] = value;
        }
        else
        {
            throw getopt_defined_twice(
                  "variable \""
                + var
                + "\" is already defined.");
        }
        break;

    //case assignment_t::ASSIGNMENT_NONE:
    //case assignment_t::ASSIGNMENT_SET:
    default:
        f_variables[var] = value;
        break;

    }
}


/** \brief Process variables against a parameter.
 *
 * Whenever a parameter is retrieved, its value is passed through this
 * function and if the variable processing is allowed, it searches for
 * `${...}` sequances and when such are found, it replaces them with the
 * corresponding variable content.
 *
 * The process is recursive meaning that if a variable includes the `${...}`
 * sequence, that variable will itself also be replaced.
 *
 * The variables can be defined in a `[variables]` section and by the
 * programmer by calling the set_variable() function.
 *
 * \note
 * This functionality is automatically used when the
 * SYSTEM_OPTION_PROCESS_VARIABLES flag is set in your environment definition.
 * If you prefer to have it only function for a few of your parameters, then
 * do not set the SYSTEM_OPTION_PROCESS_VARIABLES and only call this function
 * for the few values you want to include variables.
 *
 * \todo
 * Consider having a cache, although for variables that would return system
 * information, it could change at any time.
 *
 * \param[in] value  The parameter value to be processed.
 *
 * \return The processed value with the variables updated.
 */
std::string variables::process_value(std::string const & value) const
{
    // to support the recursivity, we call a sub-function which calls itself
    // whenever a variable is discovered to include another variable; that
    // recursivity is broken immediately if a variable includes itself;
    // this function is private
    //
    variable_names_t names;
    return recursive_process_value(value, names);
}


/** \brief Internal function processing variables recursively.
 *
 * This function goes through value and replaces the `${...}` with the
 * corresponding variable data. The content of a variable is itself
 * passed through this process so it is recursive.
 *
 * The function records which variables it has worked on so far to
 * prevent the function from re-adding the same variable (avoid infinite
 * loop).
 *
 * \param[in] value  The value to parse.
 * \param[in] names  A set of variable names that have already been processed.
 */
std::string variables::recursive_process_value(
      std::string const & value
    , variable_names_t & names) const
{
    std::string result;

    for(char const * s(value.c_str()); *s != '\0'; ++s)
    {
        char c(*s);
        if(c == '$' && s[1] == '{') // start variable reference?
        {
            s += 2;
            char const * name(s);
            for(; *s != '}' && *s != '\0'; ++s);
            if(*s == '\0')
            {
                // invalid variable reference
                //
                result += "${";
                result += name;
                return result;
            }

            // TODO: add support for conversions like we have in bash
            //       (i.e. ${var:-extension} ${var%.extension} ...
            //       see man bash section "Parameter Expansion")
            //
            // TODO: add support for emitting errors on an undefined
            //       variable
            //
            std::string var(std::string(name, s - name));
            auto allowed(names.insert(var));
            if(allowed.second)
            {
                result += recursive_process_value(get_variable(var), names);
                names.erase(allowed.first);
            }
            else
            {
                result += "<variable \"" + var + "\" loops>";
            }
        }
        else
        {
            result += c;
        }
    }

    return result;
} // LCOV_EXCL_LINE



} // namespace advgetopt
// vim: ts=4 sw=4 et
