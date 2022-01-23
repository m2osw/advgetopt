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

// advgetopt lib
//
#include    "advgetopt/validator.h"



namespace advgetopt
{



class validator_size
    : public validator
{
public:
    typedef std::uint32_t       flag_t;

    static constexpr flag_t     VALIDATOR_SIZE_DEFAULT_FLAGS    = 0x00;
    static constexpr flag_t     VALIDATOR_SIZE_POWER_OF_TWO     = 0x01;

                                validator_size(string_list_t const & data);

    // validator implementation
    //
    virtual std::string const   name() const;
    virtual bool                validate(std::string const & value) const;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    static bool                 convert_string(std::string const & size
                                             , flag_t flags
                                             , __int128 & result);
#pragma GCC diagnostic pop

private:
    flag_t                      f_flags = VALIDATOR_SIZE_DEFAULT_FLAGS;
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
