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
#pragma once

/** \file
 * \brief Declaration of validators which can be used to verify the parameters.
 *
 * The library offers parameter validations using validator objects. You
 * can even make your own validator objects available before parsing your
 * data so that way it can be verified as expected.
 *
 * Validators are recognized by name. A value can be assigned a validator
 * by specify the \em type of data it supports.
 */

// self
//
#include    <advgetopt/utils.h>


// C++
//
#include    <memory>



namespace advgetopt
{



class validator;

class validator_factory
{
public:
    virtual                     ~validator_factory();

    virtual std::string         get_name() const = 0;
    virtual std::shared_ptr<validator>
                                create(string_list_t const & data) const = 0;
};


class validator
{
public:
    typedef std::shared_ptr<validator>      pointer_t;
    typedef std::vector<pointer_t>          vector_t;

    virtual                     ~validator();

    // virtuals
    //
    virtual std::string         name() const = 0;
    virtual bool                validate(std::string const & value) const = 0;

    static void                 register_validator(validator_factory const & factory);
    static pointer_t            create(std::string const & name, string_list_t const & data);
    static pointer_t            create(std::string const & name_and_params);
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
