// Copyright (c) 2006-2022  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Implementation of the double validator.
 *
 * This validator can be used to convert the value of a parameter to a
 * double with overflow and underflow verifications.
 */

// self
//
#include    "advgetopt/validator_double.h"


// cppthread
//
#include    <cppthread/log.h>


// snapdev
//
#include    <snapdev/not_used.h>


// boost
//
#include    <boost/algorithm/string/trim.hpp>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{


class validator_double_factory
    : public validator_factory
{
public:
    validator_double_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("double");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        snapdev::NOT_USED(data); // ignore `data`
        return std::make_shared<validator_double>(data);
    }
};

validator_double_factory       g_validator_double_factory;


} // no name namespace




/** \brief Initialize the double validator.
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
 *    number: [-+]?[0-9]+(.[0-9]+([eE][+-][0-9]+)?)?
 * \endcode
 *
 * Note that a single number is considered to be a range and is managed
 * the exact same way. A value which matches any of the ranges is considered
 * valid.
 *
 * Example:
 *
 * \code
 *     "-10.01...+10.05,0.0005661"
 * \endcode
 *
 * This example allows all values between -10.01 and +10.05 inclusive and also
 * allows the value 0.0005661.
 *
 * \param[in] ranges  The ranges used to limit the double.
 */
validator_double::validator_double(string_list_t const & range_list)
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
                               << " is not a valid standalone value;"
                                  " it must be a valid floating point,"
                                  " optionally preceeded by a sign (+ or -)."
                               << cppthread::end;
                continue;
            }
            range.f_maximum = range.f_minimum;
        }
        else
        {
            std::string min_value(r.substr(0, pos));
            boost::trim(min_value);
            if(!convert_string(min_value, range.f_minimum))
            {
                cppthread::log << cppthread::log_level_t::error
                               << min_value
                               << " is not a valid value for your range's start;"
                                  " it must be a valid floating point,"
                                  " optionally preceeded by a sign (+ or -)."
                               << cppthread::end;
                continue;
            }

            std::string max_value(r.substr(pos + 3));
            boost::trim(max_value);
            if(!convert_string(max_value, range.f_maximum))
            {
                cppthread::log << cppthread::log_level_t::error
                               << max_value
                               << " is not a valid value for your range's end;"
                                  " it must be a valid floating point,"
                                  " optionally preceeded by a sign (+ or -)."
                               << cppthread::end;
                continue;
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
 * This function returns "double".
 *
 * \return "double".
 */
std::string validator_double::name() const
{
    return std::string("double");
}


/** \brief Determine whether value is a double.
 *
 * This function verifies that the specified value is a valid double.
 *
 * It makes sures that the value is only composed of digits (`[0-9]+`)
 * and optionally has a decimal pointer followed by more digits and
 * an optional exponent.
 *
 * The number may also start with a sign (`[-+]?`).
 *
 * If ranges were defined, then the function also verifies that the
 * value is within at least one of the ranges.
 *
 * \param[in] value  The value to validate.
 *
 * \return true if the value validates.
 */
bool validator_double::validate(std::string const & value) const
{
    double result(0.0);
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


/** \brief Convert a string to a double value.
 *
 * This function is used to convert a string to a double with full
 * boundary verification.
 *
 * \todo
 * This function calls std::strtod() which interprets the decimal separator
 * depending on the current locale. We really want to only accept periods.
 *
 * \warning
 * There is no range checks in this function since it does not have access
 * to the ranges (i.e. it is static).
 *
 * \param[in] value  The value to be converted to a double.
 * \param[out] result  The resulting double.
 *
 * \return true if the conversion succeeded.
 */
bool validator_double::convert_string(std::string const & value, double & result)
{
    char const * start(value.c_str());

    // do not allow spaces before the number
    //
    if(start[0] != '+'
    && start[0] != '-'
    && (start[0] < '0' || start[0] > '9'))
    {
        return false;
    }

    char * end(nullptr);
    errno = 0;
    result = std::strtod(start, &end);

    // do not allow anything after the last digit
    // also return false on an overflow
    //
    return end == start + value.length()
        && errno != ERANGE;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
