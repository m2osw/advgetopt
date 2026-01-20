// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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
 *
 * This file is covered by the following tests:
 *
 * \li options_parser
 * \li invalid_options_parser
 * \li valid_options_files
 * \li invalid_options_files
 */

// self
//
#include    "advgetopt/advgetopt.h"

#include    "advgetopt/conf_file.h"
#include    "advgetopt/exception.h"


// cppthread
//
#include    <cppthread/log.h>


// snapdev
//
#include    <snapdev/glob_to_list.h>
#include    <snapdev/join_strings.h>
#include    <snapdev/tokenize_string.h>


// C++
//
#include    <list>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{







/** \brief Reset all the options.
 *
 * This function goes through the list of options and mark them all as
 * undefined. This is useful if you want to reuse a getopt object.
 *
 * The effect is that all calls to is_defined() made afterward return false
 * until new arguments get parsed.
 */
void getopt::reset()
{
    for(auto & opt : f_options_by_name)
    {
        opt.second->reset();
    }
}


/** \brief Parse the options to option_info objects.
 *
 * This function transforms an array of options in a vector of option_info
 * objects.
 *
 * \param[in] opts  An array of options to be parsed.
 * \param[in] ignore_duplicates  Whether to ignore potential duplicates.
 */
void getopt::parse_options_info(option const * opts, bool ignore_duplicates)
{
    if(opts == nullptr)
    {
        return;
    }

    for(
      ; (opts->f_flags & GETOPT_FLAG_END) == 0
      ; ++opts)
    {
        if(opts->f_name == nullptr
        || opts->f_name[0] == '\0')
        {
            throw getopt_logic_error("option long name missing or empty.");
        }
        short_name_t const one_char(string_to_short_name(opts->f_name));
        if(one_char != NO_SHORT_NAME)
        {
            throw getopt_logic_error("a long name option must be at least 2 characters.");
        }

        short_name_t const short_name(opts->f_short_name);

        char const * const n(strrchr(opts->f_name, ':'));
        if((opts->f_flags & GETOPT_FLAG_REMOVE_NAMESPACE) != 0
        && n != nullptr)
        {
            // TODO: this is probably not a well thought feature... when adding
            //       dynamic options within a library we want to include the
            //       name of that library to the option, something like:
            //
            //           fluid-settings::fluid-settings-timeout
            //
            //       and it is important to keep the namespace in this case,
            //       as we do above; however, for the command line, we don't
            //       want to have enter the namespace, so here we remove it
            //       for that option (and above we hid the one with the namespace)
            //

            // the official option uses `basename`
            //
            std::string const basename(n + 1);
            option_info::pointer_t o(std::make_shared<option_info>(
                                                  basename
                                                , short_name));
            o->set_variables(f_variables);

            o->set_environment_variable_name(opts->f_environment_variable_name);
            o->add_flag(opts->f_flags);
            o->set_default(opts->f_default);
            o->set_help(opts->f_help);
            o->set_multiple_separators(opts->f_multiple_separators);

            if(opts->f_validator != nullptr)
            {
                o->set_validator(opts->f_validator);
            }

            add_option(o, ignore_duplicates);

            // the full named option (with namespaces) uses an alias
            //
            option_info::pointer_t alias(std::make_shared<option_info>(
                                                  opts->f_name
                                                , short_name));
            alias->set_variables(f_variables);

            alias->set_environment_variable_name(opts->f_environment_variable_name);
            alias->add_flag(opts->f_flags | GETOPT_FLAG_ALIAS);
            alias->set_default(opts->f_default);
            alias->set_help(basename);
            alias->set_multiple_separators(opts->f_multiple_separators);

            if(opts->f_validator != nullptr)
            {
                alias->set_validator(opts->f_validator);
            }

            add_option(alias, ignore_duplicates);
        }
        else
        {
            option_info::pointer_t o(std::make_shared<option_info>(
                                                  opts->f_name
                                                , short_name));
            o->set_variables(f_variables);

            o->set_environment_variable_name(opts->f_environment_variable_name);
            o->add_flag(opts->f_flags);
            o->set_default(opts->f_default);
            o->set_help(opts->f_help);
            o->set_multiple_separators(opts->f_multiple_separators);

            if(opts->f_validator != nullptr)
            {
                o->set_validator(opts->f_validator);
            }

            add_option(o, ignore_duplicates);
        }
    }
}


/** \brief Add one option to the advgetopt object.
 *
 * This function is used to dynamically add one option to the advgetopt
 * object.
 *
 * This is often used in a library which wants to dynamically add support
 * for library specific parameters to the command line.
 *
 * \note
 * The \p ignore_duplicates option still gets the option added if only
 * the short-name is a duplicate. In that case, we set the option's
 * short-name to NO_SHORT_NAME before adding the option to the tables.
 *
 * \param[in] opt  The option to be added.
 * \param[in] ignore_duplicates  If option is a duplicate, do not add it.
 */
void getopt::add_option(option_info::pointer_t opt, bool ignore_duplicates)
{
    if(get_option(opt->get_name(), true) != nullptr)
    {
        if(ignore_duplicates)
        {
            return;
        }
        throw getopt_defined_twice(
                  std::string("option named \"")
                + opt->get_name()
                + "\" found twice.");
    }

    short_name_t short_name(opt->get_short_name());
    if(get_option(short_name, true) != nullptr)
    {
        if(ignore_duplicates)
        {
            short_name = NO_SHORT_NAME;
            opt->set_short_name(NO_SHORT_NAME);
        }
        else
        {
            throw getopt_defined_twice(
                      "option with short name \""
                    + short_name_to_string(short_name)
                    + "\" found twice.");
        }
    }

    if(opt->is_default_option())
    {
        if(f_default_option != nullptr)
        {
            throw getopt_logic_error("two default options found.");
        }
        if(opt->has_flag(GETOPT_FLAG_FLAG))
        {
            throw getopt_logic_error("a default option must accept parameters, it can't be a GETOPT_FLAG_FLAG.");
        }

        f_default_option = opt;
    }

    f_options_by_name[opt->get_name()] = opt;

    if(short_name != NO_SHORT_NAME)
    {
        f_options_by_short_name[short_name] = opt;
    }
}


std::string getopt::get_path_to_option_files() const
{
    std::string path;

    char const * const options_files_directory(getenv("ADVGETOPT_OPTIONS_FILES_DIRECTORY"));
    if(options_files_directory != nullptr
    && *options_files_directory != '\0')
    {
        // environment variable has priority
        //
        path = options_files_directory;
    }
    else if(f_options_environment.f_options_files_directory != nullptr
         && f_options_environment.f_options_files_directory[0] != '\0')
    {
        // next the tool option environment has priority
        //
        path = f_options_environment.f_options_files_directory;
    }
    else
    {
        // finally, use a default
        //
        path = "/usr/share/advgetopt/options/";
    }

    if(path.back() != '/')
    {
        path += '/';
    }

    return path;
}


/** \brief Get the path and filename to options.
 *
 * The programmer can define a path to options that the tool loads on
 * initialization of the advgetopt object. By default, the path is
 * expected to be `/usr/share/advgetopt/options` and the filename is
 * set to the name of the group (if defined) or the name of the project
 * (if defined).
 *
 * Note that if neither the group nor the project names are defined, then
 * the function returns an empty list. If at least one of the names is
 * defined, then at least one filename is added: the main filename. This
 * does not mean that file exists. For the additional files, however, they
 * are found using a pattern so they for sure exist.
 *
 * In order to allow debugging as a programmer, we also support changing
 * the source directory through an environment variable named
 * `ADVGETOPT_OPTIONS_FILES_DIRECTORY`. This variable is checked
 * first and any other path is ignored if it is defined and not just an
 * empty string.
 *
 * The following is the order in which things are being checked:
 *
 * \code
 *     # if the options path variable & group name are defined
 *     <variable-options-path>/<group-name>.ini
 *     <variable-options-path>/<group-name>-*.ini
 *
 *     # if the options path variable & project name are defined
 *     <variable-options-path>/<project-name>.ini
 *     <variable-options-path>/<project-name>-*.ini
 *
 *     # if the options path & group name are defined
 *     <options-path>/<group-name>.ini
 *     <options-path>/<group-name>-*.ini
 *
 *     # if the options path & project name are defined
 *     <options-path>/<project-name>.ini
 *     <options-path>/<project-name>-*.ini
 *
 *     # if the group name is defined:
 *     /usr/share/advgetopt/options/<group-name>.ini
 *     /usr/share/advgetopt/options/<group-name>-*.ini
 *
 *     # if the project name is defined:
 *     /usr/share/advgetopt/options/<project-name>.ini
 *     /usr/share/advgetopt/options/<project-name>-*.ini
 * \endcode
 *
 * \return A list of filenames to load option definitions from.
 */
string_list_t getopt::get_filenames_of_option_definitions() const
{
    string_list_t result;

    std::string const filename(get_group_or_project_name());
    if(filename.empty())
    {
        return result;
    }

    std::string const path(get_path_to_option_files());

    // add the plain filename
    //
    result.push_back(path + filename + ".ini");

    // look for additional filenames
    //
    std::string pattern(path + filename + "-*.ini");
    snapdev::glob_to_list<std::list<std::string>> list;
    if(list.read_path<snapdev::glob_to_list_flag_t::GLOB_FLAG_IGNORE_ERRORS>(pattern))
    {
        for(auto const & l : list)
        {
            result.push_back(l);
        }
    }

    return result;
}


/** \brief Check for a file with option definitions.
 *
 * This function tries to read the default option file for this process.
 * This filename is generated using the option environment files
 * directory and the group or project name.
 *
 * First, we test with the name "<group-name>.ini" then again with a
 * pattern: "<group-name>-*.ini". The order in which the files are defined
 * is not important so there is no number required. If the group name is
 * not defined, then the project name is used (i.e. "<project-name>-*.ini").
 *
 * If the directory is not defined, the function uses this default path:
 * `"/usr/share/advgetopt/options/"`. See the other
 * parse_options_from_file(std::string const & filename, int min_sections,
 * int max_sections, bool ignore_duplicates)
 * function for additional details.
 *
 * \note
 * If you support plugins and thus want to possibly accept many extensions
 * to your list of options, you may want to consider defining your own
 * directory (the `options_environment.f_options_files_directory` parameter).
 *
 * \sa parse_options_from_file(std::string const & filename, int min_sections, int max_sections, bool ignore_duplicates)
 */
void getopt::parse_options_from_file()
{
    string_list_t const list(get_filenames_of_option_definitions());
    for(auto const & l : list)
    {
        parse_options_from_file(l, 1, 1);
    }
}


/** \brief Check for a file with option definitions.
 *
 * This function tries to read the specified file for command line options
 * for this application. These are similar to the option structure, only it
 * is defined in a file.
 *
 * The format of the file is like so:
 *
 * \li Option names are defined on a line by themselves between square brackets.
 * \li Parameters of that option are defined below as a `name=<value>`.
 *
 * Example:
 *
 * \code
 *     [<command-name>]
 *     short_name=<character>
 *     default=<default value>
 *     help=<help sentence>
 *     validator=<validator name>[(<param>[,<param>...])]|/<regex>/<flags>
 *     alias=<name of aliased option>
 *     allowed=command-line,environment-variable,configuration-file,dynamic-configuration
 *     show-usage-on-error
 *     no-arguments|multiple
 *     required
 * \endcode
 *
 * The number of namespaces in `<command-name>` can be limited using the
 * \p min_sections and \p max_sections parameters.
 *
 * The function can be called multiple times. The first time, it verifies
 * that there are not duplicated settings. On following loads, that test
 * is ignored.
 *
 * \todo
 * Test that options get 100% updated on a reload.
 *
 * \note
 * By default, this function is called with one specific filename based
 * on the f_project_name field and the f_options_files_directory as
 * defined in the options environment.
 *
 * \param[in] filename  The filename to load.
 * \param[in] min_sections  The minimum number of namespaces.
 * \param[in] max_sections  The maximum number of namespaces.
 * \param[in] ignore_duplicates  Whether duplicates are okay or not.
 * \param[in] keep_all_sections  Whether remove this project's namespace.
 *
 * \sa parse_options_from_file()
 */
void getopt::parse_options_from_file(
          std::string const & filename
        , int const min_sections
        , int const max_sections
        , bool ignore_duplicates
        , bool keep_all_sections)
{
    if(filename.empty())
    {
        return;
    }

    section_operator_t operators(SECTION_OPERATOR_INI_FILE);
    if(min_sections == 1
    && max_sections == 1)
    {
        operators |= SECTION_OPERATOR_ONE_SECTION;
    }

    conf_file_setup conf_setup(
              filename
            , line_continuation_t::line_continuation_unix
            , ASSIGNMENT_OPERATOR_EQUAL
            , COMMENT_INI | COMMENT_SHELL
            , operators);
    if(!conf_setup.is_valid())
    {
        return;  // LCOV_EXCL_LINE
    }

    // if the file includes a section named after the group or project
    // we can remove it completely (this helps with sharing fluid settings)
    //
    // the format of an option file is:
    //
    // [<option-name>]
    // help=option description
    //
    // For fluid-settings to work, we need to include the name of service
    // or tool as in:
    //
    // [<service>::<option-name>]
    // help=option description
    //
    // so we want to remove the "<service>::" part to avoid the namespace
    // in the --<option-name> command line options.
    //
    std::string const section_to_ignore(get_group_or_project_name());
    conf_setup.set_section_to_ignore(section_to_ignore);

    conf_file::pointer_t conf(conf_file::get_conf_file(conf_setup));
    conf_file::sections_t const & sections(conf->get_sections());
    for(auto & section_names : sections)
    {
        string_list_t names;
        split_string(section_names, names, {"::"});
        std::string option_name;
        if(keep_all_sections
        && names.size() > 1
        && *names.begin() == section_to_ignore)
        {
            names.erase(names.begin());
            option_name = snapdev::join_strings(names, "::");
        }
        else
        {
            option_name = section_names;
        }

        if(names.size() < static_cast<std::size_t>(min_sections)
        || names.size() > static_cast<std::size_t>(max_sections))
        {
            if(min_sections == 1
            && max_sections == 1)  // LCOV_EXCL_LINE
            {
                // right now this case cannot happen because we set the
                // SECTION_OPERATOR_ONE_SECTION flag so errors are caught
                // directly inside the conf_file::get_conf_file() call
                //
                cppthread::log << cppthread::log_level_t::error                             // LCOV_EXCL_LINE
                    << filename                                                             // LCOV_EXCL_LINE
                    << ": the name of a settings definition must include one namespace; \"" // LCOV_EXCL_LINE
                    << section_names                                                        // LCOV_EXCL_LINE
                    << "\" is not considered valid."                                        // LCOV_EXCL_LINE
                    << cppthread::end;                                                      // LCOV_EXCL_LINE
            }
            else
            {
                cppthread::log << cppthread::log_level_t::error
                    << filename
                    << ": the name of a settings definition must include between "
                    << min_sections
                    << " and "
                    << max_sections
                    << " namespaces; \""
                    << section_names
                    << "\" is not considered valid."
                    << cppthread::end;
            }
            continue;
        }

        std::string const parameter_name(option_name);
        std::string const short_name(unquote(conf->get_parameter(parameter_name + "::shortname")));
        short_name_t const sn(string_to_short_name(short_name));
        if(sn == NO_SHORT_NAME
        && !short_name.empty())
        {
            throw getopt_logic_error(
                      "option \""
                    + section_names
                    + "\" has an invalid short name \""
                    + short_name
                    + "\" in \""
                    + filename
                    + "\", it can't be more than one character.");
        }

        option_info::pointer_t opt(std::make_shared<option_info>(parameter_name, sn));
        opt->set_variables(f_variables);

        std::string const environment_variable_name(parameter_name + "::environment_variable_name");
        if(conf->has_parameter(environment_variable_name))
        {
            opt->set_environment_variable_name(unquote(conf->get_parameter(environment_variable_name)));
        }

        std::string const default_name(parameter_name + "::default");
        if(conf->has_parameter(default_name))
        {
            opt->set_default(unquote(conf->get_parameter(default_name)));
        }

        opt->set_help(unquote(conf->get_parameter(parameter_name + "::help")));

        std::string const validator_name_and_params(conf->get_parameter(parameter_name + "::validator"));
        opt->set_validator(validator_name_and_params);

        std::string const alias_name(parameter_name + "::alias");
        if(conf->has_parameter(alias_name))
        {
            if(!opt->get_help().empty())
            {
                throw getopt_logic_error(
                          "option \""
                        + section_names
                        + "\" is an alias and as such it can't include a help=... parameter in \""
                        + filename
                        + "\".");
            }
            opt->set_help(unquote(conf->get_parameter(alias_name)));
            opt->add_flag(GETOPT_FLAG_ALIAS);
        }

        std::string const allowed_name(parameter_name + "::allowed");
        if(conf->has_parameter(allowed_name))
        {
            std::string const allowed_list(conf->get_parameter(allowed_name));
            string_list_t allowed;
            split_string(allowed_list, allowed, {","});
            for(auto const & a : allowed)
            {
                if(a == "command-line")
                {
                    opt->add_flag(GETOPT_FLAG_COMMAND_LINE);
                }
                else if(a == "environment-variable")
                {
                    opt->add_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE);
                }
                else if(a == "configuration-file")
                {
                    opt->add_flag(GETOPT_FLAG_CONFIGURATION_FILE);
                }
                else if(a == "dynamic-configuration")
                {
                    opt->add_flag(GETOPT_FLAG_DYNAMIC_CONFIGURATION);
                }
            }
        }

        std::string const group_name(parameter_name + "::group");
        if(conf->has_parameter(group_name))
        {
            std::string const group(conf->get_parameter(group_name));
            if(group == "commands")
            {
                opt->add_flag(GETOPT_FLAG_GROUP_COMMANDS);
            }
            else if(group == "options")
            {
                opt->add_flag(GETOPT_FLAG_GROUP_OPTIONS);
            }
            else if(group == "three")
            {
                opt->add_flag(GETOPT_FLAG_GROUP_THREE);
            }
            else if(group == "four")
            {
                opt->add_flag(GETOPT_FLAG_GROUP_FOUR);
            }
            else if(group == "five")
            {
                opt->add_flag(GETOPT_FLAG_GROUP_FIVE);
            }
            else if(group == "six")
            {
                opt->add_flag(GETOPT_FLAG_GROUP_SIX);
            }
            else if(group == "seven")
            {
                opt->add_flag(GETOPT_FLAG_GROUP_SEVEN);
            }
        }

        if(conf->has_parameter(parameter_name + "::show-usage-on-error"))
        {
            opt->add_flag(GETOPT_FLAG_SHOW_USAGE_ON_ERROR);
        }

        if(conf->has_parameter(parameter_name + "::no-arguments"))
        {
            opt->add_flag(GETOPT_FLAG_FLAG);
        }

        if(conf->has_parameter(parameter_name + "::multiple"))
        {
            opt->add_flag(GETOPT_FLAG_MULTIPLE);
        }

        if(conf->has_parameter(parameter_name + "::required"))
        {
            opt->add_flag(GETOPT_FLAG_REQUIRED);
        }

        add_option(opt, ignore_duplicates);
    }
}


/** \brief Link options marked as a GETOPT_FLAG_ALIAS.
 *
 * After we defined all the options, go through the list again to find
 * aliases and link them with their corresponding alias option.
 *
 * \exception getopt_exception_invalid
 * All aliases must exist or this exception is raised.
 */
void getopt::link_aliases()
{
    for(auto & c : f_options_by_name)
    {
        if(c.second->has_flag(GETOPT_FLAG_ALIAS))
        {
            std::string const & alias_name(c.second->get_help());
            if(alias_name.empty())
            {
                throw getopt_logic_error(
                          "the default value of your alias cannot be an empty string for \""
                        + c.first
                        + "\".");
            }

            // we have to use the `true` flag in this get_option() because
            // aliases may not yet be defined
            //
            option_info::pointer_t alias(get_option(alias_name, true));
            if(alias == nullptr)
            {
                throw getopt_logic_error(
                          "no option named \""
                        + alias_name
                        + "\" to satisfy the alias of \""
                        + c.first
                        + "\".");
            }

            flag_t const expected_flags(c.second->get_flags() & ~GETOPT_FLAG_ALIAS);
            if(alias->get_flags() != expected_flags)
            {
                std::stringstream ss;
                ss << std::hex
                   << "the flags of alias \""
                   << c.first
                   << "\" (0x"
                   << expected_flags
                   << ") are different than the flags of \""
                   << alias_name
                   << "\" (0x"
                   << alias->get_flags()
                   << ").";
                throw getopt_logic_error(ss.str());
            }

            c.second->set_alias_destination(alias);
        }
    }
}


/** \brief Assign a short name to an option.
 *
 * This function allows for dynamically assigning a short name to an option.
 * This is useful for cases where a certain number of options may be added
 * dynamically and may share the same short name or similar situation.
 *
 * On our end we like to add `-c` as the short name of the `--config-dir`
 * command line or environment variable option. However, some of our tools
 * use `-c` for other reason (i.e. our `cxpath` tool uses `-c` for its
 * `--compile` option.) So we do not want to have it as a default in
 * `--config-dir`. Instead we assign it afterward if possible.
 *
 * **IMPORTANT:** It is possible to change the short-name at any time.
 * However, note that you can't have duplicates. It is also possible
 * to remove a short-name by setting it to the advgetopt::NO_SHORT_NAME
 * special value.
 *
 * \note
 * This function requires you to make use of the constructor without the
 * `argc` and `argv` parameters, add the short name, then run all the
 * parsing.
 *
 * \exception getopt_exception_logic
 * The same short name cannot be used more than once. This exception is
 * raised if it is discovered that another option already makes use of
 * this short name. This exception is also raised if \p name does not
 * reference an existing option.
 *
 * \param[in] name  The name of the option which is to receive a short name.
 * \param[in] short_name  The short name to assigned to the \p name option.
 */
void getopt::set_short_name(std::string const & name, short_name_t short_name)
{
    auto opt(f_options_by_name.find(name));
    if(opt == f_options_by_name.end())
    {
        throw getopt_logic_error(
                  "option with name \""
                + name
                + "\" not found.");
    }

    if(short_name != NO_SHORT_NAME)
    {
        auto it(f_options_by_short_name.find(short_name));
        if(it != f_options_by_short_name.end())
        {
            if(it->second == opt->second)
            {
                // same option, already named 'short_name'
                //
                return;
            }

            throw getopt_logic_error(
                      "found another option (\""
                    + it->second->get_name()
                    + "\") with short name '"
                    + short_name_to_string(short_name)
                    + "'.");
        }
    }

    short_name_t const old_short_name(opt->second->get_short_name());
    if(old_short_name != NO_SHORT_NAME)
    {
        auto it(f_options_by_short_name.find(old_short_name));
        if(it != f_options_by_short_name.end())
        {
            f_options_by_short_name.erase(it);
        }
    }

    opt->second->set_short_name(short_name);

    if(short_name != NO_SHORT_NAME)
    {
        f_options_by_short_name[short_name] = opt->second;
    }
}


/** \brief Output the source of each option.
 *
 * This function goes through the list of options by name ("alphabetically")
 * and prints out the sources or "(undefined)" if not defined anywhere.
 *
 * This function gets called when using the `--show-option-sources`
 * system command line option at the time the process_system_options()
 * function gets called.
 *
 * \param[in] out  The output streaming where the info is written.
 */
void getopt::show_option_sources(std::basic_ostream<char> & out)
{
    int idx(1);
    out << "Option Sources:\n";
    for(auto const & opt : f_options_by_name)
    {
        out << "  " << idx << ". option \"" << opt.second->get_name() << "\"";
        string_list_t sources(opt.second->trace_sources());
        if(sources.empty())
        {
            out << " (undefined)\n";
        }
        else
        {
            out << "\n";
            for(auto const & src : sources)
            {
                out << "     " << src << "\n";
            }
        }
        out << "\n";

        ++idx;
    }
    out << std::flush;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
