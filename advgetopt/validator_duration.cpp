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
 * \brief Implementation of the duration validator.
 *
 * The advgetopt allows for validating the input parameters automatically.
 * This one validator checks whether the input represents what is considered
 * a valid duration.
 *
 * This includes a floating point number followed by a suffix such as "week"
 * or "days".
 */

// self
//
#include    "advgetopt/validator_duration.h"

#include    "advgetopt/validator_double.h"


// cppthread lib
//
#include    <cppthread/log.h>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{



class validator_duration_factory
    : public validator_factory
{
public:
    validator_duration_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("duration");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_duration>(data);
    }
};

validator_duration_factory      g_validator_duration_factory;



} // no name namespace





/** \brief Initialize the duration validator.
 *
 * The constructor accepts a string defining the acceptable durations.
 *
 * The string uses the following format:
 *
 * \code
 *    start: flags
 *         | start flags
 *
 *    flags: 'small'
 *         | 'large'
 * \endcode
 *
 * 'small' stands for small values (down to 1 second).
 *
 * 'large' stands for large values (so the 'm' suffix represents month,
 * not minutes).
 *
 * The 'small' and 'large' flags are exclusive, the last one will be effective.
 *
 * \param[in] flag_list  The flags used to define the usage of the 'm' suffix.
 */
validator_duration::validator_duration(string_list_t const & flag_list)
{
    for(auto r : flag_list)
    {
        if(r == "small")
        {
            f_flags &= ~VALIDATOR_DURATION_LONG;
        }
        else if(r == "large")
        {
            f_flags |= VALIDATOR_DURATION_LONG;
        }
        else
        {
            cppthread::log << cppthread::log_level_t::error
                           << r
                           << " is not a valid flag for the duration validator."
                           << cppthread::end;
            continue;
        }
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "duration".
 *
 * \return "duration".
 */
std::string const validator_duration::name() const
{
    return std::string("duration");
}


/** \brief Determine whether value is a valid duration.
 *
 * This function verifies that the specified value is a valid duration.
 *
 * It makes sures that the value is a valid decimal number which optionally
 * starts with a sign (`[-+]?`) and is optionally followed by a known
 * measurement suffix.
 *
 * \param[in] value  The value to validate.
 *
 * \return true if the value validates.
 */
bool validator_duration::validate(std::string const & value) const
{
    double result(0);
    return convert_string(value, f_flags, result);
}


/** \brief Convert a string to a double value representing a duration.
 *
 * This function is used to convert a string to a double representing a
 * duration. The duration can be specified with one of the following suffixes:
 *
 * * "s" or " second" or " seconds" -- the double is returned as is
 * * "m" or " minute" or " minutes" -- the double is multiplied by 60
 * * "h" or " hour" or " hours" -- the double is multiplied by 3600
 * * "d" or " day" or " days" -- the double is multiplied by 86400
 * * "w" or " week" or " weeks" -- the double is multiplied by 604800
 * * "m" or " month" or " months" -- the double is multiplied by 2592000
 * * "y" or " year" or " years" -- the double is multiplied by 31536000
 *
 * The "m" suffix is interpreted as "minute" by default. The flags passed
 * to the contructor can change that interpretation into "month" instead.
 *
 * One month uses 30 days.
 *
 * One year uses 365 days.
 *
 * Note that the input can be a double. So you can define a duration of
 * "1.3 seconds" or "2.25 days".
 *
 * \todo
 * The last multiplication doesn't verify that no overflow or underflow
 * happens.
 *
 * \warning
 * There is no range checks in this function since it does not have access
 * to the ranges (i.e. it is static).
 *
 * \param[in] value  The value to be converted to a duration.
 * \param[in] flags  The flags to determine how to interpret the suffix.
 * \param[out] result  The resulting duration in seconds.
 *
 * \return true if the conversion succeeded.
 */
bool validator_duration::convert_string(
          std::string const & value
        , flag_t flags
        , double & result)
{
    // determine the factor by checking the suffix
    //
    double factor(1.0);
    std::string::size_type pos(value.length());
    for(; pos > 0; --pos)
    {
        char c(value[pos - 1]);
        if(c >= '0'
        && c <= '9'
        || c == '.')
        {
            break;
        }
    }
    if(pos == 0)
    {
        return false;
    }
    std::string const number(value.substr(0, pos));
    for(; pos < value.length() && isspace(value[pos]); ++pos);
    if(pos < value.length())
    {
        // copy and force lowercase
        //
        std::string suffix;
        for(; pos < value.length(); ++pos)
        {
            if(value[pos] >= 'A'
            && value[pos] <= 'Z')
            {
                suffix += value[pos] + 0x20;
            }
            else
            {
                suffix += value[pos];
            }
        }

        switch(suffix[0])
        {
        case 'd':
            if(suffix == "d"
            || suffix == "day"
            || suffix == "days")
            {
                factor = 86400;
            }
            else
            {
                return false;
            }
            break;

        case 'h':
            if(suffix == "h"
            || suffix == "hour"
            || suffix == "hours")
            {
                factor = 3600;
            }
            else
            {
                return false;
            }
            break;

        case 'm':
            if(suffix == "m")
            {
                if((flags & VALIDATOR_DURATION_LONG) != 0)
                {
                    factor = 86400.0 * 30.0;    // 1 month
                }
                else
                {
                    factor = 60;                // 1 minute
                }
            }
            else if(suffix == "minute"
                 || suffix == "minutes")
            {
                factor = 60;
            }
            else if(suffix == "month"
                 || suffix == "months")
            {
                factor = 86400.0 * 30.0;
            }
            else
            {
                return false;
            }
            break;

        case 's':
            if(suffix != "s"
            && suffix != "second"
            && suffix != "seconds")
            {
                return false;
            }
            break;

        case 'w':
            if(suffix == "w"
            || suffix == "week"
            || suffix == "weeks")
            {
                factor = 86400 * 7;
            }
            else
            {
                return false;
            }
            break;

        case 'y':
            if(suffix == "y"
            || suffix == "year"
            || suffix == "years")
            {
                factor = 86400 * 365;
            }
            else
            {
                return false;
            }
            break;

        default:
            return false;

        }
    }

    if(!validator_double::convert_string(number, result))
    {
        return false;
    }

    // TODO: catch ERANGE errors
    result *= factor;

    return true;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
