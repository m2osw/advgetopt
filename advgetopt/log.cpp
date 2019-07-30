/*
 * License:
 *    Copyright (c) 2006-2019  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/
 *    contact@m2osw.com
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *    Alexis Wilke   alexis@m2osw.com
 *    Doug Barbieri  doug@m2osw.com
 */


/** \file
 * \brief Implementation of the logging facility.
 *
 * The library is very often used by daemons meaning that it will be running
 * on its own in the background. For this reason, all the output is done
 * through the log facility.
 *
 * This defines a function which you are expected to call to setup a callback.
 * By default, the callback is set to a function that simply prints error
 * in std::cerr.
 */

// self
//
#include "advgetopt/log.h"


// advgetopt lib
//
#include "advgetopt/exception.h"


// C++ lib
//
#include    <iostream>


// last include
//
#include <snapdev/poison.h>



namespace advgetopt
{


logger      log;


namespace
{


log_callback        g_log_callback = nullptr;


} // no name namespace



void set_log_callback(log_callback callback)
{
    g_log_callback = callback;
}


logger & logger::end()
{
    if(g_log_callback != nullptr)
    {
        g_log_callback(f_level, f_log.str());
    }
    else
    {
        std::cerr << to_string(f_level) << ": " << f_log.str() << std::endl;
    }

    f_log.str(std::string());

    return *this;
}


std::string to_string(log_level_t level)
{
    switch(level)
    {
    case log_level_t::debug:
        return "debug";

    case log_level_t::info:
        return "info";

    case log_level_t::warning:
        return "warning";

    case log_level_t::error:
        return "error";

    case log_level_t::fatal:
        return "fatal";

    }

    throw getopt_exception_invalid("unknown log level ("
                                 + std::to_string(static_cast<int>(level))
                                 + ")");
}


}   // namespace advgetopt
// vim: ts=4 sw=4 et
