/*
 * File:
 *    advgetopt/advgetopt_config.cpp -- advanced get option implementation
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

/** \file
 * \brief Advanced getopt data access implementation.
 *
 * The advgetopt class has many function used to access the data in the
 * class. These functions are gathered here.
 */

// self
//
#include    "advgetopt/advgetopt.h"

// advgetopt lib
//
#include    "advgetopt/conf_file.h"
#include    "advgetopt/log.h"

// boost lib
//
#include    <boost/algorithm/string/join.hpp>
#include    <boost/algorithm/string/replace.hpp>


// last include
//
#include <snapdev/poison.h>


namespace advgetopt
{




/** \brief Generate a list of configuration filenames.
 *
 * This function goes through the list of filenames and directories and
 * generates a complete list of all the configuration files that the
 * system will load when you call the parse_configuration_files()
 * function.
 *
 * Set the flag \p exists to true if you only want the name of files
 * that currently exists.
 *
 * The \p writable file means that we only want files under the
 * \<project-name>.d folder and the user configuration folder.
 *
 * \param[in] exists  Remove files that do not exist from the list.
 * \param[in] writable  Only return files we consider writable.
 *
 * \return The list of configuration filenames.
 */
string_list_t getopt::get_configuration_filenames(bool exists, bool writable) const
{
    string_list_t result;

    if(f_options_environment.f_configuration_files != nullptr)
    {
        // load options from configuration files specified as is by caller
        //
        for(char const * const * configuration_files(f_options_environment.f_configuration_files)
          ; *configuration_files != nullptr
          ; ++configuration_files)
        {
            char const * filename(*configuration_files);
            if(*filename != '\0')
            {
                std::string const user_filename(handle_user_directory(filename));
                if(user_filename == filename)
                {
                    if(!writable)
                    {
                        result.push_back(user_filename);
                    }

                    std::string const with_project_name(insert_project_name(user_filename, f_options_environment.f_project_name));
                    if(!with_project_name.empty())
                    {
                        result.push_back(with_project_name);
                    }
                }
                else
                {
                    result.push_back(user_filename);
                }
            }
        }
    }

    if(f_options_environment.f_configuration_filename != nullptr)
    {
        string_list_t directories;
        if(has_flag(GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS))
        {
            if(is_defined("config-dir"))
            {
                size_t const max(size("config-dir"));
                for(size_t idx(0); idx < max; ++idx)
                {
                    directories.push_back(get_string("config-dir", idx));
                }
            }
        }

        if(f_options_environment.f_configuration_directories != nullptr)
        {
            for(char const * const * configuration_directories(f_options_environment.f_configuration_directories)
              ; *configuration_directories != nullptr
              ; ++configuration_directories)
            {
                directories.push_back(*configuration_directories);
            }
        }

        std::string const filename(f_options_environment.f_configuration_filename);

        for(auto directory : directories)
        {
            if(!directory.empty())
            {
                std::string const full_filename(directory + ("/" + filename));
                std::string const user_filename(handle_user_directory(full_filename));
                if(user_filename == full_filename)
                {
                    if(!writable)
                    {
                        result.push_back(user_filename);
                    }

                    std::string const with_project_name(insert_project_name(user_filename, f_options_environment.f_project_name));
                    if(!with_project_name.empty())
                    {
                        result.push_back(with_project_name);
                    }
                }
                else
                {
                    result.push_back(user_filename);
                }
            }
        }
    }

    if(!exists)
    {
        return result;
    }

    string_list_t existing_files;
    int const mode(R_OK | (writable ? W_OK : 0));
    for(auto r : result)
    {
        if(access(r.c_str(), mode) == 0)
        {
            existing_files.push_back(r);
        }
    }
    return existing_files;
}


/** \brief This function checks for arguments in configuration files.
 *
 * Each configuration file is checked one after another. Each file that is
 * defined is loaded and each line is viewed as an option. If valid, it is
 * added to the resulting getopt list of options.
 *
 * Note that it is an error to define a command in a configuration file. If
 * that happens, an error occurs and the process stops. Technically this is
 * defined with the GETOPT_FLAG_CONFIGURATION_FILE flag in your opt table.
 *
 * The list of files is checked from beginning to end. So if a later file
 * changes an option of an earlier file, it is the one effective.
 *
 * The configuration file loader supports a project name as defined in the
 * get_project_name() function. It allows for a sub-directory to
 * be inserted between the path and the basename of the configuration
 * file. This allows for a file to be search in an extra sub-directory
 * so one can avoid changing the original definitions and only use
 * configuration files in the sub-directory. The path looks like this
 * when a project name is specified:
 *
 * \code
 *      <path>/<project name>.d/<basename>
 * \endcode
 *
 * Notice that we add a ".d" as usual in other projects under Linux.
 *
 * \exception getopt_exception_invalid
 * This function generates the getopt_exception_invalid exception whenever
 * something invalid is found in the list of options passed as the \p opts
 * parameter.
 *
 * \exception getopt_exception_default
 * The function detects whether two options are marked as the default
 * option (the one receiving parameters that are not used by another command
 * or match a command.) This exception is raised when such is detected.
 *
 * \sa process_configuration_file()
 */
void getopt::parse_configuration_files()
{
    string_list_t const filenames(get_configuration_filenames(false, false));

    for(auto f : filenames)
    {
        process_configuration_file(f);
    }
}


/** \brief Parse one specific configuration file and process the results.
 *
 * This function reads one specific configuration file using a conf_file
 * object and then goes through the resulting arguments and add them to
 * the options of this getopt object.
 *
 * The options found in the configuration file must match an option by
 * its long name. In a configuration file, it is not allowed to have an
 * option which name is only one character.
 *
 * \note
 * If the filename points to a file which can't be read or does not exist,
 * then nothing happens and the function returns without an error.
 *
 * \todo
 * Extend the support by having the various flags that the conf_file
 * class supports appear in the list of configuration filenames.
 *
 * \param[in] filename  The name of the configuration file to check out.
 *
 * \sa parse_configuration_files()
 */
void getopt::process_configuration_file(std::string const & filename)
{
    conf_file_setup conf_setup(filename);
    if(!conf_setup.is_valid())
    {
        return;
    }
    conf_file::pointer_t conf(conf_file::get_conf_file(conf_setup));

    conf_file::sections_t sections(conf->get_sections());
    if(!sections.empty())
    {
        std::string const name(CONFIGURATION_SECTIONS);
        option_info::pointer_t configuration_sections(get_option(name));
        if(configuration_sections == nullptr)
        {
            configuration_sections = std::make_shared<option_info>(name);
            configuration_sections->add_flag(
                          GETOPT_FLAG_MULTIPLE
                        | GETOPT_FLAG_CONFIGURATION_FILE
                        );
            f_options_by_name[name] = configuration_sections;
        }
        else if(!configuration_sections->has_flag(GETOPT_FLAG_MULTIPLE))
        {
            log << log_level_t::error
                << "option \""
                << name
                << "\" must have GETOPT_FLAG_MULTIPLE set."
                << end;
            return;
        }
        for(auto s : sections)
        {
            configuration_sections->add_value(s);
        }
    }

    for(auto const & param : conf->get_parameters())
    {
        // in configuration files we only allow long arguments
        //
        option_info::pointer_t opt(get_option(param.first));
        if(opt == nullptr)
        {
            if(!has_flag(GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS)
            || param.first.length() == 1)
            {
                log << log_level_t::error
                    << "unknown option \""
                    << param.first
                    << "\" found in configuration file \""
                    << filename
                    << "\"."
                    << end;
                continue;
            }
            else
            {
                // add a new parameter dynamically
                //
                opt = std::make_shared<option_info>(param.first);

                opt->set_flags(GETOPT_FLAG_CONFIGURATION_FILE | GETOPT_FLAG_DYNAMIC);

                // consider the first definition as the default
                // (which is likely in our environment)
                //
                opt->set_default(param.second);

                f_options_by_name[param.first] = opt;
            }
        }
        else
        {
            if(!opt->has_flag(GETOPT_FLAG_CONFIGURATION_FILE))
            {
                // in configuration files we are expected to use '_' so
                // print an error with such
                //
                log << log_level_t::error
                    << "option \""
                    << boost::replace_all_copy(param.first, "-", "_")
                    << "\" is not supported in configuration files (found in \""
                    << filename
                    << "\")."
                    << end;
                continue;
            }
        }

        if(opt != nullptr)
        {
            add_option_from_string(opt, param.second, filename);
        }
    }
}






} // namespace advgetopt
// vim: ts=4 sw=4 et
