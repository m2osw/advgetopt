/*
 * File:
 *    tests/main.h
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
 */
#pragma once


// advgetopt lib
//
#include <advgetopt/advgetopt.h>
#include <advgetopt/log.h>

// catch lib
//
#include <catch2/snapcatch2.hpp>

// C++ lib
//
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>


namespace SNAP_CATCH2_NAMESPACE
{

extern std::string                  g_tmp_dir;

void                                push_expected_log(std::string const & message);
void                                log_for_test(advgetopt::log_level_t level, std::string const & message);


class obj_setenv
{
public:
    obj_setenv(const std::string& var)
        : f_copy(strdup(var.c_str()))
    {
        putenv(f_copy);
        std::string::size_type p(var.find_first_of('='));
        f_name = var.substr(0, p);
    }
    obj_setenv(obj_setenv const & rhs) = delete;
    obj_setenv & operator = (obj_setenv const & rhs) = delete;
    ~obj_setenv()
    {
        putenv(strdup((f_name + "=").c_str()));
        free(f_copy);
    }

private:
    char *          f_copy = nullptr;
    std::string     f_name = std::string();
};



}
// vim: ts=4 sw=4 et
