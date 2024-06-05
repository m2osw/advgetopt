// Copyright (c) 2006-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Implementation of the size validator.
 *
 * The advgetopt allows for validating the input parameters automatically.
 * This one validator checks whether the input represents what is considered
 * a valid size of bits or bytes.
 *
 * This includes a floating point number followed by a suffix such as "kB"
 * or "Gb" or "TiB".
 *
 * \note
 * The size can also represents bits, even though it was written to read
 * bytes. If your command line option is expecting a size in bits, this works
 * just as expected.
 *
 * The size conversions are based on the International System of Units (SI).
 *
 * See: https://en.wikipedia.org/wiki/Kilobyte
 */

// self
//
#include    "advgetopt/validator_size.h"

#include    "advgetopt/validator_double.h"


// cppthread
//
#include    <cppthread/log.h>


// snapdev
//
#include    <snapdev/int128_literal.h>
#include    <snapdev/math.h>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{



class validator_size_factory
    : public validator_factory
{
public:
    validator_size_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("size");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_size>(data);
    }
};

validator_size_factory      g_validator_size_factory;



} // no name namespace





/** \brief Initialize the size validator.
 *
 * The constructor accepts a string defining the acceptable sizes.
 *
 * The string uses the following format:
 *
 * \code
 *    start: flags
 *         | start flags
 *
 *    flags: 'si'
 *         | 'legacy'
 * \endcode
 *
 * 'si' stands for "Systeme International" (French for International
 * System of Units); this means "1kB" will stand for "1000 bytes".
 *
 * 'legacy' means that one kilo bytes will be represented by 1024 bytes.
 * So "1kB" with the legacy flag turned on represents "1024 bytes".
 *
 * The 'si' and 'legacy' flags are exclusive, the last one will be effective.
 *
 * \param[in] flag_list  The flags used to define how to interpret the data.
 */
validator_size::validator_size(string_list_t const & flag_list)
{
    for(auto r : flag_list)
    {
        if(r == "si")
        {
            f_flags &= ~VALIDATOR_SIZE_POWER_OF_TWO;
        }
        else if(r == "legacy")
        {
            f_flags |= VALIDATOR_SIZE_POWER_OF_TWO;
        }
        else
        {
            cppthread::log << cppthread::log_level_t::error
                           << r
                           << " is not a valid flag for the size validator."
                           << cppthread::end;
            continue;
        }
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "size".
 *
 * \return "size".
 */
std::string validator_size::name() const
{
    return std::string("size");
}


/** \brief Determine whether value is a valid size.
 *
 * This function verifies that the specified value is a valid size.
 *
 * It makes sures that the value is a valid decimal number which optionally
 * starts with a sign (`[-+]?`) and is optionally followed by a known
 * measurement suffix.
 *
 * \param[in] value  The value to validate.
 *
 * \return true if the value validates.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
bool validator_size::validate(std::string const & value) const
{
    using namespace snapdev::literals;
    __int128 result(0_int128);
    return convert_string(value, f_flags, result);
}
#pragma GCC diagnostic pop


/** \brief Convert a string to a large integer (128 bits) value representing a size.
 *
 * This function is used to convert a string to a double representing a
 * size. The size can be specified with one of the following suffixes:
 *
 * * "B" -- 1000^0 bytes
 * * "kB" -- 1000^1 bytes
 * * "MB" -- 1000^2 bytes
 * * "GB" -- 1000^3 bytes
 * * "TB" -- 1000^4 bytes
 * * "PB" -- 1000^5 bytes
 * * "EB" -- 1000^6 bytes
 * * "ZB" -- 1000^7 bytes
 * * "YB" -- 1000^8 bytes
 * * "RB" -- 1000^9 bytes
 * * "QB" -- 1000^10 bytes
 * * "KiB" -- 1024^1 bytes
 * * "MiB" -- 1024^2 bytes
 * * "GiB" -- 1024^3 bytes
 * * "TiB" -- 1024^4 bytes
 * * "PiB" -- 1024^5 bytes
 * * "EiB" -- 1024^6 bytes
 * * "ZiB" -- 1024^7 bytes
 * * "YiB" -- 1024^8 bytes
 * * "RiB" -- 1024^9 bytes
 * * "QiB" -- 1024^10 bytes
 *
 * The suffix capitalization is not important since we can always distinguish
 * both types (power of 1000 or 1024). The 'B' represents bytes either way so
 * it does not need to be dinstinguished.
 *
 * In legacy mode (VALIDATOR_SIZE_POWER_OF_TWO flag set), the 1024 power
 * is always used.
 *
 * The final result is an integer representing bytes. If you use a decimal
 * number, it will be rounded down (floor). So "1.9B" returns 1. A decimal
 * number is practical for larger sizes such as "1.3GiB".
 *
 * \note
 * The result is returned in a 128 bit number because Zeta and Yeta values
 * do not fit in 64 bits.
 *
 * \todo
 * We may want to support full names instead of just the minimal abbreviated
 * suffixes (i.e. "3 bytes" fails). Also we could support bits but that is
 * \em complicated because the only difference is whether you use upper or
 * lower case characters (i.e. kB is kilo bytes, kb is kilo bits).
 *
 * \param[in] value  The value to be converted to a size.
 * \param[in] flags  The flags to determine how to interpret the suffix.
 * \param[out] result  The resulting size in bits or bytes.
 *
 * \return true if the conversion succeeded.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
bool validator_size::convert_string(
          std::string const & value
        , flag_t flags
        , __int128 & result)
{
    using namespace snapdev::literals;

    // determine the factor by checking the suffix
    //
    __int128 factor(1_int128);
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
    __int128 const base(
        (flags & VALIDATOR_SIZE_POWER_OF_TWO) != 0
            ? 1024_int128
            : 1000_int128);
    std::string const number(value.substr(0, pos));
    for(; pos < value.length() && isspace(value[pos]); ++pos);
    if(pos < value.length())
    {
        // copy and force lowercase (although the case is important
        // when writing such a measurement, it does not matter when
        // testing here)
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
        case 'b':
            if(suffix != "b")
            {
                return false;
            }
            break;

        case 'e':
            if(suffix == "eb")
            {
                factor = snapdev::pow(base, 6);
            }
            else if(suffix == "eib")
            {
                factor = snapdev::pow(1024_int128, 6);
            }
            else
            {
                return false;
            }
            break;

        case 'g':
            if(suffix == "gb")
            {
                factor = snapdev::pow(base, 3);
            }
            else if(suffix == "gib")
            {
                factor = snapdev::pow(1024_int128, 3);
            }
            else
            {
                return false;
            }
            break;

        case 'k':
            if(suffix == "kb")
            {
                factor = base;
            }
            else if(suffix == "kib")
            {
                factor = 1024_int128;
            }
            else
            {
                return false;
            }
            break;

        case 'm':
            if(suffix == "mb")
            {
                factor = snapdev::pow(base, 2);
            }
            else if(suffix == "mib")
            {
                factor = snapdev::pow(1024_int128, 2);
            }
            else
            {
                return false;
            }
            break;

        case 'p':
            if(suffix == "pb")
            {
                factor = snapdev::pow(base, 5);
            }
            else if(suffix == "pib")
            {
                factor = snapdev::pow(1024_int128, 5);
            }
            else
            {
                return false;
            }
            break;

        case 'q':
            if(suffix == "qb")
            {
                factor = snapdev::pow(base, 10);
            }
            else if(suffix == "qib")
            {
                factor = snapdev::pow(1024_int128, 10);
            }
            else
            {
                return false;
            }
            break;

        case 'r':
            if(suffix == "rb")
            {
                factor = snapdev::pow(base, 9);
            }
            else if(suffix == "rib")
            {
                factor = snapdev::pow(1024_int128, 9);
            }
            else
            {
                return false;
            }
            break;

        case 't':
            if(suffix == "tb")
            {
                factor = snapdev::pow(base, 4);
            }
            else if(suffix == "tib")
            {
                factor = snapdev::pow(1024_int128, 4);
            }
            else
            {
                return false;
            }
            break;

        case 'y':
            if(suffix == "yb")
            {
                factor = snapdev::pow(base, 8);
            }
            else if(suffix == "yib")
            {
                factor = snapdev::pow(1024_int128, 8);
            }
            else
            {
                return false;
            }
            break;

        case 'z':
            if(suffix == "zb")
            {
                factor = snapdev::pow(base, 7);
            }
            else if(suffix == "zib")
            {
                factor = snapdev::pow(1024_int128, 7);
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

    double base_number(0.0);
    if(!validator_double::convert_string(number, base_number))
    {
        return false;
    }

    // TODO: I think that even with the long double this will lose bits
    result = static_cast<long double>(base_number) * factor;

    return true;
}
#pragma GCC diagnostic pop



} // namespace advgetopt
// vim: ts=4 sw=4 et
