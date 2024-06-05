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
 * \brief Declaration of the keywords validator.
 *
 * The advgetopt library offers a keywords validator which makes it really
 * simple to verify that a command line option matches an exact word without
 * having to handle a regular expression. It will also be slightly faster.
 *
 * The keywords are written as parameters separated by commas (,):
 *
 * \code
 *     keywords(on,off)
 * \endcode
 */

// self
//
#include    <advgetopt/validator.h>



namespace advgetopt
{



class validator_keywords
    : public validator
{
public:
                                validator_keywords(string_list_t const & data);

    // validator implementation
    //
    virtual std::string         name() const override;
    virtual bool                validate(std::string const & value) const override;

private:
    string_set_t                f_keywords = string_set_t();
};



}   // namespace advgetopt
// vim: ts=4 sw=4 et
