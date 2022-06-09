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
#pragma once

/** \file
 * \brief Definitions of the options class a initialization functions.
 *
 * The advgetopt library offers an advanced way to manage your command
 * line tools options on the command line, in environment variables, and
 * in configuration files.
 */


// C++
//
#include    <cstdint>



namespace advgetopt
{



typedef std::uint32_t       flag_t;

static constexpr flag_t     GETOPT_FLAG_NONE                  = static_cast<flag_t>(0x00000000);

static constexpr flag_t     GETOPT_FLAG_COMMAND_LINE          = static_cast<flag_t>(0x00000001);  // acceptable on the command line
static constexpr flag_t     GETOPT_FLAG_ENVIRONMENT_VARIABLE  = static_cast<flag_t>(0x00000002);  // acceptable in environment variable
static constexpr flag_t     GETOPT_FLAG_CONFIGURATION_FILE    = static_cast<flag_t>(0x00000004);  // acceptable in configuration files
static constexpr flag_t     GETOPT_FLAG_DYNAMIC_CONFIGURATION = static_cast<flag_t>(0x00000008);  // acceptable from the dynamic configuration system (see fluid-settings)

static constexpr flag_t     GETOPT_FLAG_ALIAS                 = static_cast<flag_t>(0x00000010);  // alias, result in another option defined in "help" string
static constexpr flag_t     GETOPT_FLAG_FLAG                  = static_cast<flag_t>(0x00000020);  // no parameter allowed (--help)
static constexpr flag_t     GETOPT_FLAG_REQUIRED              = static_cast<flag_t>(0x00000040);  // required (--host 127.0.0.1)
static constexpr flag_t     GETOPT_FLAG_MULTIPLE              = static_cast<flag_t>(0x00000080);  // any number of parameter is allowed (--files a b c d ...)
static constexpr flag_t     GETOPT_FLAG_DEFAULT_OPTION        = static_cast<flag_t>(0x00000100);  // where entries go by default (a.k.a. after "--")
static constexpr flag_t     GETOPT_FLAG_HAS_DEFAULT           = static_cast<flag_t>(0x00000200);  // default value is defined
static constexpr flag_t     GETOPT_FLAG_PROCESS_VARIABLES     = static_cast<flag_t>(0x00000400);  // variables within this parameter will automatically be processed

static constexpr flag_t     GETOPT_FLAG_SHOW_MOST             = static_cast<flag_t>(0x00000000);  // show in usage() when not in GROUP1 or GROUP2
static constexpr flag_t     GETOPT_FLAG_SHOW_USAGE_ON_ERROR   = static_cast<flag_t>(0x00001000);  // show in usage() when an error occurs
static constexpr flag_t     GETOPT_FLAG_SHOW_ALL              = static_cast<flag_t>(0x00002000);  // show in usage() when --long-help is used
static constexpr flag_t     GETOPT_FLAG_SHOW_GROUP1           = static_cast<flag_t>(0x00004000);  // show in usage() when --<group1>-help is used (app dependent)
static constexpr flag_t     GETOPT_FLAG_SHOW_GROUP2           = static_cast<flag_t>(0x00008000);  // show in usage() when --<group2>-help is used (app dependent)

static constexpr flag_t     GETOPT_FLAG_GROUP_MASK            = static_cast<flag_t>(0x00700000);
static constexpr flag_t     GETOPT_FLAG_GROUP_MINIMUM         = static_cast<flag_t>(0);
static constexpr flag_t     GETOPT_FLAG_GROUP_MAXIMUM         = static_cast<flag_t>(7);
static constexpr flag_t     GETOPT_FLAG_GROUP_SHIFT           = static_cast<flag_t>(20);
static constexpr flag_t     GETOPT_FLAG_GROUP_NONE            = static_cast<flag_t>(0x00000000);  // not in a group
static constexpr flag_t     GETOPT_FLAG_GROUP_COMMANDS        = static_cast<flag_t>(0x00100000);  // in command group (group 1)
static constexpr flag_t     GETOPT_FLAG_GROUP_OPTIONS         = static_cast<flag_t>(0x00200000);  // in option group (group 2)
static constexpr flag_t     GETOPT_FLAG_GROUP_THREE           = static_cast<flag_t>(0x00300000);  // in group 3
static constexpr flag_t     GETOPT_FLAG_GROUP_FOUR            = static_cast<flag_t>(0x00400000);  // in group 4
static constexpr flag_t     GETOPT_FLAG_GROUP_FIVE            = static_cast<flag_t>(0x00500000);  // in group 5
static constexpr flag_t     GETOPT_FLAG_GROUP_SIX             = static_cast<flag_t>(0x00600000);  // in group 6
static constexpr flag_t     GETOPT_FLAG_GROUP_SEVEN           = static_cast<flag_t>(0x00700000);  // in group 7

static constexpr flag_t     GETOPT_FLAG_DYNAMIC               = static_cast<flag_t>(0x20000000);  // this value was found in a configuration file and dynamic parameters are allowed (i.e. no definition for this option was found)
static constexpr flag_t     GETOPT_FLAG_LOCK                  = static_cast<flag_t>(0x40000000);  // this value is currently locked (can't be modified)

static constexpr flag_t     GETOPT_FLAG_END                   = static_cast<flag_t>(0x80000000);  // mark the end of the list



template<class none = void>
constexpr flag_t option_flags_merge()
{
    return GETOPT_FLAG_NONE;
}


template<flag_t flag, flag_t ...args>
constexpr flag_t option_flags_merge()
{
    static_assert((flag & GETOPT_FLAG_GROUP_MASK) == 0
               || (option_flags_merge<args...>() & GETOPT_FLAG_GROUP_MASK) == 0
               , "more than one GETOPT_FLAG_GROUP_... is not allowed within one set of flags.");

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
constexpr flag_t any_flags()
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
    constexpr flag_t result(combine_option_flags<GETOPT_FLAG_CONFIGURATION_FILE, GETOPT_FLAG_REQUIRED, args...>());

    static_assert((result & (GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_ENVIRONMENT_VARIABLE)) == 0
                , "a config_flag() cannot include GETOPT_FLAG_COMMAND_LINE | GETOPT_FLAG_ENVIRONMENT_VARIABLE");

    return result;
}






}   // namespace advgetopt
// vim: ts=4 sw=4 et
