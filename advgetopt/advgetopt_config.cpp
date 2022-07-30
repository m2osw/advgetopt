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

/** \file
 * \brief Advanced getopt data access implementation.
 *
 * The advgetopt class has many function used to access the data in the
 * class. These functions are gathered here.
 */

// self
//
#include    "advgetopt/advgetopt.h"

#include    "advgetopt/conf_file.h"


// cppthread
//
#include    <cppthread/log.h>

// boost
//
#include    <boost/algorithm/string/join.hpp>
#include    <boost/algorithm/string/replace.hpp>


// last include
//
#include    <snapdev/poison.h>


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
 * \note
 * The argc/argv and environment variable parameters are used whenever the
 * function is called early and we can't call is_defined(). These are
 * ignored otherwise.
 *
 * \param[in] exists  Remove files that do not exist from the list.
 * \param[in] writable  Only return files we consider writable.
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  The arguments passed to the finish_parsing() function or
 * nullptr.
 * \param[in] environment_variable  The environment variable or an empty string.
 *
 * \return The list of configuration filenames.
 */
string_list_t getopt::get_configuration_filenames(
          bool exists
        , bool writable
        , int argc
        , char * argv[]) const
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

                    string_list_t const with_project_name(insert_group_name(
                                  user_filename
                                , f_options_environment.f_group_name
                                , f_options_environment.f_project_name));
                    if(!with_project_name.empty())
                    {
                        result.insert(
                                  result.end()
                                , with_project_name.begin()
                                , with_project_name.end());
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
            if(f_parsed)
            {
                // WARNING: at this point the command line and environment
                //          variable may not be parsed in full if at all
                //
                //if(has_flag(SYSTEM_OPTION_CONFIGURATION_FILENAMES))
                if(is_defined("config-dir"))
                {
                    size_t const max(size("config-dir"));
                    directories.reserve(max);
                    for(size_t idx(0); idx < max; ++idx)
                    {
                        directories.push_back(get_string("config-dir", idx));
                    }
                }
            }
            else
            {
                // we've got to do some manual parsing (argh!)
                //
                directories = find_config_dir(argc, argv);
                if(directories.empty())
                {
                    string_list_t args(split_environment(f_environment_variable));

                    std::vector<char *> sub_argv;
                    sub_argv.resize(args.size() + 2);
                    sub_argv[0] = const_cast<char *>(f_program_fullname.c_str());
                    for(size_t idx(0); idx < args.size(); ++idx)
                    {
                        sub_argv[idx + 1] = const_cast<char *>(args[idx].c_str());
                    }
                    sub_argv[args.size() + 1] = nullptr;
                    
                    directories = find_config_dir(sub_argv.size() - 1, sub_argv.data());
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

                    string_list_t const with_project_name(insert_group_name(user_filename, f_options_environment.f_group_name, f_options_environment.f_project_name));
                    if(!with_project_name.empty())
                    {
                        result.insert(
                                  result.end()
                                , with_project_name.begin()
                                , with_project_name.end());
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


/** \brief Search for the "--config-dir" option in a set of arguments.
 *
 * This function searches the given list of \p argv arguments for the
 * "--config-dir".
 *
 * This is done that way because we prematurely need that information
 * in order to properly search for the configuration file. This is because
 * the "--config-dir" is not yet defined when we attempt to read the
 * user specific configuration file.
 *
 * \param[in] argc  The number of arguments.
 * \param[in] argv  The list of arguments to be searched.
 */
string_list_t getopt::find_config_dir(
          int argc
        , char * argv[])
{
    if(argv == nullptr)
    {
        return string_list_t();
    }

    string_list_t result;
    for(int idx(1); idx < argc; ++idx)
    {
        if(strcmp(argv[idx], "--config-dir") == 0)
        {
            for(++idx; idx < argc; ++idx)
            {
                if(argv[idx][0] == '-')
                {
                    --idx;
                    break;
                }
                result.push_back(argv[idx]);
            }
        }
        else if(strncmp(argv[idx], "--config-dir=", 13) == 0)
        {
            result.push_back(argv[idx] + 13);
        }
    }

    return result;
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
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  The arguments passed to the finish_parsing() function.
 *
 * \sa process_configuration_file()
 * \sa get_configuration_filenames()
 * \sa finish_parsing()
 */
void getopt::parse_configuration_files(int argc, char * argv[])
{
    string_list_t const filenames(get_configuration_filenames(false, false, argc, argv));

    for(auto f : filenames)
    {
        process_configuration_file(f);
        f_parsed = false;
    }

    f_parsed = true;
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
    option_info::set_configuration_filename(filename);

    conf_file_setup conf_setup(filename);
    if(!conf_setup.is_valid())
    {
        // a non-existant file is considered valid now so this should never
        // happen; later we may use the flag if we find errors in the file
        //
        return; // LCOV_EXCL_LINE
    }
    conf_file::pointer_t conf(conf_file::get_conf_file(conf_setup));

    conf_file::sections_t sections(conf->get_sections());

    // is there a variable section?
    //
    if(f_options_environment.f_section_variables_name != nullptr)
    {
        conf->section_to_variables(
                      f_options_environment.f_section_variables_name
                    , f_variables);
    }

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
            f_options_by_name[configuration_sections->get_name()] = configuration_sections;
        }
        else if(!configuration_sections->has_flag(GETOPT_FLAG_MULTIPLE))
        {
            cppthread::log << cppthread::log_level_t::error
                           << "option \""
                           << name
                           << "\" must have GETOPT_FLAG_MULTIPLE set."
                           << cppthread::end;
            return;
        }
        for(auto s : sections)
        {
            if(!configuration_sections->has_value(s))
            {
                configuration_sections->add_value(s, option_source_t::SOURCE_CONFIGURATION);
            }
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
                cppthread::log << cppthread::log_level_t::error
                               << "unknown option \""
                               << boost::replace_all_copy(param.first, "-", "_")
                               << "\" found in configuration file \""
                               << filename
                               << "\" on line "
                               << param.second.get_line()
                               << "."
                               << cppthread::end;
                continue;
            }
            else
            {
                // add a new parameter dynamically
                //
                opt = std::make_shared<option_info>(param.first);
                opt->set_variables(f_variables);

                opt->set_flags(GETOPT_FLAG_CONFIGURATION_FILE | GETOPT_FLAG_DYNAMIC);

                // consider the first definition as the default
                // (which is likely in our environment)
                //
                opt->set_default(param.second);

                f_options_by_name[opt->get_name()] = opt;
            }
        }
        else
        {
            if(!opt->has_flag(GETOPT_FLAG_CONFIGURATION_FILE))
            {
                // in configuration files we are expected to use '_' so
                // print an error with such
                //
                cppthread::log << cppthread::log_level_t::error
                               << "option \""
                               << boost::replace_all_copy(param.first, "-", "_")
                               << "\" is not supported in configuration files (found in \""
                               << filename
                               << "\")."
                               << cppthread::end;
                continue;
            }
        }

        if(opt != nullptr)
        {
            add_option_from_string(
                      opt
                    , param.second
                    , filename
                    , option_source_t::SOURCE_CONFIGURATION);
        }
    }

    f_parsed = true;
}






} // namespace advgetopt
// vim: ts=4 sw=4 et
