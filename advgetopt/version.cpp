/*
 * License:
 *    Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/project/advgetopt
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
 * \brief Advanced getopt version functions.
 *
 * The advgetopt environment is versioned. The functions available here
 * give you access to the version, in case you wanted to make sure you
 * had a minimum version or had some special case options when you
 * want to be able to support various versions.
 */

// self
//
#include    "advgetopt/version.h"


// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{




/** \brief Get the major version of the library
 *
 * This function returns the version of the running library (the
 * one you are linked against at runtime).
 *
 * \return The major version.
 */
int get_major_version()
{
    return LIBADVGETOPT_VERSION_MAJOR;
}


/** \brief Get the release version of the library.
 *
 * This function returns the release version of the running library
 * (the one you are linked against at runtime).
 *
 * \return The release version.
 */
int get_release_version()
{
    return LIBADVGETOPT_VERSION_MINOR;
}


/** \brief Get the patch version of the library.
 *
 * This function returns the patch version of the running library
 * (the one you are linked against at runtime).
 *
 * \return The patch version.
 */
int get_patch_version()
{
    return LIBADVGETOPT_VERSION_PATCH;
}


/** \brief Get the full version of the library as a string.
 *
 * This function returns the major, release, and patch versions of the
 * running library (the one you are linked against at runtime) in the
 * form of a string.
 *
 * The build version is not made available.
 *
 * \return The library version.
 */
char const * get_version_string()
{
    return LIBADVGETOPT_VERSION_STRING;
}


} // advgetopt namespace
// vim: ts=4 sw=4 et
