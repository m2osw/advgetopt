/*
 * File:
 *    advgetopt/options.h -- a replacement to the Unix getopt() implementation
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

// C++ lib
//
#include <iostream>


namespace advgetopt
{





template<class none = void>
constexpr flag_t option_flags_merge()
{
    return GETOPT_FLAG_NONE;
}


template<flag_t flag, flag_t ...args>
constexpr flag_t option_flags_merge()
{
    return flag | option_flags_merge<args...>();
}


template<flag_t flag, flag_t ...args>
constexpr flag_t combine_option_flags()
{
    constexpr flag_t result(option_flags_merge<flag, args...>());

    static_assert(static_cast<int>((result & GETOPT_FLAG_FLAG) != 0)
                + static_cast<int>((result & (GETOPT_FLAG_REQUIRED | GETOPT_FLAG_MULTIPLE | GETOPT_FLAG_DEFAULT_OPTION)) != 0)
                + static_cast<int>((result & GETOPT_FLAG_END) != 0)
                        <= 1
                , "flag GETOPT_FLAG_FLAG is not compatible with any of GETOPT_FLAG_REQUIRED | GETOPT_FLAG_MULTIPLE | GETOPT_FLAG_DEFAULT_OPTION or none of these flags were specified.");

    static_assert(((result & (GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE)) != 0)
                ^ ((result & GETOPT_FLAG_END) != 0)
                , "flags must include at least one of GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE or be set to GETOPT_FLAG_END");

    return result;
}


constexpr flag_t end_flags()
{
    return combine_option_flags<GETOPT_FLAG_END>();
}


template<flag_t ...args>
constexpr flag_t optional_flags()
{
    constexpr flag_t result(combine_option_flags<args...>());

    static_assert((result & GETOPT_FLAG_END) == 0
                , "an option_flag() cannot include GETOPT_FLAG_END");

    return result;
}


template<flag_t ...args>
constexpr flag_t option_flags()
{
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_FLAG, args...>());

    //static_assert((result & (GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE)) == 0
    //            , "an option_flag() cannot include GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE");

    return result;
}


template<flag_t ...args>
constexpr flag_t all_flags()
{
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_COMMAND_LINE
                                               , GETOPT_FLAG_ENVIRONMENT_VARIABLE
                                               , GETOPT_FLAG_CONFIGURATION_FILE
                                               , args...>());

    return result;
}


template<flag_t ...args>
constexpr flag_t standalone_all_flags()
{
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_COMMAND_LINE
                                               , GETOPT_FLAG_ENVIRONMENT_VARIABLE
                                               , GETOPT_FLAG_CONFIGURATION_FILE
                                               , GETOPT_FLAG_FLAG
                                               , args...>());

    return result;
}


template<flag_t ...args>
constexpr flag_t standalone_command_flags()
{
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_COMMAND_LINE, GETOPT_FLAG_FLAG, args...>());

    static_assert((result & (GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE)) == 0
                , "an option_flag() cannot include GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE");

    return result;
}


template<flag_t ...args>
constexpr flag_t command_flags()
{
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_COMMAND_LINE, args...>());

    static_assert((result & (GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE)) == 0
                , "an option_flag() cannot include GETOPT_FLAG_ENVIRONMENT_VARIABLE | GETOPT_FLAG_CONFIGURATION_FILE");

    return result;
}


template<flag_t ...args>
constexpr flag_t var_flags()
{
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_ENVIRONMENT_VARIABLE, args...>());

    static_assert((result & (GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_CONFIGURATION_FILE)) == 0
                , "a config_flag() cannot include GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_CONFIGURATION_FILE");

    return result;
}


template<flag_t ...args>
constexpr flag_t config_flags()
{
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_CONFIGURATION_FILE, GETOPT_FLAG_FLAG, args...>());

    static_assert((result & (GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_ENVIRONMENT_VARIABLE)) == 0
                , "a config_flag() cannot include GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_ENVIRONMENT_VARIABLE");

    return result;
}



// this structure is used to declare your command line options in a
// constexpr array
//
struct option
{
    short_name_t        f_short_name = NO_SHORT_NAME;   // letter option (or '\0')
    flag_t              f_flags = GETOPT_FLAG_NONE;     // set of flags
    char const *        f_name = nullptr;               // name of the option (i.e. "test" for --test, or nullptr)
    char const *        f_default = nullptr;            // a default value if not nullptr
    char const *        f_help = nullptr;               // help for this option, if nullptr it's a hidden option; if ALIAS then this is the actual alias
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

    constexpr DefaultValue(char const * help)
        : OptionValue<char const *>(help)
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








constexpr flag_t    GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS  = 0x0001;   // accept parameters that are not declared
constexpr flag_t    GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS   = 0x0002;   // add system parameters (i.e. --help, --version, etc.)


struct options_environment
{
    char const *                f_project_name = nullptr;               // project/application name--used as filename for the .conf files
    option const *              f_options = nullptr;                    // raw options
    char const *                f_options_files_directory = nullptr;    // directory to check for option files (default "/usr/shared/advgetopt")
    char const *                f_environment_variable_name = nullptr;  // environment variable with additional options
    char const * const *        f_configuration_files = nullptr;        // nullptr terminated array of full paths to configuration files
    char const *                f_configuration_filename = nullptr;     // the configuration filename to search in f_configuration_directories
    char const * const *        f_configuration_directories = nullptr;  // nullptr terminated array of paths only to configuration files
    flag_t                      f_environment_flags = 0;                // GETOPT_ENVIRONMENT_FLAG_...
    char const *                f_help_header = nullptr;                // show on --help
    char const *                f_help_footer = nullptr;                // show on --help
    char const *                f_version = nullptr;                    // show on --version and %v
    char const *                f_license = nullptr;                    // show on --license and %l
    char const *                f_copyright = nullptr;                  // show on --copyright and %c
    char const *                f_build_date = __DATE__;                // available to parameter %b
    char const *                f_build_time = __TIME__;                // available to parameter %t
};





}   // namespace advgetopt
// vim: ts=4 sw=4 et
