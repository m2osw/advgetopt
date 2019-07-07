/*
 * File:
 *    advgetopt/exception.h -- a replacement to the Unix getopt() implementation
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
 * \brief Definitions of the advanced getopt exceptions.
 *
 * The library generates exceptions which are defined here. These are
 * derived from the libexcept library so we get a stack trace whenever
 * an exception occurs.
 */

#include <libexcept/exception.h>




namespace advgetopt
{



// generic logic error (something's wrong in the library)
class getopt_exception_logic : public libexcept::logic_exception_t
{
public:
    getopt_exception_logic(std::string const & msg) : logic_exception_t(msg) {}
};


// generic getopt exception
class getopt_exception : public libexcept::exception_t
{
public:
    getopt_exception(std::string const & msg) : exception_t(msg) {}
};


// problem with a default argument
class getopt_exception_default : public getopt_exception
{
public:
    getopt_exception_default(std::string const & msg) : getopt_exception(msg) {}
};


// trying to get an undefined option
class getopt_exception_undefined : public getopt_exception
{
public:
    getopt_exception_undefined(std::string const & msg) : getopt_exception(msg) {}
};


// something wrong in the user options
class getopt_exception_invalid : public getopt_exception
{
public:
    getopt_exception_invalid(std::string const & msg) : getopt_exception(msg) {}
};


// some initialization failed at run-time
class getopt_exception_initialization : public getopt_exception
{
public:
    getopt_exception_initialization(std::string const & msg) : getopt_exception(msg) {}
};



}   // namespace advgetopt

// vim: ts=4 sw=4 et
