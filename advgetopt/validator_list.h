// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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
#pragma once

/** \file
 * \brief Declaration of a validator handling multiple sub-validators.
 *
 * The library offers a way to validate the parameters passed on the command
 * line or in configuration files.
 *
 * At times, a parameter can be set to several different types. This
 * validator gives the library this ability.
 */

// self
//
#include    <advgetopt/validator.h>



namespace advgetopt
{



class validator_list
    : public validator
{
public:
    typedef std::shared_ptr<validator_list>      pointer_t;

                                validator_list(string_list_t const & data);

    void                        add_validator(validator::pointer_t v);

    // validator implementation
    //
    virtual std::string         name() const override;
    virtual bool                validate(std::string const & value) const override;

private:
    validator::vector_t         f_validators = validator::vector_t();
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
