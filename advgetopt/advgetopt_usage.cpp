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
 * \brief Advanced getopt usage() implementation.
 *
 * The advgetopt class usage() and helper functions are grouped in this
 * file.
 */

// self
//
#include    "advgetopt/advgetopt.h"

#include    "advgetopt/exception.h"


// snapdev
//
#include    <snapdev/join_strings.h>
#include    <snapdev/tokenize_format.h>


// C++
//
#include    <iomanip>
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{



/** \brief Transform group names in --\<name>-help commands.
 *
 * This function allows for the group names to be transformed into help
 * command line options.
 */
void getopt::parse_options_from_group_names()
{
    // add the --long-help if at least one option uses the GROUP1, GROUP2,
    // or SYSTEM
    //
    // add the --system-help if at least one option uses SYSTEM
    //
    bool add_long_help(false);
    bool add_system_help(false);

    for(auto it(f_options_by_name.begin())
      ; it != f_options_by_name.end()
      ; ++it)
    {
        if(it->second->has_flag(GETOPT_FLAG_SHOW_GROUP1 | GETOPT_FLAG_SHOW_GROUP2 | GETOPT_FLAG_SHOW_SYSTEM))
        {
            add_long_help = true;
        }

        if(it->second->has_flag(GETOPT_FLAG_SHOW_SYSTEM))
        {
            add_system_help = true;
        }
    }

    if(add_long_help)
    {
        option_info::pointer_t opt(std::make_shared<option_info>("long-help"));
        opt->add_flag(GETOPT_FLAG_COMMAND_LINE
                    | GETOPT_FLAG_FLAG
                    | GETOPT_FLAG_GROUP_COMMANDS);
        opt->set_help("show all the help from all the available options.");
        f_options_by_name["long-help"] = opt;
        if(f_options_by_short_name.find(L'?') == f_options_by_short_name.end())
        {
            opt->set_short_name(L'?');
            f_options_by_short_name[L'?'] = opt;
        }
    }

    if(add_system_help)
    {
        option_info::pointer_t opt(std::make_shared<option_info>("system-help"));
        opt->add_flag(GETOPT_FLAG_COMMAND_LINE
                    | GETOPT_FLAG_FLAG
                    | GETOPT_FLAG_GROUP_COMMANDS);
        opt->set_help("show commands and options added by libraries.");
        f_options_by_name["system-help"] = opt;
        if(f_options_by_short_name.find(L'S') == f_options_by_short_name.end())
        {
            opt->set_short_name(L'S');
            f_options_by_short_name[L'S'] = opt;
        }
    }

    if(f_options_environment.f_groups == nullptr)
    {
        // no groups, ignore following loop
        //
        return;
    }

    for(group_description const * grp = f_options_environment.f_groups
      ; grp->f_group != GETOPT_FLAG_GROUP_NONE
      ; ++grp)
    {
        // the name is not mandatory, without it you do not get the command
        // line option but still get the group description
        //
        if(grp->f_name != nullptr
        && *grp->f_name != '\0')
        {
            std::string const name(grp->f_name);
            std::string const option_name(name + "-help");
            option_info::pointer_t opt(std::make_shared<option_info>(option_name));
            opt->add_flag(GETOPT_FLAG_COMMAND_LINE
                        | GETOPT_FLAG_FLAG
                        | GETOPT_FLAG_GROUP_COMMANDS);
            opt->set_help("show help from the \""
                        + name
                        + "\" group of options.");
            f_options_by_name[option_name] = opt;
        }
    }
}


/** \brief Search for \p group in the list of group names.
 *
 * This function is used to search for the name of a group.
 *
 * Groups are used by the usage() function to list options by some user
 * selected group.
 *
 * For example, it is often that a tool has a set of commands such as
 * `--delete` and a set of options such as `--verbose`. These can represent
 * to clear groups of commands and options.
 *
 * \param[in] group  The group to look for (i.e. GETOPT_FLAG_GROUP_ONE).
 *
 * \return The group structure or nullptr when not found.
 */
group_description const * getopt::find_group(flag_t group) const
{
    if(f_options_environment.f_groups == nullptr)
    {
        return nullptr;
    }

    if((group & ~GETOPT_FLAG_GROUP_MASK) != 0)
    {
        throw getopt_logic_error("group parameter must represent a valid group.");
    }
    if(group == GETOPT_FLAG_GROUP_NONE)
    {
        throw getopt_logic_error("group NONE cannot be assigned a name so you cannot search for it.");
    }

    for(group_description const * grp(f_options_environment.f_groups)
      ; grp->f_group != GETOPT_FLAG_GROUP_NONE
      ; ++grp)
    {
        if(group == grp->f_group)
        {
            if((grp->f_name == nullptr || *grp->f_name == '\0')
            && (grp->f_description == nullptr || *grp->f_description == '\0'))
            {
                throw getopt_logic_error("at least one of a group name or description must be defined (a non-empty string).");
            }
            return grp;
        }
    }

    // group not defined
    //
    return nullptr;
}


/** \brief Create a string of the command line arguments.
 *
 * This function assembles the command line arguments in a string and
 * returns that string.
 *
 * The function has the ability to wrap strings around for better formatting.
 *
 * The list of arguments to show is defined by the \p show parameter. When
 * \p show is 0, then only the regular and error arguments are shown.
 * Otherwise only the argumenst with the specified flags are show. Only
 * the `..._SHOW_...` flags are valid here.
 *
 * When an error occurs, it is customary to set \p show to
 * GETOPT_FLAG_SHOW_USAGE_ON_ERROR so only a limited set of arguments
 * are shown.
 *
 * The library offers two groups in case you have a command line tools
 * with a large number of options, those two can be used to only show
 * those specific set of options with using a specific `--help` argument.
 *
 * \note
 * This function does NOT print anything in the output. This is your
 * responsibility. We do it this way because you may be using a logger
 * and not want to print the usage in the \em wrong destination.
 *
 * \bug
 * The options are written from our map. This means the order will be
 * alphabetical and not the order in which you defined the options.
 * We are not looking into fixing this problem. That's just something
 * you want to keep in mind.
 *
 * \param[in] show  Selection of the options to show.
 *
 * \return The assembled command line arguments.
 */
std::string getopt::usage(flag_t show) const
{
    std::stringstream ss;

    flag_t specific_group(show & GETOPT_FLAG_GROUP_MASK);

    // ignore all the non-show flags
    //
    show &= GETOPT_FLAG_SHOW_USAGE_ON_ERROR
          | GETOPT_FLAG_SHOW_ALL
          | GETOPT_FLAG_SHOW_GROUP1
          | GETOPT_FLAG_SHOW_GROUP2
          | GETOPT_FLAG_SHOW_SYSTEM;

    size_t const line_width(get_screen_width());
    ss << breakup_line(process_help_string(f_options_environment.f_help_header), 0, line_width);

    std::string save_default;
    std::string save_help;

    flag_t pos(GETOPT_FLAG_GROUP_MINIMUM);
    flag_t group_max(GETOPT_FLAG_GROUP_MAXIMUM);
    if(f_options_environment.f_groups == nullptr)
    {
        group_max = GETOPT_FLAG_GROUP_MINIMUM;
        specific_group = GETOPT_FLAG_GROUP_NONE;
    }
    else if(specific_group != GETOPT_FLAG_GROUP_NONE)
    {
        // only display that specific group if asked to do so
        //
        pos = specific_group >> GETOPT_FLAG_GROUP_SHIFT;
        group_max = pos;
    }

    std::multimap<advgetopt::option_info::pointer_t, advgetopt::option_info::pointer_t> alias_reverse_references;
    for(auto const & opt : f_options_by_name)
    {
        if(!opt.second->has_flag(GETOPT_FLAG_ALIAS))
        {
            continue;
        }
        alias_reverse_references.insert(std::make_pair(opt.second->get_alias_destination(), opt.second));
    }

    for(; pos <= group_max; ++pos)
    {
        bool group_name_shown(false);
        flag_t const group(pos << GETOPT_FLAG_GROUP_SHIFT);
        for(auto const & opt : f_options_by_name)
        {
            if((opt.second->get_flags() & GETOPT_FLAG_GROUP_MASK) != group
            && f_options_environment.f_groups != nullptr)
            {
                // this could be optimized but we'd probably not see much
                // difference overall and it's just for the usage() call
                //
                continue;
            }

            std::string const help(opt.second->get_help());
            if(help.empty())
            {
                // ignore entries without help
                //
                continue;
            }

            if(opt.second->has_flag(GETOPT_FLAG_ALIAS))
            {
                // ignore entries representing an alias
                //
                continue;
            }

            if((show & GETOPT_FLAG_SHOW_ALL) == 0)
            {
                if(show != 0)
                {
                    if(!opt.second->has_flag(show))
                    {
                        // usage selected group is not present in this option, ignore
                        //
                        continue;
                    }
                }
                else if(opt.second->has_flag(GETOPT_FLAG_SHOW_GROUP1
                                           | GETOPT_FLAG_SHOW_GROUP2
                                           | GETOPT_FLAG_SHOW_SYSTEM))
                {
                    // do not show specialized groups
                    //
                    continue;
                }
            }

            if(!group_name_shown)
            {
                group_name_shown = true;

                if(group != GETOPT_FLAG_GROUP_NONE)
                {
                    group_description const * grp(find_group(group));
                    if(grp != nullptr)
                    {
                        ss << std::endl
                           << breakup_line(process_help_string(grp->f_description), 0, line_width);
                    }
                }
            }

            std::stringstream argument;

            if(opt.second->is_default_option())
            {
                switch(opt.second->get_flags() & (GETOPT_FLAG_REQUIRED | GETOPT_FLAG_MULTIPLE))
                {
                case 0:
                    argument << "[default argument]";
                    break;

                case GETOPT_FLAG_REQUIRED:
                    argument << "<default argument>";
                    break;

                case GETOPT_FLAG_MULTIPLE:
                    argument << "[default arguments]";
                    break;

                case GETOPT_FLAG_REQUIRED | GETOPT_FLAG_MULTIPLE:
                    argument << "<default arguments>";
                    break;

                }
            }
            else
            {
                argument << "--" << opt.second->get_name();
                auto aliases(alias_reverse_references.lower_bound(opt.second));
                if(aliases->first == opt.second)
                {
                    auto end(alias_reverse_references.upper_bound(opt.second));
                    for(auto a(aliases); a != end; ++a)
                    {
                        argument << " or --" << a->second->get_name();
                    }
                }
                if(opt.second->get_short_name() != NO_SHORT_NAME)
                {
                    argument << " or -" << short_name_to_string(opt.second->get_short_name());
                }
                if(aliases->first == opt.second)
                {
                    auto end(alias_reverse_references.upper_bound(opt.second));
                    for(; aliases != end; ++aliases)
                    {
                        if(aliases->second->get_short_name() != NO_SHORT_NAME)
                        {
                            argument << " or -" << short_name_to_string(aliases->second->get_short_name());
                        }
                    }
                }

                switch(opt.second->get_flags() & (GETOPT_FLAG_FLAG | GETOPT_FLAG_REQUIRED | GETOPT_FLAG_MULTIPLE))
                {
                case 0:
                    argument << " [<arg>]";
                    break;

                case GETOPT_FLAG_REQUIRED:
                    argument << " <arg>";
                    break;

                case GETOPT_FLAG_MULTIPLE:
                    argument << " {<arg>}";
                    break;

                case GETOPT_FLAG_REQUIRED | GETOPT_FLAG_MULTIPLE:
                    argument << " <arg> {<arg>}";
                    break;

                }
            }

            if(opt.second->has_flag(GETOPT_FLAG_DYNAMIC_CONFIGURATION))
            {
                argument << "*";
            }

            if(opt.second->has_default())
            {
                argument << " (default is \""
                         << opt.second->get_default()
                         << "\")";
            }

            // Output argument string with help
            //
            if(opt.second->is_default_option())
            {
                save_default = argument.str();
                save_help = help;
            }
            else
            {
                std::string variable_name;
                if(!opt.second->get_environment_variable_name().empty())
                {
                    variable_name += "\nEnvironment Variable Name: \"";
                    if(f_options_environment.f_environment_variable_intro != nullptr)
                    {
                        variable_name += f_options_environment.f_environment_variable_intro;
                    }
                    variable_name += opt.second->get_environment_variable_name();
                    variable_name += '"';
                }
                ss << format_usage_string(argument.str()
                                        , process_help_string((help + variable_name).c_str())
                                        , 30
                                        , line_width);
            }
        }
    }

    if(!save_default.empty())
    {
        ss << format_usage_string(save_default
                                , process_help_string(save_help.c_str())
                                , 30
                                , line_width);
    }

    if(f_options_environment.f_help_footer != nullptr
    && f_options_environment.f_help_footer[0] != '\0')
    {
        ss << std::endl;
        ss << breakup_line(process_help_string(f_options_environment.f_help_footer), 0, line_width);
    }

    return ss.str();
}


class usage_flag_traits
{
public:
    static constexpr snapdev::format_flag_t const       FORMAT_FLAG_EXTENDED    = 0x01; // '*'

    static bool is_flag(char c, snapdev::format_item<char> & f)
    {
        switch(c)
        {
        case '*':
            if(f.has_flags(FORMAT_FLAG_EXTENDED))
            {
                f.add_error(snapdev::format_error_t::FORMAT_ERROR_DUPLICATE);
            }
            f.add_flags(FORMAT_FLAG_EXTENDED);
            return true;

        default:
            return false;

        }
    }
};


class usage_letter_traits
{
public:
    static std::string::size_type is_format(char const * s, snapdev::format_item<char> & f)
    {
        switch(s[0])
        {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'i':
        case 'l':
        case 'm':
        case 'o':
        case 'p':
        case 's':
        case 't':
        case 'v':
        case 'w':
            f.format(s[0]);
            return 1UL;

        }

        f.add_error(snapdev::format_error_t::FORMAT_ERROR_UNKNOWN);
        return 0;
    }
};


/** \brief Change the % flags in help strings.
 *
 * This function goes through the help string and replaces the `%\<flag>`
 * with various content available in the getopt object.
 *
 * This is helpful for various reasons. For example, you may use the
 * same set of options in several different programs, in which case the
 * `%p` is likely useful to print out the name of the program currently
 * in use.
 *
 * Similarly we offer ways to print out lists of configuration files,
 * the environment variable name & value, etc. The following is the
 * list of supported flags:
 *
 * \li "%%" -- print out a percent
 * \li "%a" -- print out the project name (a.k.a. application name)
 * \li "%b" -- print out the build date
 * \li "%c" -- print out the copyright notice
 * \li "%d" -- print out the first directory with configuration files.
 * \li "%*d" -- print out the complete list of directories with configuration
 * files.
 * \li "%e" -- print out the name of the environment variable.
 * \li "%*e" -- print out the name and value of the environment variable.
 * \li "%f" -- print out the first configuration path and filename.
 * \li "%*f" -- print out all the configuration full paths.
 * \li "%g" -- print out the list of existing configuration files.
 * \li "%*g" -- print out the list of all possible configuration files.
 * \li "%i" -- print out the filenames of the option files.
 * \li "%*i" -- print out the path to the option files.
 * \li "%l" -- print out the license.
 * \li "%o" -- show the configuration filename where changes get written.
 * \li "%p" -- print out the program basename.
 * \li "%*p" -- print out the full program name.
 * \li "%s" -- print out the group name.
 * \li "%t" -- print out the build time.
 * \li "%v" -- print out the version.
 * \li "%w" -- print out the list of all the writable configuration files.
 *
 * Here is an example where the `%p` can be used:
 *
 * \code
 *    "Usage: %p [-opt] filename ..."
 * \endcode
 *
 * The other flags are more often used in places like the copyright notice
 * the footer, the license notice, etc.
 *
 * \param[in] help  A string that may include `%` flags.
 *
 * \return The string with any '%\<flag>' replaced.
 *
 * \sa parse_program_name()
 */
std::string getopt::process_help_string(char const * help) const
{
    if(help == nullptr)
    {
        return std::string();
    }

    snapdev::format_item<char>::list_t items(snapdev::tokenize_format<
              char
            , usage_letter_traits
            , usage_flag_traits>(help));

    for(auto it(items.begin()); it != items.end(); ++it)
    {
        switch(it->format())
        {
        case 'a':
            if(f_options_environment.f_project_name != nullptr)
            {
                it->string(f_options_environment.f_project_name);
            }
            break;

        case 'b':
            if(f_options_environment.f_build_date != nullptr)
            {
                it->string(f_options_environment.f_build_date);
            }
            break;

        case 'c':
            if(f_options_environment.f_copyright != nullptr)
            {
                it->string(f_options_environment.f_copyright);
            }
            break;

        case 'd':
            if(f_options_environment.f_configuration_directories != nullptr
            && *f_options_environment.f_configuration_directories != nullptr)
            {
                if(it->has_flags(usage_flag_traits::FORMAT_FLAG_EXTENDED))
                {
                    std::string joined;
                    for(char const * const * directories(f_options_environment.f_configuration_directories);
                        *directories != nullptr;
                        ++directories)
                    {
                        if(!joined.empty())
                        {
                            joined += ", ";
                        }
                        joined += *directories;
                    }
                    it->string(joined);
                }
                else
                {
                    it->string(*f_options_environment.f_configuration_directories);
                }
            }
            break;

        case 'e':
            if(f_options_environment.f_environment_variable_name != nullptr
            && *f_options_environment.f_environment_variable_name != '\0')
            {
                if(it->has_flags(usage_flag_traits::FORMAT_FLAG_EXTENDED))
                {
                    std::string var(f_options_environment.f_environment_variable_name);
                    char const * env(getenv(f_options_environment.f_environment_variable_name));
                    if(env != nullptr)
                    {
                        var += '=';
                        var += env;
                    }
                    else
                    {
                        var += " (not set)";
                    }
                    it->string(var);
                }
                else
                {
                    it->string(f_options_environment.f_environment_variable_name);
                }
            }
            break;

        case 'E':
            if(f_options_environment.f_environment_variable_intro != nullptr)
            {
                it->string(f_options_environment.f_environment_variable_intro);
            }
            break;

        case 'f':
            if(f_options_environment.f_configuration_files != nullptr
            && *f_options_environment.f_configuration_files != nullptr)
            {
                if(it->has_flags(usage_flag_traits::FORMAT_FLAG_EXTENDED))
                {
                    std::string joined;
                    for(char const * const * filenames(f_options_environment.f_configuration_files);
                        *filenames != nullptr;
                        ++filenames)
                    {
                        if(!joined.empty())
                        {
                            joined += ", ";
                        }
                        joined += *filenames;
                    }
                    it->string(joined);
                }
                else
                {
                    it->string(*f_options_environment.f_configuration_files);
                }
            }
            break;

        case 'g':
            {
                string_list_t const list(get_configuration_filenames(!it->has_flags(usage_flag_traits::FORMAT_FLAG_EXTENDED), false));
                it->string(snapdev::join_strings(list, ", "));
            }
            break;

        case 'i':
            if(it->has_flags(usage_flag_traits::FORMAT_FLAG_EXTENDED))
            {
                it->string(get_path_to_option_files());
            }
            else
            {
                string_list_t const list(get_filenames_of_option_definitions());
                it->string(snapdev::join_strings(list, ", "));
            }
            break;

        case 'l':
            if(f_options_environment.f_license != nullptr)
            {
                it->string(f_options_environment.f_license);
            }
            break;

        case 'm':
            if(f_options_environment.f_section_variables_name != nullptr)
            {
                it->string(f_options_environment.f_section_variables_name);
            }
            break;

        case 'o':
            it->string(get_output_filename());
            break;

        case 'p':
            if(it->has_flags(usage_flag_traits::FORMAT_FLAG_EXTENDED))
            {
                it->string(f_program_fullname);
            }
            else
            {
                it->string(f_program_name);
            }
            break;

        case 's':
            if(f_options_environment.f_group_name != nullptr)
            {
                it->string(f_options_environment.f_group_name);
            }
            break;

        case 't':
            if(f_options_environment.f_build_time != nullptr)
            {
                it->string(f_options_environment.f_build_time);
            }
            break;

        case 'v':
            if(f_options_environment.f_version != nullptr)
            {
                it->string(f_options_environment.f_version);
            }
            break;

        case 'w':
            {
                string_list_t const list(get_configuration_filenames(true, true));
                it->string(snapdev::join_strings(list, ", "));
            }
            break;

        }
    }

    snapdev::format_item<char> empty_item;
    return snapdev::join_strings(items, empty_item);
}




} // namespace advgetopt
// vim: ts=4 sw=4 et
