// Copyright (c) 2006-2022  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Definitions of the advanced getopt class.
 *
 * The advgetopt library offers an advanced way to manage your command
 * line tools options on the command line, in environment variables, and
 * in configuration files.
 */

// advgetopt
//
#include    <advgetopt/option_info.h>
#include    <advgetopt/options.h>
#include    <advgetopt/validator.h>
#include    <advgetopt/variables.h>


// C++
//
#include    <limits>
#include    <map>
#include    <memory>
#include    <ostream>
#include    <vector>



namespace advgetopt
{


constexpr char const        CONFIGURATION_SECTIONS[] = "configuration_sections";


constexpr flag_t            SYSTEM_OPTION_NONE                          = 0x0000;

// system commands
constexpr flag_t            SYSTEM_OPTION_HELP                          = 0x0001;
constexpr flag_t            SYSTEM_OPTION_VERSION                       = 0x0002;
constexpr flag_t            SYSTEM_OPTION_COPYRIGHT                     = 0x0004;
constexpr flag_t            SYSTEM_OPTION_LICENSE                       = 0x0008;
constexpr flag_t            SYSTEM_OPTION_BUILD_DATE                    = 0x0010;
constexpr flag_t            SYSTEM_OPTION_ENVIRONMENT_VARIABLE_NAME     = 0x0020;
constexpr flag_t            SYSTEM_OPTION_CONFIGURATION_FILENAMES       = 0x0040;
constexpr flag_t            SYSTEM_OPTION_PATH_TO_OPTION_DEFINITIONS    = 0x0080;
constexpr flag_t            SYSTEM_OPTION_SHOW_OPTION_SOURCES           = 0x0100;
constexpr flag_t            SYSTEM_OPTION_SHOW_OPTION_VALUE             = 0x0200;

// system options
constexpr flag_t            SYSTEM_OPTION_CONFIG_DIR                    = 0x1000;

constexpr flag_t            SYSTEM_OPTION_COMMANDS_MASK                 = 0x0FFF;
constexpr flag_t            SYSTEM_OPTION_OPTIONS_MASK                  = 0xF000;



class getopt
{
public:
    typedef std::shared_ptr<getopt>     pointer_t;

                            getopt(options_environment const & opts);
                            getopt(options_environment const & opts
                                 , int argc
                                 , char * argv[]);

    options_environment const &
                            get_options_environment() const;
    bool                    has_flag(flag_t flag) const;
    void                    reset();

    void                    parse_options_info(
                                      option const * opts
                                    , bool ignore_duplicates = false);
    void                    parse_options_from_file(
                                      std::string const & filename
                                    , int min_sections
                                    , int max_sections
                                    , bool ignore_duplicates = false);
    void                    add_option(
                                      option_info::pointer_t opt
                                    , bool ignore_duplicates = false);
    void                    link_aliases();
    void                    set_short_name(
                                      std::string const & name
                                    , short_name_t short_name);

    void                    finish_parsing(int argc, char * argv[]);

    void                    parse_program_name(char * argv[]);

    void                    parse_configuration_files(
                                      int argc = 0
                                    , char * argv[] = nullptr);
    void                    process_configuration_file(std::string const & filename);

    std::string             get_environment_variable() const;
    static string_list_t    split_environment(std::string const & environment);
    void                    parse_environment_variable();

    void                    parse_string(
                                      std::string const & str
                                    , option_source_t source = option_source_t::SOURCE_DIRECT
                                    , bool only_environment_variable = false);
    void                    parse_arguments(
                                      int argc
                                    , char * argv[]
                                    , option_source_t source = option_source_t::SOURCE_DIRECT
                                    , bool only_environment_variable = false);

    flag_t                  process_system_options(std::basic_ostream<char> & out);

    option_info::map_by_name_t const &
                            get_options() const;
    option_info::pointer_t  get_option(
                                      std::string const & name
                                    , bool exact_option = false) const;
    option_info::pointer_t  get_option(
                                      short_name_t name
                                    , bool exact_option = false) const;
    std::string             options_to_string(
                                      bool include_progname = false
                                    , bool keep_defaults = false) const;
    bool                    is_defined(std::string const & name) const;
    std::size_t             size(std::string const & name) const;
    bool                    has_default(std::string const & name) const;
    std::string             get_default(std::string const & name) const;
    long                    get_long(
                                      std::string const & name
                                    , int idx = 0
                                    , long min = std::numeric_limits<long>::min()
                                    , long max = std::numeric_limits<long>::max()) const;
    double                  get_double(
                                      std::string const & name
                                    , int idx = 0
                                    , double min = std::numeric_limits<double>::min()
                                    , double max = std::numeric_limits<double>::max()) const;
    std::string             get_string(
                                      std::string const & name
                                    , int idx = 0
                                    , bool raw = false) const;
    std::string             operator [] (std::string const & name) const;
    option_info_ref         operator [] (std::string const & name);

    std::string             get_program_name() const;
    std::string             get_program_fullname() const;
    std::string             get_project_name() const;
    std::string             get_group_name() const;
    std::string             get_group_or_project_name() const;
    void                    define_environment_variable_data();
    std::string             get_environment_variable_name() const;
    std::string             get_options_filename() const;
    size_t                  get_configuration_filename_size() const;
    std::string             get_configuration_filename(int idx) const;
    string_list_t           get_configuration_filenames(
                                      bool exists
                                    , bool writable
                                    , int argc = 0
                                    , char * argv[] = nullptr) const;

    group_description const *
                            find_group(flag_t group) const;
    std::string             usage(flag_t show = GETOPT_FLAG_SHOW_MOST) const;
    std::string             process_help_string(char const * help) const;

    variables::pointer_t    get_variables() const;

private:
    void                    initialize_parser(options_environment const & opt_env);
    void                    parse_options_from_group_names();
    void                    parse_options_from_file();
    void                    show_option_sources(std::basic_ostream<char> & out);
    option_info::pointer_t  get_alias_destination(option_info::pointer_t opt) const;
    void                    is_parsed() const;
    static string_list_t    find_config_dir(int argc, char * argv[]);
    static void             add_configuration_filename(string_list_t & names, std::string const & add);
    void                    get_managed_configuration_filenames(
                                  string_list_t & names
                                , bool writable
                                , int argc
                                , char * argv[]) const;
    void                    get_direct_configuration_filenames(
                                  string_list_t & names
                                , bool writable) const;

    void                    add_options(
                                      option_info::pointer_t opt
                                    , int & i
                                    , int argc
                                    , char ** argv
                                    , option_source_t source = option_source_t::SOURCE_DIRECT);
    void                    add_option_from_string(
                                      option_info::pointer_t opt
                                    , std::string const & value
                                    , std::string const & filename
                                    , option_source_t source = option_source_t::SOURCE_DIRECT);

    std::string                         f_program_fullname = std::string();
    std::string                         f_program_name = std::string();

    options_environment                 f_options_environment = options_environment();
    option_info::map_by_name_t          f_options_by_name = option_info::map_by_name_t();
    option_info::map_by_short_name_t    f_options_by_short_name = option_info::map_by_short_name_t();
    option_info::pointer_t              f_default_option = option_info::pointer_t();
    std::string                         f_environment_variable = std::string();
    variables::pointer_t                f_variables = variables::pointer_t();
    bool                                f_parsed = false;
};




}   // namespace advgetopt
// vim: ts=4 sw=4 et
