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
#pragma once

/** \file
 * \brief Definitions of the options class a initialization functions.
 *
 * The advgetopt library offers an advanced way to manage your command
 * line tools options on the command line, in environment variables, and
 * in configuration files.
 */


// advgetopt lib
//
#include    "advgetopt/option_info.h"


// snapdev lib
//
#include    <snapdev/not_used.h>



namespace advgetopt
{






// this structure is used to declare your command line options in a
// constexpr array
//
// TODO: plan to transform all the strings in an array with a scheme such
//       as:
//
//          "a:<alias>",
//          "d:<default>",
//          "h:<help>",
//          "n:<name>",
//          "s:<separator>",
//          "v:<validator>(<param>, ...)"
//
//       our templates should be able to create that array automatically.
//       This way we avoid many nullptr in so many definitions (i.e. most
//       our definitions do not have a default, separators, or a validator)
//       We would also avoid the alias/help overload.
//
struct option
{
    short_name_t        f_short_name = NO_SHORT_NAME;   // letter option (or '\0')
    flag_t              f_flags = GETOPT_FLAG_NONE;     // set of flags
    char const *        f_name = nullptr;               // name of the option (i.e. "test" for --test, or nullptr)
    char const *        f_default = nullptr;            // a default value if not nullptr
    char const *        f_help = nullptr;               // help for this option, if nullptr it's a hidden option; if ALIAS then this is the actual alias
    char const *        f_validator = nullptr;          // the name of a validator and optional parameters between parenthesis
    char const * const *f_multiple_separators = nullptr;// nullptr terminated list of strings used as separators when GETOPT_FLAG_MULTIPLE is set
};



template<typename T>
class OptionValue
{
public:
    typedef T   value_t;

    constexpr OptionValue<T>(T const v)
        : f_value(v)
    {
    }

    constexpr value_t get() const
    {
        return f_value;
    }

private:
    value_t     f_value;
};


class ShortName
    : public OptionValue<short_name_t>
{
public:
    constexpr ShortName()
        : OptionValue<short_name_t>(NO_SHORT_NAME)
    {
    }

    constexpr ShortName(short_name_t name)
        : OptionValue<short_name_t>(name)
    {
    }
};

class Flags
    : public OptionValue<flag_t>
{
public:
    constexpr Flags()
        : OptionValue<flag_t>(GETOPT_FLAG_NONE)
    {
    }

    constexpr Flags(flag_t flags)
        : OptionValue<flag_t>(flags)
    {
    }
};

class Name
    : public OptionValue<char const *>
{
public:
    constexpr Name()
        : OptionValue<char const *>(nullptr)
    {
    }

    constexpr Name(char const * name)
        : OptionValue<char const *>(name)
    {
    }
};

class DefaultValue
    : public OptionValue<char const *>
{
public:
    constexpr DefaultValue()
        : OptionValue<char const *>(nullptr)
    {
    }

    constexpr DefaultValue(char const * default_value)
        : OptionValue<char const *>(default_value)
    {
    }
};

class Alias
    : public OptionValue<char const *>
{
public:
    constexpr Alias()
        : OptionValue<char const *>(nullptr)
    {
    }

    constexpr Alias(char const * alias)
        : OptionValue<char const *>(alias)
    {
    }
};

class Help
    : public OptionValue<char const *>
{
public:
    constexpr Help()
        : OptionValue<char const *>(nullptr)
    {
    }

    constexpr Help(char const * help)
        : OptionValue<char const *>(help)
    {
    }
};

class Validator
    : public OptionValue<char const *>
{
public:
    constexpr Validator()
        : OptionValue<char const *>(nullptr)
    {
    }

    constexpr Validator(char const * validator)
        : OptionValue<char const *>(validator)
    {
    }
};

class Separators
    : public OptionValue<char const * const *>
{
public:
    constexpr Separators()
        : OptionValue<char const * const *>(nullptr)
    {
    }

    constexpr Separators(char const * const * separators)
        : OptionValue<char const * const *>(separators)
    {
    }
};



template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<std::is_same<T, F>::value, typename T::value_t>::type find_option(F first, ARGS ...args)
{
    snap::NOTUSED(args...);
    return first.get();
}


template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<!std::is_same<T, F>::value, typename T::value_t>::type find_option(F first, ARGS ...args)
{
    snap::NOTUSED(first);
    return find_option<T>(args...);
}



template<class ...ARGS>
constexpr option define_option(ARGS ...args)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    option opt =
    {
        .f_short_name =          find_option<ShortName   >(args..., ShortName()),
        .f_flags =               find_option<Flags       >(args..., Flags())
                                    | (find_option<Alias       >(args..., Alias()) != nullptr
                                            ? GETOPT_FLAG_ALIAS
                                            : GETOPT_FLAG_NONE),
        .f_name =                find_option<Name        >(args...),    // no default, must be defined
        .f_default =             find_option<DefaultValue>(args..., DefaultValue()),
        .f_help =                find_option<Alias       >(args..., Alias()) != nullptr
                                    ? find_option<Alias       >(args..., Alias())
                                    : find_option<Help        >(args..., Help()),
        .f_validator =           find_option<Validator   >(args..., Validator()),
        .f_multiple_separators = find_option<Separators  >(args..., Separators()),
    };
#pragma GCC diagnostic pop

    // TODO: once possible (C++17/20?) add verification tests here

    return opt;
}





constexpr option end_options()
{
    return define_option(
              advgetopt::Name(nullptr)
            , advgetopt::Flags(advgetopt::end_flags())
        );
}








struct group_description
{
    flag_t          f_group = GETOPT_FLAG_GROUP_NONE;   // the default is used to mark the end of the list
    char const *    f_name = nullptr;                   // for --<name>-help
    char const *    f_description = nullptr;            // for usage() output
};




template<typename T>
class GroupValue
{
public:
    typedef T   value_t;

    constexpr GroupValue<T>(T const v)
        : f_value(v)
    {
    }

    constexpr value_t get() const
    {
        return f_value;
    }

private:
    value_t     f_value;
};

class GroupNumber
    : public OptionValue<flag_t>
{
public:
    constexpr GroupNumber()
        : OptionValue<flag_t>(GETOPT_FLAG_GROUP_NONE)
    {
    }

    constexpr GroupNumber(flag_t group)
        : OptionValue<flag_t>(group)
    {
    }
};

class GroupName
    : public GroupValue<char const *>
{
public:
    constexpr GroupName()
        : GroupValue<char const *>(nullptr)
    {
    }

    constexpr GroupName(char const * name)
        : GroupValue<char const *>(name)
    {
    }
};

class GroupDescription
    : public GroupValue<char const *>
{
public:
    constexpr GroupDescription()
        : GroupValue<char const *>(nullptr)
    {
    }

    constexpr GroupDescription(char const * description)
        : GroupValue<char const *>(description)
    {
    }
};


template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<std::is_same<T, F>::value, typename T::value_t>::type find_group(F first, ARGS ...args)
{
    snap::NOTUSED(args...);
    return first.get();
}


template<typename T, typename F, class ...ARGS>
constexpr typename std::enable_if<!std::is_same<T, F>::value, typename T::value_t>::type find_group(F first, ARGS ...args)
{
    snap::NOTUSED(first);
    return find_group<T>(args...);
}



template<class ...ARGS>
constexpr group_description define_group(ARGS ...args)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    group_description grp =
    {
        .f_group =              find_group<GroupNumber     >(args..., GroupNumber()),
        .f_name =               find_group<GroupName       >(args..., GroupName()),
        .f_description =        find_group<GroupDescription>(args..., GroupDescription()),
    };
#pragma GCC diagnostic pop

    return grp;
}


constexpr group_description end_groups()
{
    // the defaults are what we expect to end the list of groups
    return define_group();
}








constexpr flag_t    GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS          = 0x0001;   // accept parameters that are not declared

constexpr flag_t    GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS           = 0x0002;   // add system parameters (i.e. --help, --version, etc.)
constexpr flag_t    GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS   = 0x0004;   // add & process system parameters


struct options_environment
{
    char const *                f_project_name = nullptr;               // project/application name--used as filename for the .conf files (%a)
    char const *                f_group_name = nullptr;                 // sub-folder name (i.e. "snapwebsites")--if nullptr, use f_project_name
    option const *              f_options = nullptr;                    // raw options
    char const *                f_options_files_directory = nullptr;    // directory to check for option files (default "/usr/shared/advgetopt")
    char const *                f_environment_variable_name = nullptr;  // environment variable with additional options (%e)
    char const * const *        f_configuration_files = nullptr;        // nullptr terminated array of full paths to configuration files (%f)
    char const *                f_configuration_filename = nullptr;     // the configuration filename to search in f_configuration_directories (%g)
    char const * const *        f_configuration_directories = nullptr;  // nullptr terminated array of paths only to configuration files (%d)
    flag_t                      f_environment_flags = 0;                // GETOPT_ENVIRONMENT_FLAG_...
    char const *                f_help_header = nullptr;                // show on --help
    char const *                f_help_footer = nullptr;                // show on --help
    char const *                f_version = nullptr;                    // show on --version and %v
    char const *                f_license = nullptr;                    // show on --license and %l
    char const *                f_copyright = nullptr;                  // show on --copyright and %c
    char const *                f_build_date = UTC_BUILD_DATE;          // available to parameter %b
    char const *                f_build_time = UTC_BUILD_TIME;          // available to parameter %t
    group_description const *   f_groups = nullptr;                     // nullptr terminated array of group names
};





}   // namespace advgetopt
// vim: ts=4 sw=4 et
