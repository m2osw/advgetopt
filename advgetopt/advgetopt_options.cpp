/*
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


// advgetopt lib
//
#include    "advgetopt/conf_file.h"
#include    "advgetopt/exception.h"
#include    "advgetopt/log.h"


// last include
//
#include <snapdev/poison.h>




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
            throw getopt_exception_logic("option long name missing or empty.");
        }
        short_name_t const one_char(string_to_short_name(opts->f_name));
        if(one_char != NO_SHORT_NAME)
        {
            throw getopt_exception_logic("a long name option must be at least 2 characters.");
        }

        if(get_option(opts->f_name, true) != nullptr)
        {
            if(ignore_duplicates)
            {
                continue;
            }
            throw getopt_exception_logic(
                      std::string("option named \"")
                    + opts->f_name
                    + "\" found twice.");
        }
        short_name_t short_name(opts->f_short_name);
        if(get_option(short_name, true) != nullptr)
        {
            if(ignore_duplicates)
            {
                short_name = U'\0';
            }
            else
            {
                throw getopt_exception_logic(
                          "option with short name \""
                        + short_name_to_string(opts->f_short_name)
                        + "\" found twice.");
            }
        }

        option_info::pointer_t o(std::make_shared<option_info>(
                                              opts->f_name
                                            , short_name));

        o->add_flag(opts->f_flags);
        o->set_default(opts->f_default);
        o->set_help(opts->f_help);
        o->set_multiple_separators(opts->f_multiple_separators);

        if(opts->f_validator != nullptr)
        {
            o->set_validator(opts->f_validator);
        }

        if(o->is_default_option())
        {
            if(f_default_option != nullptr)
            {
                throw getopt_exception_logic("two default options found after check of long names duplication.");
            }
            if(o->has_flag(GETOPT_FLAG_FLAG))
            {
                throw getopt_exception_logic("a default option must accept parameters, it can't be a GETOPT_FLAG_FLAG.");
            }

            f_default_option = o;
        }

        f_options_by_name[o->get_name()] = o;
        if(short_name != NO_SHORT_NAME)
        {
            f_options_by_short_name[short_name] = o;
        }
    }
}


/** \brief Check for a file with option definitions.
 *
 * This function tries to read a file of options for this application.
 * These are similar to the option structure, only it is defined in a
 * file.
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
 *     validator=<validator name>[(<param>)]|/<regex>/<flags>
 *     alias=<name of aliased option>
 *     allowed=command-line,environment-variable,configuration-file
 *     show-usage-on-error
 *     no-arguments|multiple
 *     required
 * \endcode
 */
void getopt::parse_options_from_file()
{
    std::string filename;

    if(f_options_environment.f_project_name == nullptr
    || f_options_environment.f_project_name[0] == '\0')
    {
        return;
    }

    if(f_options_environment.f_options_files_directory == nullptr
    || f_options_environment.f_options_files_directory[0] == '\0')
    {
        filename = "/usr/share/advgetopt/options";
    }
    else
    {
        filename = f_options_environment.f_options_files_directory;
        if(filename.back() != '/')
        {
            filename += '/';
        }
    }
    filename += f_options_environment.f_project_name;
    filename += ".ini";

    conf_file_setup conf_setup(filename
                             , line_continuation_t::line_continuation_unix
                             , ASSIGNMENT_OPERATOR_EQUAL
                             , COMMENT_INI | COMMENT_SHELL
                             , SECTION_OPERATOR_INI_FILE | SECTION_OPERATOR_ONE_SECTION);
    if(!conf_setup.is_valid())
    {
        return;
    }

    conf_file::pointer_t conf(conf_file::get_conf_file(conf_setup));
    conf_file::sections_t const & sections(conf->get_sections());
    for(auto & section_name : sections)
    {
        std::string::size_type pos(section_name.find("::"));
        if(pos != std::string::npos)
        {
            // this should never happen since we use the
            // SECTION_OPERATOR_ONE_SECTION flag
            //
            throw getopt_exception_logic(                               // LCOV_EXCL_LINE
                      "section \""                                      // LCOV_EXCL_LINE
                    + section_name                                      // LCOV_EXCL_LINE
                    + "\" includes a section separator (::) in \""      // LCOV_EXCL_LINE
                    + filename                                          // LCOV_EXCL_LINE
                    + "\". We only support one level.");                // LCOV_EXCL_LINE
        }

        std::string const parameter_name(section_name);
        std::string const short_name(unquote(conf->get_parameter(parameter_name + "::shortname")));
        if(short_name.length() > 1)
        {
            throw getopt_exception_logic(
                      "option \""
                    + section_name
                    + "\" has an invalid short name in \""
                    + filename
                    + "\", it can't be more than one character.");
        }
        short_name_t sn('\0');
        if(short_name.length() == 1)
        {
            sn = short_name[0];
        }

        option_info::pointer_t opt(std::make_shared<option_info>(parameter_name, sn));

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
                throw getopt_exception_logic(
                          "option \""
                        + section_name
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

        f_options_by_name[parameter_name] = opt;
        if(sn != NO_SHORT_NAME)
        {
            f_options_by_short_name[sn] = opt;
        }
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
                throw getopt_exception_logic(
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
                throw getopt_exception_logic(
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
                throw getopt_exception_logic(ss.str());
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
 * `--compile` option.) So we do not want to have it has a default in
 * that option. Instead we assign it afterward.
 *
 * **IMPORTANT:** To make this call useful, make sure to make it before
 * you call the parse functions. Setting the short name after the parsing
 * was done is going to be useless.
 *
 * \note
 * This function requires you to make use of the constructor without the
 * `argc` and `argv` parameters, add the short name, then run all the
 * parsing.
 *
 * \note
 * The function calls the option_info::set_short_name() function which
 * may raise an exception if the option already has a short name (or if
 * you inadvertendly passed NO_SHORT_NAME.)
 *
 * \exception getopt_exception_logic
 * The same short name cannot be used more than once. This exception is
 * raised if it is discovered that another option already makes use of
 * this short name. This exception is also raised if the \p name
 * parameter does not reference an existing option.
 *
 * \param[in] name  The name of the option which is to receive a short name.
 * \param[in] short_name  The short name to assigned to the \p name option.
 */
void getopt::set_short_name(std::string const & name, short_name_t short_name)
{
    auto it(f_options_by_short_name.find(short_name));
    if(it != f_options_by_short_name.end())
    {
        throw getopt_exception_logic(
                  "found another option (\""
                + it->second->get_name()
                + "\") with short name '"
                + short_name_to_string(short_name)
                + "'.");
    }

    auto opt(f_options_by_name.find(name));
    if(opt == f_options_by_name.end())
    {
        throw getopt_exception_logic(
                  "option with name \""
                + name
                + "\" not found.");
    }

    opt->second->set_short_name(short_name);

    f_options_by_short_name[short_name] = opt->second;
}




} // namespace advgetopt
// vim: ts=4 sw=4 et
