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
 * \brief Implementation of the email validator.
 *
 * The email validator allows us to check the input as an email address.
 */

// self
//
#include    "advgetopt/validator_email.h"


// cppthread
//
#include    <cppthread/log.h>


// libtld
//
#include    <libtld/tld.h>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



namespace
{



class validator_email_factory
    : public validator_factory
{
public:
    validator_email_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("email");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_email>(data);
    }
};

validator_email_factory         g_validator_email_factory;



} // no name namespace





validator_email::validator_email(string_list_t const & param_list)
{
    // at this time the tld library does not offer support for
    // flags or anything
    //
    if(param_list.size() > 1)
    {
        cppthread::log << cppthread::log_level_t::error
                       << "validator_email() supports zero or one parameter."
                       << cppthread::end;
        return;
    }

    if(param_list.size() == 1)
    {
        if(param_list[0] == "single")
        {
            f_multiple = false;
        }
        else if(param_list[0] == "multiple")
        {
            f_multiple = true;
        }
        else
        {
            cppthread::log << cppthread::log_level_t::error
                           << "validator_email(): unknown parameter \""
                           << param_list[0]
                           << "\"."
                           << cppthread::end;
        }
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "email".
 *
 * \return "email".
 */
std::string validator_email::name() const
{
    return std::string("email");
}


/** \brief Check the value to make sure emails are considered valid.
 *
 * This function is used to verify the value for a valid email.
 *
 * \param[in] value  The value to be validated.
 *
 * \return true on a match.
 */
bool validator_email::validate(std::string const & value) const
{
    tld_email_list list;
    if(list.parse(value, 0) != TLD_RESULT_SUCCESS)
    {
        return false;
    }

    if(f_multiple)
    {
        return list.count() > 0;
    }
    return list.count() == 1;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
