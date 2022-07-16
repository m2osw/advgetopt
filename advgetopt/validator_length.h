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
 * \brief Declaration of the length validator.
 *
 * The advgetopt library offers a length validator which verifies that the
 * parameter has that many characters.
 *
 * The length are written as ranges separated by commas (,):
 *
 * \code
 *     length(1...100,128)
 * \endcode
 *
 * A number by itself means the length can be that number of characters or
 * less.
 */

// self
//
#include    <advgetopt/validator.h>



namespace advgetopt
{



class validator_length
    : public validator
{
public:
                                validator_length(string_list_t const & data);

    // validator implementation
    //
    virtual std::string         name() const override;
    virtual bool                validate(std::string const & value) const override;

private:
    struct range_t
    {
        typedef std::vector<range_t>    vector_t;

        std::int64_t            f_minimum = std::numeric_limits<std::int64_t>::min();
        std::int64_t            f_maximum = std::numeric_limits<std::int64_t>::max();
    };

    range_t::vector_t           f_allowed_lengths = range_t::vector_t();
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
