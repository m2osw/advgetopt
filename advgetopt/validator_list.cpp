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
 * \brief Implementation of the list validator.
 *
 * The list validator allows the library to run multiple validators against
 * parameters to easily allow for multiple possible values.
 *
 * This is especially useful if a parameter supports a value such as an
 * integer and a few keywords (i.e. "off", "disabled", "maximum", etc.)
 */

// self
//
#include    "advgetopt/validator_list.h"


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



class validator_list_factory
    : public validator_factory
{
public:
    validator_list_factory()
    {
        validator::register_validator(*this);
    }

    virtual std::string get_name() const override
    {
        return std::string("list");
    }

    virtual std::shared_ptr<validator> create(string_list_t const & data) const override
    {
        return std::make_shared<validator_list>(data);
    }
};

validator_list_factory         g_validator_list_factory;



} // no name namespace





validator_list::validator_list(string_list_t const & param_list)
{
    if(!param_list.empty())
    {
        cppthread::log << cppthread::log_level_t::error
                       << "validator_list() does not support any parameter."
                       << cppthread::end;
        return;
    }
}


void validator_list::add_validator(validator::pointer_t v)
{
    if(v != nullptr)
    {
        f_validators.push_back(v);
    }
}


/** \brief Return the name of this validator.
 *
 * This function returns "list".
 *
 * \return "list".
 */
std::string validator_list::name() const
{
    return std::string("list");
}


/** \brief Check the value against all the validators for one valid one.
 *
 * This function goes through the validators registered with it and if
 * at least one of these validators return true, then the function
 * considered that input value as valid and it returns true.
 *
 * \param[in] value  The value to be validated.
 *
 * \return true on a match.
 */
bool validator_list::validate(std::string const & value) const
{
    for(auto const & v : f_validators)
    {
        if(v->validate(value))
        {
            return true;
        }
    }

    return false;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
