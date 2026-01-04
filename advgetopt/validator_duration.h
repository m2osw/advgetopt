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

// advgetopt
//
#include    "advgetopt/validator.h"


// C++
//
#include    <cstdint>
#include    <limits>



namespace advgetopt
{



class validator_duration
    : public validator
{
public:
    typedef std::uint32_t       flag_t;

    static constexpr flag_t     VALIDATOR_DURATION_DEFAULT_FLAGS = 0x00;
    static constexpr flag_t     VALIDATOR_DURATION_LONG          = 0x01;

                                validator_duration(string_list_t const & data);

    // validator implementation
    //
    virtual std::string         name() const override;
    virtual bool                validate(std::string const & value) const override;

    static bool                 convert_string(std::string const & duration
                                             , flag_t flags
                                             , double & result);

private:
    struct range_t
    {
        typedef std::vector<range_t>    vector_t;

        double            f_minimum = std::numeric_limits<double>::min();
        double            f_maximum = std::numeric_limits<double>::max();
    };

    flag_t                      f_flags = VALIDATOR_DURATION_DEFAULT_FLAGS;
    range_t::vector_t           f_allowed_values = range_t::vector_t();
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
