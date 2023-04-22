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
#include    "advgetopt/exception.h"


// cppthread
//
#include    <cppthread/log.h>


// C
//
#include    <string.h>
#include    <unistd.h>


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

    get_managed_configuration_filenames(result, writable, argc, argv);
    get_direct_configuration_filenames(result, writable);

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


/** \brief Add one configuration filename to our list.
 *
 * This function adds the specified \p add name to the \p names list unless
 * already present in the list.
 *
 * Several of the functions computing configuration filenames can end up
 * attempting to add the same filename multiple times. This function
 * prevents the duplication. This also means the order may be slightly
 * different than expected (i.e. the filenames don't get reordered when
 * a duplicate is found).
 *
 * \param[in,out] names  The list of configuration names.
 * \param[in] add  The new configuration filename to add.
 */
void getopt::add_configuration_filename(string_list_t & names, std::string const & add)
{
    if(std::find(names.begin(), names.end(), add) == names.end())
    {
        names.push_back(add);
    }
}


/** \brief Generate the list of managed configuration filenames.
 *
 * As the programmer, you can define a configuration filename and a set
 * of directory names. This function uses that information to generate
 * a list of full configuration filenames that is then used to load
 * those configurations.
 *
 * If a filename is defined, but no directories, the this function
 * defines three default paths like so:
 *
 * \li `/usr/share/advgetopt/options/\<name>`
 * \li `/usr/share/\<name>/options`
 * \li `/etc/\<name>`
 *
 * \param[in,out] names  The list of names are added to this list.
 * \param[in] writable  Whether the destination has to be writable.
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  The command line arguments.
 */
void getopt::get_managed_configuration_filenames(
      string_list_t & names
    , bool writable
    , int argc
    , char * argv[]) const
{
    if(f_options_environment.f_configuration_filename == nullptr
    || *f_options_environment.f_configuration_filename == '\0')
    {
        return;
    }

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
                std::size_t const max(size("config-dir"));
                directories.reserve(max);
                for(std::size_t idx(0); idx < max; ++idx)
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
                for(std::size_t idx(0); idx < args.size(); ++idx)
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
        for(char const * const * configuration_directories(f_options_environment.f_configuration_directories);
            *configuration_directories != nullptr;
            ++configuration_directories)
        {
            directories.push_back(*configuration_directories);
        }
    }

    if(directories.empty())
    {
        std::string const name(get_group_or_project_name());

        if(!name.empty())
        {
            std::string directory_name("/usr/share/advgetopt/options/");
            directory_name += name;
            directories.push_back(directory_name);

            directory_name = "/usr/share/";
            directory_name += name;
            directory_name += "/options";
            directories.push_back(directory_name);

            directory_name = "/etc/";
            directory_name += name;
            directories.push_back(directory_name);
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
                    add_configuration_filename(names, user_filename);
                }

                string_list_t const with_project_name(insert_group_name(
                                      user_filename
                                    , f_options_environment.f_group_name
                                    , f_options_environment.f_project_name));
                if(!with_project_name.empty())
                {
                    for(auto const & n : with_project_name)
                    {
                        add_configuration_filename(names, n);
                    }
                }
            }
            else
            {
                add_configuration_filename(names, user_filename);
            }
        }
    }
}


/** \brief Define the list of direct configuration filenames.
 *
 * We generate two lists of configurations: a managed list and a direct
 * configuration list. The managed list is created with the
 * get_managed_configuration_filenames(). The direct list is created with
 * this function and the list of filenames defined in the
 * f_configuration_files list of paths.
 *
 * In this case, the paths defined in that list are directly used. No
 * additional directory are added, except for the sub-directory to allow
 * for administrator files to be edited (i.e. `\<name>.d/??-filename.conf`).
 *
 * \param[in,out] names  The list of configuration filenames.
 * \param[in] writable  Whether only writable filenames get added.
 */
void getopt::get_direct_configuration_filenames(
      string_list_t & names
    , bool writable) const
{
    if(f_options_environment.f_configuration_files == nullptr)
    {
        return;
    }

    // load options from configuration files specified as is by the programmer
    //
    for(char const * const * configuration_files(f_options_environment.f_configuration_files);
        *configuration_files != nullptr;
        ++configuration_files)
    {
        char const * filename(*configuration_files);
        if(*filename == '\0')
        {
            continue;
        }

        std::string const user_filename(handle_user_directory(filename));
        if(user_filename == filename)
        {
            if(!writable)
            {
                add_configuration_filename(names, user_filename);
            }

            string_list_t const with_project_name(insert_group_name(
                                  user_filename
                                , f_options_environment.f_group_name
                                , f_options_environment.f_project_name));
            if(!with_project_name.empty())
            {
                for(auto const & n : with_project_name)
                {
                    add_configuration_filename(names, n);
                }
            }
        }
        else
        {
            add_configuration_filename(names, user_filename);
        }
    }
}


/** \brief Determine the best suited file for updates.
 *
 * This function determines the best suited filename where an administrator
 * is expected to save his changes. For some tools, there may be many
 * choices. This function looks for the last entry since that last entry
 * will allow the administrator to override anything defined prior to
 * this last entry.
 *
 * The search first uses the direct configuration filenames if these are
 * defined. It uses the last directory which does not start with a tilde
 * (i.e. no user file).
 *
 * If the direct configuration is not defined in that process, we next
 * test with the managed configuration filenames. We again look for the
 * last path and that along the last configuration filename.
 *
 * If all of that fails, we build a name from "/etc/" the project name,
 * and use the project name plus ".conf" for the filename:
 *
 * \code
 *     "/etc/" + project_name + "/" + project_name + ".conf"
 * \endcode
 *
 * then pass that file to the default_group_name() function. The result
 * is what gets returned.
 *
 * \return The file the administrator is expected to edit to make changes
 * to the configuration of the given project.
 */
std::string getopt::get_output_filename() const
{
    if(f_options_environment.f_configuration_files != nullptr)
    {
        // check the programmer defined paths as is
        //
        char const * found(nullptr);
        for(char const * const * configuration_files(f_options_environment.f_configuration_files);
            *configuration_files != nullptr;
            ++configuration_files)
        {
            char const * filename(*configuration_files);
            if(*filename == '\0')
            {
                // ignore empty filenames
                //
                continue;
            }

            if(filename[0] == '~'
            && (filename[1] == '/' || filename[1] == '\0'))
            {
                // ignore user directory entries
                //
                continue;
            }

            // we want the last one, so we are not done once we found one...
            //
            found = filename;
        }

        if(found != nullptr)
        {
            return default_group_name(
                  found
                , f_options_environment.f_group_name
                , f_options_environment.f_project_name);
        }
    }

    if(f_options_environment.f_configuration_filename != nullptr
    && *f_options_environment.f_configuration_filename != '\0')
    {
        // check the directories either defined by the programmer or if
        // none defined by the programmer, as defined by advgetopt which
        // in this case simply means "/etc/"; we ignore the possible
        // use of the --config-dir because in that case the administrator
        // knows where to save his file
        //
        std::string const name(get_group_or_project_name());

        std::string directory;
        if(f_options_environment.f_configuration_directories != nullptr)
        {
            for(char const * const * configuration_directories(f_options_environment.f_configuration_directories);
                *configuration_directories != nullptr;
                ++configuration_directories)
            {
                char const * dir(*configuration_directories);
                if(dir[0] == '\0')
                {
                    continue;
                }

                if(dir[0] == '~'
                && (dir[1] == '/' || dir[1] == '\0'))
                {
                    continue;
                }

                // we want to keep the last entry unless it starts with ~/...
                //
                directory = dir;
            }
        }

        if(directory.empty())
        {
            // no programmer defined directory, use a system defined one
            // instead
            //
            directory = "/etc/" + name;
        }

        if(directory.back() != '/')
        {
            directory += '/';
        }

        std::string const filename(directory + f_options_environment.f_configuration_filename);

        return default_group_name(
              filename
            , f_options_environment.f_group_name
            , f_options_environment.f_project_name);
    }

    // the programmer did not define anything, it is likely that no files
    // will be loaded but we still generate a default name
    //
    std::string filename("/etc/");
    if(f_options_environment.f_group_name != nullptr
    && *f_options_environment.f_group_name != '\0')
    {
        filename += f_options_environment.f_group_name;
    }
    else if(f_options_environment.f_project_name != nullptr
         && *f_options_environment.f_project_name != '\0')
    {
        filename += f_options_environment.f_project_name;
    }
    else
    {
        // really nothing can be done in this case... we have no name
        // to generate a valid path/configuration filename
        //
        return std::string();
    }

    filename += '/';

    if(f_options_environment.f_project_name != nullptr
    && *f_options_environment.f_project_name != '\0')
    {
        filename += f_options_environment.f_project_name;
    }
    else if(f_options_environment.f_group_name != nullptr
         && *f_options_environment.f_group_name != '\0')
    {
        filename += f_options_environment.f_group_name;
    }
    else
    {
        throw getopt_logic_error("we just checked both of those names and at least one was valid.");       // LCOV_EXCL_LINE
    }

    filename += ".conf";

    return filename;
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

    conf_file_setup::pointer_t conf_setup;
    if(f_options_environment.f_config_setup == nullptr)
    {
        conf_setup = std::make_shared<conf_file_setup>(filename);
    }
    else
    {
        conf_setup = std::make_shared<conf_file_setup>(
                          filename
                        , *f_options_environment.f_config_setup);
    }
    if(!conf_setup->is_valid())
    {
        // a non-existant file is considered valid now so this should never
        // happen; later we may use the flag if we find errors in the file
        //
        return; // LCOV_EXCL_LINE
    }
    conf_file::pointer_t conf(conf_file::get_conf_file(*conf_setup));

    // is there a variable section?
    //
    if(f_options_environment.f_section_variables_name != nullptr)
    {
        conf->section_to_variables(
                      f_options_environment.f_section_variables_name
                    , f_variables);
    }

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
                        | GETOPT_FLAG_CONFIGURATION_FILE);
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
                configuration_sections->add_value(
                          s
                        , string_list_t()
                        , option_source_t::SOURCE_CONFIGURATION);
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
                               << option_with_underscores(param.first)
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
                               << option_with_underscores(param.first)
                               << "\" is not supported in configuration files (found in \""
                               << filename
                               << "\")."
                               << cppthread::end;
                continue;
            }
        }

        std::string value(param.second.get_value());
        switch(param.second.get_assignment_operator())
        {
        case advgetopt::assignment_t::ASSIGNMENT_SET:
        case advgetopt::assignment_t::ASSIGNMENT_NONE:
            // nothing special in this case, just overwrite if already defined
            //
            break;

        case advgetopt::assignment_t::ASSIGNMENT_OPTIONAL:
            if(opt->is_defined())
            {
                // already set, do not overwrite
                //
                continue;
            }
            break;

        case advgetopt::assignment_t::ASSIGNMENT_APPEND:
            if(opt->is_defined()
            && !opt->has_flag(GETOPT_FLAG_MULTIPLE))
            {
                // append the new value
                //
                value = opt->get_value() + value;
            }
            break;

        case advgetopt::assignment_t::ASSIGNMENT_NEW:
            if(opt->is_defined())
            {
                // prevent re-assignment
                //
                cppthread::log << cppthread::log_level_t::error
                               << "option \""
                               << option_with_underscores(param.first)
                               << "\" found in configuration file \""
                               << filename
                               << "\" on line "
                               << param.second.get_line()
                               << " uses the := operator but the value is already defined."
                               << cppthread::end;
                continue;
            }
            break;

        }

        add_option_from_string(
                  opt
                , value
                , filename
                , string_list_t()
                , option_source_t::SOURCE_CONFIGURATION);
    }

    f_parsed = true;
}






} // namespace advgetopt
// vim: ts=4 sw=4 et
