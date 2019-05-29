/*
 * File:
 *    advgetopt/advgetopt.h -- a replacement to the Unix getopt() implementation
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
 * The advgetopt library offers an advanced way to manage your command
 * line tools options on the command line, in environment variables, and
 * in configuration files.
 */

// advgetopt lib
//
#include    "advgetopt/option_info.h"
#include    "advgetopt/options.h"
#include    "advgetopt/validator.h"

// C++ lib
//
#include    <limits>
#include    <map>
#include    <memory>
#include    <vector>


namespace advgetopt
{




class getopt
{
public:
    typedef std::shared_ptr<getopt>     pointer_t;

                            getopt(options_environment const & opts
                                 , int argc
                                 , char * argv[]);

    void                    reset();

    void                    parse_string(
                                      std::string const & str
                                    , bool only_environment_variable);
    void                    parse_arguments(
                                      int argc
                                    , char * argv[]
                                    , bool only_environment_variable);
    void                    parse_options_info(
                                      option const * opts
                                    , bool ignore_duplicates);

    option_info::pointer_t  get_option(std::string const & name) const;
    option_info::pointer_t  get_option(short_name_t name) const;
    bool                    is_defined(std::string const & name) const;
    size_t                  size(std::string const & name) const;
    std::string             get_default(std::string const & name) const;
    long                    get_long(
                                      std::string const & name
                                    , int idx = 0
                                    , long min = std::numeric_limits<long>::min()
                                    , long max = std::numeric_limits<long>::max());
    std::string             get_string(std::string const & name, int idx = 0) const;

    std::string             get_program_name() const;
    std::string             get_program_fullname() const;
    std::string             get_project_name() const;
    std::string             get_environment_variable_name() const;
    size_t                  get_configuration_filename_size() const;
    std::string             get_configuration_filename(int idx) const;

    std::string             usage(flag_t show = GETOPT_FLAG_SHOW_ALL) const;
    std::string             process_help_string(char const * help) const;
    static std::string      breakup_line(
                                      std::string line
                                    , size_t const option_width
                                    , size_t const line_width);
    static std::string      format_usage_string(
                                      std::string const & argument
                                    , std::string const & help
                                    , size_t const option_width
                                    , size_t const line_width);

private:
    void                    parse_program_name(char * argv[]);
    void                    parse_options_from_file();
    void                    parse_configuration_files();
    void                    parse_environment_variable();

    void                    link_aliases();

    void                    process_configuration_file(std::string const & filename);

    void                    add_options(option_info::pointer_t opt
                                      , int & i
                                      , int argc
                                      , char ** argv);
    void                    add_option(option_info::pointer_t opt
                                     , std::string const & value);

    std::string             f_program_fullname = std::string();
    std::string             f_program_name = std::string();

    options_environment     f_options_environment = options_environment();
    option_info::pointer_t  f_options = option_info::pointer_t();
    option_info::pointer_t  f_default_option = option_info::pointer_t();
};




}   // namespace advgetopt
// vim: ts=4 sw=4 et
