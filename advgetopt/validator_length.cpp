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
 * \brief Implementation of the length validator.
 *
 * The length validator allows us to check the number of characters in the
 * value. If less than the minimum or more than the maximum, then it is not
 * considered valid.
 */

// self
//
#include    "advgetopt/validator_length.h"

#include    "advgetopt/validator_integer.h"


// cppthread
//
#include    <cppthread/log.h>


// libutf8
//
#include    <libutf8/libutf8.h>


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



class validator_length_factory
    : public validator_factory
{
public:
    validator_length_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("length");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_length>(data);
    }
};

validator_length_factory         g_validator_length_factory;



} // no name namespace





validator_length::validator_length(string_list_t const & length_list)
{
    range_t range;
    for(auto r : length_list)
    {
        std::string::size_type const pos(r.find("..."));
        if(pos == std::string::npos)
        {
            if(!validator_integer::convert_string(r, range.f_minimum))
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
            std::string min_value(r.substr(0, pos));
            boost::trim(min_value);
            if(!validator_integer::convert_string(min_value, range.f_minimum))
            {
                cppthread::log << cppthread::log_level_t::error
                               << min_value
                               << " is not a valid value for your range's start;"
                                  " it must only be digits, optionally preceeded by a sign (+ or -)"
                                  " and not overflow an int64_t value."
                               << cppthread::end;
                continue;
            }

            std::string max_value(r.substr(pos + 3));
            boost::trim(max_value);
            if(!validator_integer::convert_string(max_value, range.f_maximum))
            {
                cppthread::log << cppthread::log_level_t::error
                               << max_value
                               << " is not a valid value for your range's end;"
                                  " it must only be digits, optionally preceeded by a sign (+ or -)"
                                  " and not overflow an int64_t value."
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
        f_allowed_lengths.push_back(range);
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "length".
 *
 * \return "length".
 */
std::string validator_length::name() const
{
    return std::string("length");
}


/** \brief Check the value against a list of length ranges.
 *
 * This function is used to verify the length of \p value in characters.
 *
 * \param[in] value  The value to be validated.
 *
 * \return true on a match.
 */
bool validator_length::validate(std::string const & value) const
{
    if(f_allowed_lengths.empty())
    {
        return true;
    }

    // get the number of characters assuming the input string is UTF-8
    //
    std::int64_t const length(static_cast<std::int64_t>(libutf8::u8length(value)));
    for(auto f : f_allowed_lengths)
    {
        if(length >= f.f_minimum
        && length <= f.f_maximum)
        {
            return true;
        }
    }
    return false;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
