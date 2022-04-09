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
 * \brief Implementation of the regular expression validator.
 *
 * The regular expression validator allows us to check pretty much any type
 * of value.
 *
 * This validator does not offer a convertor since a regular expression
 * does not really offer such a feature.
 */

// self
//
#include    "advgetopt/validator_regex.h"


// cppthread
//
#include    <cppthread/log.h>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{



class validator_regex_factory
    : public validator_factory
{
public:
    validator_regex_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("regex");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_regex>(data);
    }
};

validator_regex_factory         g_validator_regex_factory;



} // no name namespace





validator_regex::validator_regex(string_list_t const & regex_list)
{
    if(regex_list.size() > 1)
    {
        cppthread::log << cppthread::log_level_t::error
                       << "validator_regex() only supports one parameter; "
                       << regex_list.size()
                       << " were supplied; single or double quotation may be required?"
                       << cppthread::end;
        return;
    }

    std::string regex;
    if(!regex_list.empty())
    {
        regex = regex_list[0];
    }
    std::regex::flag_type flags =  std::regex_constants::extended;
    if(regex.length() >= 2
    && regex[0] == '/')
    {
        auto it(regex.end());
        for(--it; it != regex.begin(); --it)
        {
            if(*it == '/')
            {
                break;
            }
            switch(*it)
            {
            case 'i':
                flags |= std::regex_constants::icase;
                break;

            default:
                cppthread::log << cppthread::log_level_t::error
                               << "unsupported regex flag "
                               << *it
                               << " in regular expression \""
                               << regex
                               << "\"."
                               << cppthread::end;
                break;

            }
        }
        if(it == regex.begin())
        {
            cppthread::log << cppthread::log_level_t::error
                           << "invalid regex definition, ending / is missing in \""
                           << regex
                           << "\"."
                           << cppthread::end;

            f_regex = std::regex(std::string(regex.begin() + 1, regex.end()), flags);
        }
        else
        {
            f_regex = std::regex(std::string(regex.begin() + 1, it), flags);
        }
    }
    else
    {
        f_regex = std::regex(regex, flags);
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "regex".
 *
 * \return "regex".
 */
std::string validator_regex::name() const
{
    return std::string("regex");
}


/** \brief Check the value against a regular expression.
 *
 * This function is used to match the value of an argument against a
 * regular expression. It returns true when it does match.
 *
 * \param[in] value  The value to be validated.
 *
 * \return true on a match.
 */
bool validator_regex::validate(std::string const & value) const
{
    std::smatch info;
    return std::regex_match(value, info, f_regex);
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
