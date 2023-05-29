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
 * \brief Implementation of the integer validator.
 *
 * This validator is used to verify that a parameter represents a valid
 * integer.
 *
 * Note that the validator supports 64 bits integers by default. You can
 * reduce the size by defining your parameter with a range as required
 * by your application.
 *
 * The value is checked for overflows on a signed 64 bits value.
 */

// self
//
#include    "advgetopt/validator_integer.h"


// cppthread
//
#include    <cppthread/log.h>


// snapdev
//
#include    <snapdev/trim_string.h>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{



class validator_integer_factory
    : public validator_factory
{
public:
    validator_integer_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("integer");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_integer>(data);
    }
};

validator_integer_factory       g_validator_integer_factory;



} // no name namespace





/** \brief Initialize the integer validator.
 *
 * The constructor accepts a string with values and ranges which are
 * used to limit the values that can be used with this parameter.
 *
 * Remember that the default does not have to be included in these
 * values. It will still be viewed as \em valid.
 *
 * The string uses the following format:
 *
 * \code
 *    start: range
 *         | start ',' range
 *
 *    range: number
 *         | number '...' number
 *
 *    number: [-+]?[0-9]+
 * \endcode
 *
 * Note that a single number is considered to be a range and is managed
 * the exact same way. A value which matches any of the ranges is considered
 * valid.
 *
 * The start and end values of a range are optional. If not specified, the
 * start value is set to the minimum int64_t value. If the not specified,
 * the end value is set to the maximum int64_t value.
 *
 * Examples:
 *
 * \code
 *     "-100...100,-1000"
 * \endcode
 *
 * This example allows all values between -100 and +100 inclusive and also
 * allows the value -1000.
 *
 * \code
 *     "1..."
 * \endcode
 *
 * This example allows all positive values.
 *
 * \param[in] ranges  The ranges used to limit the integer.
 */
validator_integer::validator_integer(string_list_t const & range_list)
{
    range_t range;
    for(auto r : range_list)
    {
        std::string::size_type const pos(r.find("..."));
        if(pos == std::string::npos)
        {
            if(!convert_string(r, range.f_minimum))
            {
                cppthread::log << cppthread::log_level_t::error
                               << r
                               << " is not a valid standalone value for your ranges;"
                                  " it must only be digits, optionally preceeded by a sign (+ or -)"
                                  " and not overflow an int64_t value."
                               << cppthread::end;
                continue;
            }
            range.f_maximum = range.f_minimum;
        }
        else
        {
            std::string const min_value(snapdev::trim_string(r.substr(0, pos)));
            if(!min_value.empty())
            {
                if(!convert_string(min_value, range.f_minimum))
                {
                    cppthread::log << cppthread::log_level_t::error
                                   << min_value
                                   << " is not a valid value for your range's start;"
                                      " it must only be digits, optionally preceeded by a sign (+ or -)"
                                      " and not overflow an int64_t value."
                                   << cppthread::end;
                    continue;
                }
            }

            std::string const max_value(snapdev::trim_string(r.substr(pos + 3)));
            if(!max_value.empty())
            {
                if(!convert_string(max_value, range.f_maximum))
                {
                    cppthread::log << cppthread::log_level_t::error
                                   << max_value
                                   << " is not a valid value for your range's end;"
                                      " it must only be digits, optionally preceeded by a sign (+ or -)"
                                      " and not overflow an int64_t value."
                                   << cppthread::end;
                    continue;
                }
            }

            if(range.f_minimum > range.f_maximum)
            {
                cppthread::log << cppthread::log_level_t::error
                               << min_value
                               << " has to be smaller or equal to "
                               << max_value
                               << "; you have an invalid range."
                               << cppthread::end;
                continue;
            }
        }
        f_allowed_values.push_back(range);
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "integer".
 *
 * \return "integer".
 */
std::string validator_integer::name() const
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
 * If ranges were defined, then the function also verifies that the
 * value is within at least one of the ranges.
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
    std::int64_t result(0);
    if(convert_string(value, result))
    {
        if(f_allowed_values.empty())
        {
            return true;
        }

        for(auto f : f_allowed_values)
        {
            if(result >= f.f_minimum
            && result <= f.f_maximum)
            {
                return true;
            }
        }
        return false;
    }

    return false;
}


/** \brief Convert a string to an std::int64_t value.
 *
 * This function is used to convert a string to an integer with full
 * boundary verification.
 *
 * \warning
 * There is no range checks in this function since it does not have access
 * to the ranges (i.e. it is static).
 *
 * \param[in] value  The value to be converted to an integer.
 * \param[out] result  The resulting integer.
 *
 * \return true if the conversion succeeded.
 */
bool validator_integer::convert_string(std::string const & value, std::int64_t & result)
{
    std::uint64_t integer(0);
    char const * s(value.c_str());

    char sign('\0');
    if(*s == '-' || *s == '+')
    {
        sign = *s;
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
            if(sign == '-')
            {
                if(integer > 0x8000000000000000ULL)
                {
                    return false;
                }
                result = -integer;
            }
            else
            {
                if(integer > 0x7FFFFFFFFFFFFFFFULL)
                {
                    return false;
                }
                result = integer;
            }
            return true;
        }
        if(c < '0' || c > '9')
        {
            // invalid digit
            //
            return false;
        }

        std::uint64_t const old(integer);
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
