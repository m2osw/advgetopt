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

//// libutf8 lib
////
//#include    <libutf8/libutf8.h>

// boost lib
//
#include    <boost/algorithm/string/replace.hpp>

//// C++ lib
////
//#include    <iomanip>



namespace advgetopt
{




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
 * \sa load_configuration_files()
 */
void getopt::parse_configuration_files()
{
    if(f_options_environment.f_configuration_files == nullptr)
    {
        return;
    }

    // load options from configuration files specified as is by caller
    //
    for(char const * const * configuration_files(f_options_environment.f_configuration_files)
      ; *configuration_files != nullptr
      ; ++configuration_files)
    {
        char const * filename(*configuration_files);
        if(*filename != '\0')
        {
            load_configuration_files(filename);
        }
    }
}


/** \brief Try loading a configuration file.
 *
 * This function attempts to load a configuration file with the specified
 * filename and if a project name is defined, also with that project name
 * added with ".d" appended like so:
 *
 * \code
 *     <filename path>/<project name>.d/<filename basename>
 * \endcode
 *
 * If you are managing your own ".d" feature, make sure to call the
 * process_configuration_file() function directly.
 *
 * The name of the file is not currently modified. It should already
 * include the necessary extension such as ".conf" or ".ini".
 *
 * \todo
 * Added more locations: we want to support a system configuration file
 * under `/etc/<project-name>/`, and also a user defined configuration
 * location under `~/.config/<project-name>/`. There could be others
 * that should be automatic. Right now you can handle all of these
 * with a list of configuration path, but automation is prime.
 *
 * \param[in] filename  The basic name of the configuration file.
 *
 * \sa process_configuration_file()
 * \sa parse_configuration_files()
 */
void getopt::load_configuration_files(std::string const & filename)
{
    process_configuration_file(filename);

    if(f_options_environment.f_project_name != nullptr
    && *f_options_environment.f_project_name != '\0')
    {
        std::string adjusted_filename(filename);

        std::string::size_type const pos(adjusted_filename.find_last_of('/'));
        if(pos != std::string::npos
        && pos > 0)
        {
            adjusted_filename = adjusted_filename.substr(0, pos + 1)
                              + f_options_environment.f_project_name
                              + ".d"
                              + adjusted_filename.substr(pos);
        }
        else
        {
            adjusted_filename = f_options_environment.f_project_name
                              + (".d/" + adjusted_filename);
        }

        process_configuration_file(adjusted_filename);
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
 * \sa load_configuration_files()
 */
void getopt::process_configuration_file(std::string const & filename)
{
    conf_file conf(filename);

    for(auto const & param : conf.get_parameters())
    {
        // in configuration files we only allow long arguments
        //
        option_info::pointer_t opt(get_option(param.first));
        if(opt == nullptr)
        {
            if((f_options_environment.f_environment_flags & GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS) == 0
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

                f_options->add_child(opt);
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

