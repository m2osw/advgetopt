/*
 * License:
 *    Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/project/advgetopt
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
 * \brief Advanced getopt version functions.
 *
 * The advgetopt environment is versioned. The functions available here
 * give you access to the version, in case you wanted to make sure you
 * had a minimum version or had some special case options when you
 * want to be able to support various versions.
 */

// self
//
#include    "advgetopt/validator.h"


// advgetopt lib
//
#include    "advgetopt/exception.h"


// cppthread lib
//
#include    <cppthread/log.h>


// snapdev lib
//
#include    <snapdev/not_used.h>


// boost lib
//
#include    <boost/algorithm/string/trim.hpp>


// C++ lib
//
#include    <map>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{


std::map<std::string, validator_factory const *>      g_validator_factories;


} // no name namespace



/** \brief The destructor to ease derived classes.
 *
 * At this point this destructor does nothing more than help with the
 * virtual table.
 */
validator_factory::~validator_factory()
{
}






/** \brief The validator destructor to support virtuals.
 *
 * This destructor is defined so virtual functions work as expected including
 * the deleter.
 */
validator::~validator()
{
}


/** \fn std::string const & validator::name() const;
 * \brief Return the name of the validator.
 *
 * The name() function is used to get the name of the validator.
 * Validators are recognized by name and added to your options
 * using their name.
 *
 * Note that when an option specifies a validator which it can't find,
 * then an error occurs.
 *
 * \return The name of the validator.
 */


/** \fn bool validator::validate(std::string const & value) const;
 * \brief Return true if \p value validates agains this validator.
 *
 * The function parses the \p value parameter and if it matches the
 * allowed parameters, then it returns true.
 *
 * \param[in] value  The value to validate.
 *
 * \return true if the value validates.
 */


void validator::register_validator(validator_factory const & factory)
{
    auto it(g_validator_factories.find(factory.get_name()));
    if(it != g_validator_factories.end())
    {
        throw getopt_logic_error(
                  "you have two or more validator factories named \""
                + factory.get_name()
                + "\".");
    }
    g_validator_factories[factory.get_name()] = &factory;
}


validator::pointer_t validator::create(std::string const & name, string_list_t const & data)
{
    auto it(g_validator_factories.find(name));
    if(it == g_validator_factories.end())
    {
        return validator::pointer_t();
    }

    return it->second->create(data);
}


/** \brief Set the validator for this option.
 *
 * This function parses the specified name and optional parameters and
 * create a corresponding validator for this option.
 *
 * The \p name_and_params string can be defined as:
 *
 * \code
 *     <validator-name>(<param1>, <param2>, ...)
 * \endcode
 *
 * The list of parameters is optional. There may be an empty, just one,
 * or any number of parameters. How the parameters are parsed is left
 * to the validator to decide.
 *
 * If the input string is empty, the current validator, if one is
 * installed, gets removed.
 *
 * \param[in] name_and_params  The validator name and parameters.
 */
validator::pointer_t validator::create(std::string const & name_and_params)
{
    if(name_and_params.empty())
    {
        return validator::pointer_t();
    }

    if(name_and_params.length() >= 2
    && name_and_params[0] == '/')
    {
        // for the regex we have a special case
        //
        string_list_t data{name_and_params};
        return create("regex", data);
    }
    else
    {
        std::string::size_type const params(name_and_params.find('('));
        std::string name(name_and_params);
        string_list_t data;
        if(params != std::string::npos)
        {
            if(name_and_params.back() != ')')
            {
                throw getopt_logic_error(
                      "invalid validator parameter definition: \""
                    + name_and_params
                    + "\", the ')' is missing.");
            }
            name = name_and_params.substr(0, params);
            split_string(name_and_params.substr(params + 1, name_and_params.length() - params - 2)
                       , data
                       , {","});
        }
        return create(name, data);
    }
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
