/*
 * File:
 *    advgetopt/log.h -- a replacement to the Unix getopt() implementation
 *
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
#pragma once

/** \file
 * \brief Declaration of the log class used to send error messages.
 *
 * The library offers a log facility for when messages are generated on
 * errors and various output (i.e. --help).
 */

// C++ lib
//
#include    <iostream>
#include    <sstream>


namespace advgetopt
{


enum class log_level_t
{
    debug,
    info,
    warning,
    error,
    fatal
};

std::string to_string(log_level_t level);


typedef void (*log_callback)(log_level_t, std::string const & message);

void set_log_callback(log_callback callback);


class logger
{
public:
    template<typename T>
    logger & operator << (T const & v)
    {
        f_log << v;
        return *this;
    }

    logger & operator << (log_level_t const & level)
    {
        f_level = level;
        return *this;
    }

    logger & operator << (logger & (*func)(logger &))
    {
        func(*this);
        return *this;
    }

    logger &            end();

private:
    log_level_t         f_level = log_level_t::error;
    std::stringstream   f_log = std::stringstream();
};


inline logger & end(logger & l) { return l.end(); }


extern logger   log;


}   // namespace advgetopt
// vim: ts=4 sw=4 et
