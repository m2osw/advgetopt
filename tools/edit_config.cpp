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
 * \brief Tool used to edit configuration files from the command line.
 *
 * Retrieve a parameter from a configuration file, allow for the
 * editing of a snap configuration file parameter, all from the
 * command line.
 */

// advgetopt
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/conf_file.h>
#include    <advgetopt/exception.h>
#include    <advgetopt/version.h>


// snapdev
//
#include    <snapdev/join_strings.h>


// libexcept
//
#include    <libexcept/file_inheritance.h>


// boost
//
#include    <boost/preprocessor/stringize.hpp>


// C++
//
#include    <iostream>


// C
//
#include    <unistd.h>


// last include
//
#include    <snapdev/poison.h>



advgetopt::option const g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("colon")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Accept a colon as the assignment operator.")
    ),
    advgetopt::define_option(
          advgetopt::Name("create-backup")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::DefaultValue(".bak")
        , advgetopt::Help("Create a backup before updating the configuration file. If the file exists, keep that old backup instead.")
    ),
    advgetopt::define_option(
          advgetopt::Name("dashes")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Output parameter names with dashes.")
    ),
    advgetopt::define_option(
          advgetopt::Name("equal")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Accept an equal sign as the assignment operator (this is the default is not assignment is specified).")
    ),
    advgetopt::define_option(
          advgetopt::Name("must-exist")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("If the value does not exist, an error is printed and the process exits with 1.")
    ),
    advgetopt::define_option(
          advgetopt::Name("no-warning")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("By default, if the --remove-comments option is used, the tool writes a warning at the beginning of the file. Use this flag to avoid that warning.")
    ),
    advgetopt::define_option(
          advgetopt::Name("priority")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Priority when saving file under a sub-directory.")
    ),
    advgetopt::define_option(
          advgetopt::Name("remove-comments")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("By default, the tool tries to keep the comments intact. Use this flag to remove comments.")
    ),
    advgetopt::define_option(
          advgetopt::Name("replace-backup")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::DefaultValue(".bak")
        , advgetopt::Help("Create a backup before updating the configuration file. If the file exists, replace it.")
    ),
    advgetopt::define_option(
          advgetopt::Name("space")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Accept just a space as the assignment operator.")
    ),
    advgetopt::define_option(
          advgetopt::Name("sub-directory")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("If defined, try reading the file from that sub-directory. If not found there, try in the parent (as defined on the command line). Always save in that sub-directory if editing.")
        , advgetopt::EnvironmentVariableName("SUBDIRECTORY")
    ),
    advgetopt::define_option(
          advgetopt::Name("underscores")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Output parameter names with underscores (default).")
    ),
    advgetopt::define_option(
          advgetopt::Name("--")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_MULTIPLE
                    , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
        , advgetopt::Help("Configuration filename, field name, optionally, a new value.")
    ),
    advgetopt::end_options()
};




advgetopt::group_description const g_group_descriptions[] =
{
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
        , advgetopt::GroupName("command")
        , advgetopt::GroupDescription("Commands:")
    ),
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
        , advgetopt::GroupName("option")
        , advgetopt::GroupDescription("Options:")
    ),
    advgetopt::end_groups()
};


// until we have C++20 remove warnings this way
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "advgetopt",
    .f_group_name = nullptr,
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = nullptr,
    .f_environment_variable_intro = "EDIT_CONFIG_",
    .f_section_variables_name = nullptr,
    .f_configuration_files = nullptr,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>] <configuration filename> <field name> [<new value>]\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = LIBADVGETOPT_VERSION_STRING,
    .f_license = "GNU GPL v2",
    .f_copyright = "Copyright (c) 2013-"
                   BOOST_PP_STRINGIZE(UTC_BUILD_YEAR)
                   " by Made to Order Software Corporation -- All Rights Reserved",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions,
};
#pragma GCC diagnostic pop





class edit_config
{
public:
                        edit_config(int argc, char * argv[]);

    void                run();

private:
    advgetopt::getopt   f_opt;
};


/** \brief Initialize the DNS options object.
 *
 * This constructor parses the command line options and returns. It
 * does not try to interpret the command line  at all, this is reserved
 * to the run() function which has the ability to return an exit code.
 *
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  The array of arguments found on the command line.
 */
edit_config::edit_config(int argc, char * argv[])
    : f_opt(g_options_environment, argc, argv)
{
    if(f_opt.is_defined("create-backup")
    && f_opt.is_defined("replace-backup"))
    {
        std::cerr
            << f_opt.get_program_name()
            << ":error: the --create-backup and --replace-backup command line options are mutually exclusive."
            << std::endl;
        exit(1);
    }

    if(!f_opt.is_defined("--"))
    {
        std::cerr
            << f_opt.get_program_name()
            << ":error: no configuration name, field name, and value defined."
            << std::endl;
        exit(1);
    }

    int const sz(f_opt.size("--"));

    if(sz < 2)
    {
        std::cerr << f_opt.get_program_name() << ":error: to the minimum a configuration name and a field name are required." << std::endl;
        exit(1);
    }

    if(sz > 3)
    {
        std::cerr << f_opt.get_program_name() << ":error: to the maximum a configuration name, a field name, and a value can be defined." << std::endl;
        exit(1);
    }
}


/** \brief Run the command.
 *
 * This checks whether we have 2 or 3 parameters, if 2 we read the field
 * and if 3 we write to the field. Note that the writing will happen in
 * the `snapwebsites.d` sub-directory.
 */
void edit_config::run()
{
    std::string config_name(f_opt.get_string("--", 0));

    std::string sub_directory_name;
    if(f_opt.is_defined("sub-directory"))
    {
        advgetopt::string_list_t segments;
        advgetopt::split_string(config_name, segments, { "/" });
        std::string filename(segments.back());
        segments.pop_back();
        segments.push_back(f_opt.get_string("sub-directory"));
        if(f_opt.is_defined("priority"))
        {
            segments.push_back(f_opt.get_string("priority") + '-' + filename);
        }
        else
        {
            segments.push_back(filename);
        }

        sub_directory_name = snapdev::join_strings(segments, "/");
        if(access(sub_directory_name.c_str(), F_OK) == 0)
        {
            config_name = sub_directory_name;
        }
    }

    advgetopt::assignment_operator_t assignment_operator(0);

    if(f_opt.is_defined("colon"))
    {
        assignment_operator |= advgetopt::ASSIGNMENT_OPERATOR_COLON;
    }
    if(f_opt.is_defined("equal"))
    {
        assignment_operator |= advgetopt::ASSIGNMENT_OPERATOR_EQUAL;
    }
    if(f_opt.is_defined("space"))
    {
        assignment_operator |= advgetopt::ASSIGNMENT_OPERATOR_SPACE;
    }

    advgetopt::name_separator_t name_separator(advgetopt::NAME_SEPARATOR_UNDERSCORES);
    if(f_opt.is_defined("dashes"))
    {
        if(f_opt.is_defined("underscore"))
        {
            std::cerr << "error: --dashes & --underscores are mutually exclusive.\n";
            exit(1);
        }
        name_separator = advgetopt::NAME_SEPARATOR_DASHES;
    }

    advgetopt::conf_file_setup setup(
              config_name
            , advgetopt::line_continuation_t::line_continuation_unix
            , assignment_operator
            , advgetopt::COMMENT_INI
                | advgetopt::COMMENT_SHELL
                | (f_opt.is_defined("remove-comments")
                        ? 0
                        : advgetopt::COMMENT_SAVE)
            , advgetopt::SECTION_OPERATOR_INI_FILE
            , name_separator);
    advgetopt::conf_file::pointer_t config(advgetopt::conf_file::get_conf_file(setup));

    std::string const field_name(f_opt.get_string("--", 1));

    if(f_opt.is_defined("must-exist")
    && !config->has_parameter(field_name))
    {
        std::cerr << "error: field \""
            << field_name
            << "\" not found in \""
            << config_name
            << "\".\n";
        exit(1);
    }

    if(f_opt.size("--") == 2)
    {
        // retrieval, get the value and print to std::cout
        //
        if(config->has_parameter(field_name))
        {
            std::cout << config->get_parameter(field_name) << std::endl;
        }
        else
        {
            std::cout << std::endl;
        }
    }
    else
    {
        std::string const new_value(f_opt.get_string("--", 2));

        std::string::size_type colon(field_name.rfind(':'));
        if(colon != std::string::npos)
        {
            std::string const name_only(field_name.substr(colon + 1));
            while(colon > 0
               && field_name[colon - 1] == ':')
            {
                --colon;
            }
            std::string const sections(field_name.substr(0, colon));
            config->set_parameter(sections, name_only, new_value);
        }
        else
        {
            config->set_parameter(std::string(), field_name, new_value);
        }

        bool replace_backup(false);
        std::string backup_extension;
        if(f_opt.is_defined("create-backup"))
        {
            backup_extension = f_opt.get_string("create-backup");
        }
        else if(f_opt.is_defined("replace-backup"))
        {
            replace_backup = true;
            backup_extension = f_opt.get_string("replace-backup");
        }
        config->save_configuration(
                  backup_extension
                , replace_backup
                , !f_opt.is_defined("no-warning")
                , sub_directory_name);
    }
}





int main(int argc, char * argv[])
{
    libexcept::verify_inherited_files();

    try
    {
        edit_config s(argc, argv);
        s.run();
        return 0;
    }
    catch( advgetopt::getopt_exit const & except )
    {
        return except.code();
    }
    catch(std::exception const & e)
    {
        std::cerr << "edit-config: exception: " << e.what() << std::endl;
        return 1;
    }
}

// vim: ts=4 sw=4 et
