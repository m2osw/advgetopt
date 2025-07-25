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
 * \brief Definitions of the advanced getopt exceptions.
 *
 * The library generates exceptions which are defined here. These are
 * derived from the libexcept library so we get a stack trace whenever
 * an exception occurs.
 */

// libexcept
//
#include    <libexcept/exception.h>




namespace advgetopt
{



// generic logic error (something's wrong in the library)
//
DECLARE_LOGIC_ERROR(getopt_logic_error);


// generic getopt exception
//
DECLARE_MAIN_EXCEPTION(getopt_exception);


// various problems
//
DECLARE_EXCEPTION(getopt_exception, getopt_defined_twice);
DECLARE_EXCEPTION(getopt_exception, getopt_initialization);
DECLARE_EXCEPTION(getopt_exception, getopt_invalid);
DECLARE_EXCEPTION(getopt_exception, getopt_invalid_default);
DECLARE_EXCEPTION(getopt_exception, getopt_invalid_parameter);
DECLARE_EXCEPTION(getopt_exception, getopt_root_filename);
DECLARE_EXCEPTION(getopt_exception, getopt_undefined);



constexpr int const     CONFIGURATION_EXIT_CODE = 9;


// the process is viewed as done, exit now
class getopt_exit
    : public getopt_exception
{
public:
    getopt_exit(std::string const & msg, int code)
        : getopt_exception(msg)
        , f_code(code)
    {
        set_parameter("exit_code", std::to_string(code));
    }

    int code() const
    {
        return f_code;
    }

private:
    int             f_code = 1;
};




} // namespace advgetopt
// vim: ts=4 sw=4 et
