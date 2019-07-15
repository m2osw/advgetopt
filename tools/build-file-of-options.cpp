/*
 * File:
 *    tools/build-file-of-options.cpp -- convert a .conf comments into a file
 *                                       of options
 *
 * License:
 *    Copyright (c) 2019  Made to Order Software Corp.  All Rights Reserved
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
 * \brief build-file-of-options tool.
 *
 * We use this tool to convert the comments found in our configuration files
 * in a list of options that can be parsed by the advgetopt objects.
 *
 * Specifically, it understands the option name, default value, and when
 * available some other features such as ranges and types.
 */


// advgetopt lib
//
#include "advgetopt/advgetopt.h"
#include "advgetopt/version.h"

// snapdev lib
//
#include <snapdev/not_reached.h>

// C++ lib
//
#include <iostream>
#include <fstream>
#include <sstream>

// last include
//
#include <snapdev/poison.h>




namespace
{

/** \brief Command line options.
 *
 * This table includes all the command line options supported by the
 * `build-file-of-options` tool.
 */
advgetopt::option const g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("output")
        , advgetopt::ShortName('o')
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("Specify the path and filename of the output file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("verbose")
        , advgetopt::ShortName('v')
        , advgetopt::Flags(advgetopt::option_flags<advgetopt::GETOPT_FLAG_COMMAND_LINE
                                                 , advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE
                                                 , advgetopt::GETOPT_FLAG_CONFIGURATION_FILE>())
        , advgetopt::Help("Show commands being executed.")
    ),
    advgetopt::define_option(
          advgetopt::Name("--")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
        , advgetopt::Help("<configuration filename>")
    ),
    advgetopt::end_options()
};


/** \brief The tool looks for this configuration file.
 *
 * The build-file-of-options allow you to have a configuration file
 * with various options in it.
 */
constexpr char const * const g_configuration_files[]
{
    "/etc/advgetopt/build-file-of-options.conf",
    nullptr
};




// TODO: once we have stdc++20, remove all defaults
#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "build-file-of-options",
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = "BUILD_FILE_OF_OPTIONS_OPTIONS",
    .f_configuration_files = g_configuration_files,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>] <configuration file>\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = LIBADVGETOPT_VERSION_STRING,
    .f_license = nullptr,
    .f_copyright = "Copyright (c) 2019  Made to Order Software Corporation",
    .f_build_date = __DATE__,
    .f_build_time = __TIME__
};





class build_file
{
public:
                                    build_file(int argc, char * argv[]);

    int                             run();

private:
    advgetopt::getopt               f_opt;
};



build_file::build_file(int argc, char * argv[])
    : f_opt(g_options_environment, argc, argv)
{
    if(f_opt.is_defined("help"))
    {
        std::cout << f_opt.usage();
        exit(1);
    }

    if(f_opt.is_defined("version"))
    {
        std::cout << LIBADVGETOPT_VERSION_STRING << std::endl;
        exit(1);
    }
}


int build_file::run()
{
    return 0;
}




} // no name namespace




int main(int argc, char * argv[])
{
    try
    {
        build_file session(argc, argv);
        return session.run();
    }
    catch(std::exception const & e)
    {
        std::cerr << "error: exception caught: " << e.what() << std::endl;
        return 1;
    }
    snap::NOTREACHED();
}


// vim: ts=4 sw=4 et

