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
#include    <advgetopt/validator.h>


// C++
//
#include    <cstdint>
#include    <limits>



namespace advgetopt
{



class validator_integer
    : public validator
{
public:
    typedef bool (*to_integer_t)(std::string const & number
                               , std::int64_t & result);

                                validator_integer(string_list_t const & data);

    // validator implementation
    //
    virtual std::string         name() const override;
    virtual bool                validate(std::string const & value) const override;

    static bool                 convert_string(std::string const & number
                                             , std::int64_t & result);

private:
    struct range_t
    {
        typedef std::vector<range_t>    vector_t;

        std::int64_t            f_minimum = std::numeric_limits<std::int64_t>::min();
        std::int64_t            f_maximum = std::numeric_limits<std::int64_t>::max();
    };

    range_t::vector_t           f_allowed_values = range_t::vector_t();
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
