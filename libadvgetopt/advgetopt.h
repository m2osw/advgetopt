/*
 * File:
 *    libadvgetopt/advgetopt.h -- a replacement to the Unix getopt() implementation
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
 * \brief Definitions of the advanced getopt class.
 *
 * The library offers an advanced way to parse command line arguments
 * and configuration files in a seamless manner. This class is what is
 * used all around for that purpose.
 */

#include    <limits>
#include    <map>
#include    <memory>
#include    <vector>


namespace advgetopt
{




// TODO:
// to avoid turning off the -Weffc++ warning, we should look into not having
// a bare pointer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

// getopt is a advanced class that parses your command line
// arguments and make them accessible by name via a
// standard map
class getopt
{
public:
    typedef std::shared_ptr<getopt>     pointer_t;

    enum class status_t
    {
        no_error,
        no_error_nobr,	// Do not insert \n to long help lines
        warning,
        error,
        fatal
    };

    static const unsigned char GETOPT_FLAG_ENVIRONMENT_VARIABLE = 0x01;
    static const unsigned char GETOPT_FLAG_CONFIGURATION_FILE   = 0x02;
    static const unsigned char GETOPT_FLAG_SHOW_USAGE_ON_ERROR  = 0x04;
    static const unsigned char GETOPT_FLAG_ALIAS                = 0x08;

    enum class argument_mode_t
    {
        no_argument,
        required_argument,
        optional_argument,
        required_multiple_argument,
        optional_multiple_argument,
        required_long,
        optional_long,
        required_multiple_long,
        optional_multiple_long,
        default_argument,
        default_multiple_argument,
        help_argument,
        end_of_options
    };

    struct option
    {
        char                f_opt = '\0';           // letter option (or '\0')
        unsigned char       f_flags = 0;            // set of flags
        char const *        f_name = nullptr;       // name of the option (i.e. "test" for --test, or nullptr)
        char const *        f_default = nullptr;    // a default value if not nullptr
        char const *        f_help = nullptr;       // help for this option, if nullptr it's a hidden option; if ALIAS then this is the actual alias
        argument_mode_t     f_arg_mode = argument_mode_t::no_argument;
    };

                        getopt(int argc, char * argv[], option const * opts, std::vector<std::string> const configuration_files, char const * environment_variable_name);

    void                reset(int argc, char * argv[], option const * opts, std::vector<std::string> const configuration_files, char const * environment_variable_name);

    bool                is_defined(std::string const & name) const;
    int                 size(std::string const & name) const;
    char const *        get_default(std::string const & name) const;
    long                get_long(std::string const & name, int idx = 0, long min = std::numeric_limits<long>::min(), long max = std::numeric_limits<long>::max());
    std::string         get_string(std::string const & name, int idx = 0) const;
    std::string         get_program_name() const;
    std::string         get_program_fullname() const;
    std::string         get_project_name() const;
    void                usage(status_t status, char const * msg, ...);

private:
    struct optmap_info
    {
        bool                        f_cvt = false;  // whether f_int is defined (true) or not (false)
        int32_t                     f_idx = 0;      // index in f_options
        std::vector<long>           f_int = std::vector<long>();
        std::vector<std::string>    f_val = std::vector<std::string>();
    };
    typedef std::map<std::string, optmap_info>  optmap_t;
    typedef std::map<char, int>                 short_opt_name_map_t;
    typedef std::map<std::string, int>          long_opt_name_map_t;

    void                parse_arguments(int argc, char * argv[], option const * opts, int def_opt,
                                        short_opt_name_map_t opt_by_short_name,
                                        long_opt_name_map_t opt_by_long_name,
                                        bool only_environment_variable);
    void                add_options(option const * opt, int & i, int argc, char ** argv);
    void                add_option(option const * opt, char const * value);
    std::string         assemble_options( status_t status /*, std::string & default_arg_help*/ ) const;
    std::string         process_help_string( char const * help ) const;

    std::string         f_program_fullname = std::string();
    std::string         f_program_name = std::string();
    std::string         f_project_name = std::string();
    option const *      f_options = nullptr;
    optmap_t            f_map = optmap_t();
};
#pragma GCC diagnostic pop



}   // namespace advgetopt

// vim: ts=4 sw=4 et
