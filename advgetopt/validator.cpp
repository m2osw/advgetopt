/*
 * File:
 *    advgetopt/version.cpp -- advanced get option implementation
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
 * \brief Advanced getopt version functions.
 *
 * The advgetopt environment is versioned. The functions available here
 * give you access to the version, in case you wanted to make sure you
 * had a minimum version or had some special case options when you
 * want to be able to support various versions.
 */
#include    "advgetopt/validator.h"


namespace advgetopt
{





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







/** \brief Return the name of this validator.
 *
 * This function returns "integer".
 *
 * \return "integer".
 */
std::string const validator_integer::name() const
{
    return std::string("integer");
}


/** \brief Determine whether value is an integer.
 *
 * This function verifies that the specified value is a valid integer.
 *
 * It makes sures that the value is only composed of digits (`[0-9]+`).
 * It may also start with a sign (`[-+]?`).
 *
 * The function also makes sure that the value fits in an `int64_t` value.
 *
 * \todo
 * Add support for binary, octal, hexadecimal.
 *
 * \param[in] value  The value to validate.
 *
 * \return true if the value validates.
 */
bool validator_integer::validate(std::string const & value) const
{
    uint64_t integer(0);
    char const * s(value.c_str());

    if(*s == '-' || *s == '+')
    {
        ++s;
    }

    if(*s == '\0')
    {
        // empty string, not considered valid
        //
        return false;
    }

    for(;;)
    {
        char const c(*s++);
        if(c == '\0')
        {
            // valid
            //
            // TODO: verify for sign overflow
            //
            return true;
        }
        if(c < '0' || c > '9')
        {
            // invalid digit
            //
            return false;
        }

        uint64_t const old(integer);
        integer = integer * 10 + c - '0';
        if(integer < old)
        {
            // overflow
            //
            return false;
        }
    }
}






} // namespace advgetopt
// vim: ts=4 sw=4 et