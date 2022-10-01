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
 * \brief Implementation of the option_info class.
 *
 * This is the implementation of the class used to define one command
 * line option.
 */

// self
//
#include    "advgetopt/option_info.h"

#include    "advgetopt/exception.h"
#include    "advgetopt/validator_double.h"
#include    "advgetopt/validator_integer.h"


// cppthread
//
#include    <cppthread/guard.h>
#include    <cppthread/log.h>
#include    <cppthread/mutex.h>


// snapdev
//
#include    <snapdev/not_used.h>
#include    <snapdev/tokenize_string.h>


// libutf8
//
#include    <libutf8/libutf8.h>
#include    <libutf8/iterator.h>


// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{


namespace
{


/** \brief The library trace mode.
 *
 * This flag is used to determine whether the source of each option should
 * be traced. Very often, I have a problem where I'm not so sure where a
 * certain option was defined and how to fix the value of that option.
 * This flag allows us to debug that information at run time.
 *
 * When the flag is set to true (automatically done by the getopt object
 * when argv includes the "--show-sources" command line option), the sources
 * start to be traced. Once all the parsing is done, getopt again will check
 * whether it has the "--show-sources" command line option specified and if
 * so, it prints out all the options current values and the various sources
 * that were involved.
 */
bool g_trace_sources = false;


/** \brief The filename of the configuration being processed.
 *
 * This variable holds the filename of the configuration currently
 * being processed. This information is used to generate the trace
 * of the sources. That way it is possible to see where the current
 * value of a given variable comes from.
 *
 * This parameter is currently set from the
 * getopt::process_configuration_file() function.
 */
std::string g_configuration_filename = std::string();



} // no name namespace



// from utils.cpp
//
// (it's here because we do not want to make cppthread public in
// out header files--we could have an advgetopt_private.h, though)
//
cppthread::mutex &  get_global_mutex();




/** \brief Transform a string to a short name.
 *
 * This function transforms a string to a short name. The input string
 * can represent a UTF-8 character that can be used as a short name.
 *
 * An empty string is not considered to represent any name and thus
 * this function returns NO_SHORT_NAME when the input is an empty
 * string.
 *
 * \param[in] name  The name to be checked.
 *
 * \return The short name character or NO_SHORT_NAME if it's not a match.
 */
short_name_t string_to_short_name(std::string const & name)
{
    if(!name.empty())
    {
        libutf8::utf8_iterator u8(name);
        short_name_t const short_name(*u8++);
        if(u8 == name.end())
        {
            return short_name;
        }
    }

    return NO_SHORT_NAME;
}


/** \brief Convert a short name to a UTF-8 string.
 *
 * This function is the opposite of the to_short_name() except that the
 * input is expected to be a valid short name or NO_SHORT_NAME.
 *
 * When the input is NO_SHORT_NAME, the function outputs an empty string.
 *
 * \note
 * There are other short names that are not really considered valid such
 * as control characters, the dash (-), and probably most other
 * punctuation, character codes which are not currently assigned to
 * any character in Unicode, etc. This function ignores all of those
 * potential problems.
 *
 * \param[in] short_name  The short name to convert to UTF-8.
 *
 * \return The short name as a UTF-8 string or an empty string.
 */
std::string short_name_to_string(short_name_t short_name)
{
    if(short_name == NO_SHORT_NAME)
    {
        return std::string();
    }
    return libutf8::to_u8string(short_name);
}


/** \brief Create a new option_info object.
 *
 * This function creates a new option_info object with the specified \p name
 * and \p short_name. The \p short_name is optional.
 *
 * When adding options to a map of options, all the long and short names
 * must be unique. See the add_child() function for details.
 *
 * The \p short_name parameter is a UTF-32 character. To not offer a short
 * name for an option, use NO_SHORT_NAME as the value (which is the default
 * if not specified to the constructor.)
 *
 * \li Special Option Name: "--"
 *
 * The "--" long name is viewed as the separator between options and
 * \em filenames. When "--" is found by itself on the command line, then
 * it is viewed as a switch to go from having options to only having
 * \em filenames. Of course, these options may be used as any type of
 * values, not just filenames (it could be URLs, email addresses, numbers,
 * etc.)
 *
 * The "--" separator cannot be assigned a short name.
 *
 * \li Special Option Name: "*"
 *
 * The "*" long name is viewed as the \em accept \em all option. This
 * means all the options may not be defined in the list of options but
 * we still want to accept them. This is to allow dynamically defined
 * (supported) command options and especially to not have to declare
 * all the valid options found in a configuration file.
 *
 * \li Underscore and Dashes
 *
 * It is customary to support dashes between words in options appearing
 * on the command line (`--help-me`), however, it is unusual in
 * configuration files where underscores are used instead (`under_score`.)
 * When we compare option names, `'-' == '_'` is always considered true
 * so either dashes or underscore can be used in both cases.
 *
 * For this reason, the long name is saved with only dashes. That
 * means all the maps are indexed using the long name with dashes.
 *
 * \exception getopt_exception_logic
 * The constructor raises the invalid exception if the long name is an
 * empty string since this is not allowed. It will also raise that
 * exception if the name is the default option ("--") and a short name
 * is also defined. (i.e. no short name is allowed along the default
 * option.)
 *
 * \param[in] name  The (long) name of this option.
 * \param[in] short_name  The short name of this option (one character.)
 */
option_info::option_info(std::string const & name, short_name_t short_name)
    : f_name(option_with_dashes(name))
    , f_short_name(short_name)
{
    if(f_name.empty())
    {
        if(short_name != NO_SHORT_NAME)
        {
            throw getopt_logic_error(
                          "option_info::option_info(): all options must at least have a long name (short name: '"
                        + libutf8::to_u8string(short_name)
                        + "'.)");
        }
        throw getopt_logic_error(
                      "option_info::option_info(): all options must at least have a long name.");
    }

    if(f_name == "--")
    {
        if(short_name != NO_SHORT_NAME)
        {
            throw getopt_logic_error(
                          "option_info::option_info(): the default parameter \"--\" cannot include a short name ('"
                        + libutf8::to_u8string(short_name)
                        + "'.)");
        }

        add_flag(GETOPT_FLAG_DEFAULT_OPTION);
    }
    else
    {
        if(f_name[0] == '-')
        {
            throw getopt_logic_error(
                          "option_info::option_info(): an option cannot start with a dash (-), \""
                        + f_name
                        + "\" is not valid.");
        }

        if(short_name == '-')
        {
            throw getopt_logic_error(
                          "option_info::option_info(): the short name of an option cannot be the dash (-).");
        }
    }
}


/** \brief Get the long name of the option.
 *
 * This option retrieves the long name of the option.
 *
 * \note
 * Remember that the underscores in long names are converted to dashes.
 * This is because it makes more sense to look for command line parameters
 * with dashes. This function will return the name with only dashes.
 *
 * \note
 * The name is always defined. The creation of an option_info object
 * fails if the name is empty.
 *
 * \return The long name with dashes instead of underscores.
 */
std::string const & option_info::get_name() const
{
    return f_name;
}


/** \brief Assign a short name to an option.
 *
 * This function is used to assign a short name to an option.
 *
 * It can be changed to anything, including the NO_SHORT_NAME special
 * value.
 *
 * \warning
 * If you want this function to work as expected (i.e. for the option
 * to later be found using its short name), make sure to call the
 * getopt::set_short_name() on your getopt object and not directly this
 * function. This is because the getopt object needs to add the newly
 * named option to its map of options sorted by short name.
 *
 * \param[in] short_name  The short name to assign to this option.
 *
 * \sa get_short_name()
 * \sa getopt::set_short_name()
 */
void option_info::set_short_name(short_name_t short_name)
{
    f_short_name = short_name;
}


/** \brief Get the short name of the option.
 *
 * This function returns the \p short_name of this option.
 *
 * The short name is a Unicode character (UTF-32).
 *
 * \return The short name character.
 */
short_name_t option_info::get_short_name() const
{
    return f_short_name;
}


/** \brief Retrieve the name of the option without any section names.
 *
 * The name of an option can include section names. These
 * are rarely used on the command line, but they are useful for
 * configuration files if you want to create multiple layers of
 * options (a.k.a. sections.)
 *
 * This function removes all the section names from the option name
 * and returns what's left.
 *
 * \return The base name without any section names.
 */
std::string option_info::get_basename() const
{
    std::string::size_type const pos(f_name.rfind("::"));
    if(pos == std::string::npos)
    {
        return f_name;
    }

    return f_name.substr(pos + 2);
}


/** \brief Retrieve the name of the sections.
 *
 * The name of an option can include section names. These
 * are rarely used on the command line, but they are useful for
 * configuration files if you want to create multiple layers of
 * options (a.k.a. sections.)
 *
 * This function returns all the section names found in this option
 * name. The last scope operator gets removed too.
 *
 * If the name does not include any sections, then this function returns
 * an empty string.
 *
 * \return The section names without the basename.
 */
std::string option_info::get_section_name() const
{
    std::string::size_type const pos(f_name.rfind("::"));
    if(pos == std::string::npos)
    {
        return std::string();
    }

    return f_name.substr(0, pos);
}


/** \brief Retrieve a list of section names.
 *
 * The name of an option can include section names. These
 * are rarely used on the command line, but they are useful for
 * configuration files if you want to create multiple layers of
 * options (a.k.a. sections.)
 *
 * This function returns a string_list_t of the section names found in
 * this option name.
 *
 * If the name does not include any sections, then this function returns
 * an empty list.
 *
 * \return The list of section name.
 */
string_list_t option_info::get_section_name_list() const
{
    std::string::size_type const pos(f_name.rfind("::"));
    if(pos == std::string::npos)
    {
        return string_list_t();
    }

    string_list_t section_list;
    snapdev::tokenize_string(section_list
                        , f_name.substr(0, pos)
                        , "::"
                        , true
                        , std::string()
                        , &snapdev::string_predicate<string_list_t>);
    return section_list;
}


/** \brief Check whether this is the default option.
 *
 * This function checks whether this option represents the default option.
 * The default option is where non-options, generally filenames, are added
 * when not following an argument.
 *
 * The name of the default option is always "--". However, it is not
 * required. When no default option is defined, filenames can't be
 * specified and when such are found on the command line, an error
 * ensues.
 *
 * \return true if the name of the option is "--".
 */
bool option_info::is_default_option() const
{
    return has_flag(GETOPT_FLAG_DEFAULT_OPTION)
        || (f_name.size() == 2 && f_name[0] == '-' && f_name[1] == '-');
}


/** \brief Set the option specific environment variable name.
 *
 * Each option can be given a specific environment variable name. That
 * parameter is used to retrieve the option value if not defined on the
 * command line.
 *
 * By default this is an empty string.
 *
 * \param[in] name  The name of the environment variable attached to this
 * option.
 *
 * \sa get_environment_variable_name()
 * \sa get_environment_variable_value()
 */
void option_info::set_environment_variable_name(std::string const & name)
{
    f_environment_variable_name = name;
}


/** \brief Set the default value of this option.
 *
 * This function is an overload which allows us to call set_default()
 * with a nullptr.
 *
 * \param[in] default_value  The new default value for this option.
 *
 * \sa get_environment_variable_name()
 * \sa get_environment_variable_value()
 */
void option_info::set_environment_variable_name(char const * name)
{
    if(name != nullptr)
    {
        set_environment_variable_name(std::string(name));
    }
}


/**  \brief Retrieve the environment variable name of this option.
 *
 * Each command line option can be assigned an environment variable
 * name. When checking for the global environment variable name, the
 * advgetopt library also checks each option's environment variable
 * name which has priority over the global variable (i.e. it will
 * overwrite a value found in the global environment variable).
 *
 * The value returned is an empty string by default, which means the
 * option does not have a value defined in an environment variable.
 *
 * \return The environment variable name of this option.
 *
 * \sa set_environment_variable_name()
 * \sa get_environment_variable_value()
 */
std::string option_info::get_environment_variable_name() const
{
    return f_environment_variable_name;
}


/**  \brief Retrieve the environment variable value of this option.
 *
 * Each command line option can be assigned an environment variable
 * name. Using this name, this function attempts to retrieve the
 * corresponding value defined in that variable.
 *
 * \param[out] value  Save the resulting value in this variable.
 * \param[in] intro  The introducer to prepend. May be nullptr.
 *
 * \return true if a value was defined.
 *
 * \sa set_environment_variable_name()
 * \sa get_environment_variable_name()
 */
bool option_info::get_environment_variable_value(
          std::string & value
        , char const * intro) const
{
    // make it empty by default
    //
    value.clear();

    if(f_environment_variable_name.empty())
    {
        return false;
    }

    std::string name(f_environment_variable_name);
    if(intro != nullptr)
    {
        name = intro + name;
    }

    char const * env(getenv(name.c_str()));
    if(env == nullptr)
    {
        return false;
    }

    value = env;

    return true;
}


/** \brief Get the flags.
 *
 * The options have flags determining various sub-options available
 * to them. Right now we have flags to tell how each option can be
 * used (on the command line, in an environment variable, or in
 * a configuration file.)
 *
 * \note
 * We have the GETOPT_FLAG_ALIAS flag which is used to define
 * an alias. That means values do not get set in an option which
 * is marked as an alias. Instead, they get set in the option
 * which is being aliased. This means your software does not have
 * to check both options. The setup function will actually call
 * the set_alias() function at some point to finalize aliases
 * so you do not really need the flag, except to know that no
 * value will be defined here because it will instead be saved
 * in the aliased option.
 *
 * \param[in] flags  The new flags.
 */
void option_info::set_flags(flag_t flags)
{
    f_flags = flags;
}


/** \brief Make sure a given flag is set.
 *
 * This function adds the given flag from the set of flags being set.
 *
 * \note
 * Multiple flags can be defined in \p flag.
 *
 * \param[in] flag  The flag(s) to set.
 */
void option_info::add_flag(flag_t flag)
{
    set_flags(f_flags | flag);
}


/** \brief Make sure a given flag is not set.
 *
 * This function removes the given flag from the set of flags currently set.
 *
 * \note
 * Multiple flags can be defined in \p flag.
 *
 * \param[in] flag  The flag(s) to clear.
 */
void option_info::remove_flag(flag_t flag)
{
    set_flags(f_flags & ~flag);
}


/** \brief Retrieve the flags.
 *
 * This function retrieves all the flags defined in this option.
 *
 * To just check whether a flag is set or not, use the has_flag()
 * function instead.
 *
 * \return This option flags.
 */
flag_t option_info::get_flags() const
{
    return f_flags;
}


/** \brief Check whether a flag is set.
 *
 * This function is used to check whether a flag is set or not.
 *
 * \note
 * The \p flag parameter can be set to more than one flag in which case
 * the function returns true if any one of those flags is set.
 *
 * \return true if the flag is set, false otherwise.
 */
bool option_info::has_flag(flag_t flag) const
{
    return (f_flags & flag) != 0;
}


/** \brief Check whether this option has a default value.
 *
 * Whenever an option is given a default value, the GETOPT_FLAG_HAS_DEFAULT
 * flag gets set. This allows us to distinguish between an option with a
 * default which is the empty string and an option without a default.
 *
 * The set_default() forces the flag to be set.
 *
 * The remove_default() clears the flag.
 *
 * \return true if the flag is set, false otherwise.
 *
 * \sa set_default()
 * \sa remove_default()
 * \sa get_default()
 */
bool option_info::has_default() const
{
    return has_flag(GETOPT_FLAG_HAS_DEFAULT);
}


/** \brief Set the default value.
 *
 * This function sets the default value for this option.
 *
 * The default value is always defined as a string, but it can later be
 * converted to a different type using the option validator.
 *
 * Often, though, the default value is not compatible with the validator.
 * For example, you may have a parameter which is set to a percentage
 * from -100% to +100% and the default may be the string "off".
 *
 * \note
 * After calling this function, the option is viewed as having a default
 * even if that's the empty string.
 *
 * \param[in] default_value  The new default value for this option.
 *
 * \sa remove_default()
 * \sa has_default()
 * \sa get_default()
 */
void option_info::set_default(std::string const & default_value)
{
    f_default_value = default_value;
    add_flag(GETOPT_FLAG_HAS_DEFAULT);
}


/** \brief Set the default value of this option.
 *
 * This function is an overload which allows us to call set_default()
 * with a nullptr.
 *
 * \param[in] default_value  The new default value for this option.
 *
 * \sa remove_default()
 * \sa has_default()
 * \sa get_default()
 */
void option_info::set_default(char const * default_value)
{
    if(default_value != nullptr)
    {
        set_default(std::string(default_value));
    }
}


/** \brief Remove the default value.
 *
 * Call this function remove the default value. The default string gets
 * cleared and the GETOPT_FLAG_NO_DEFAULT flag gets set.
 *
 * \sa set_default()
 * \sa has_default()
 * \sa get_default()
 */
void option_info::remove_default()
{
    f_default_value.clear();
    remove_flag(GETOPT_FLAG_HAS_DEFAULT);
}


/** \brief Retrieve the default value.
 *
 * This function returns the default value.
 *
 * \return The default string value.
 */
std::string const & option_info::get_default() const
{
    return f_default_value;
}


/** \brief Set the help string for this option.
 *
 * The usage() function prints this string whenever the command
 * line includes the help command line option (such as `-h` or
 * `--help`.)
 *
 * The string can include various flags such as `%p` to include
 * dynamically defined parameters. See the process_help_string()
 * function for additional details about these parameters.
 *
 * \note
 * When using a special flag (i.e. GETOPT_FLAG_HELP), the help value
 * string is used as the value used by that special feature:
 *
 * \li GETOPT_FLAG_HELP
 *
 * It represents a string to print out by the usage() function. The option
 * purpose is solaly for adding a string of help in the output.
 *
 * \li GETOPT_FLAG_EXTERNAL_OPTIONS
 *
 * It represents the filename to read additional advgetopt options. In
 * some cases, your static array of option structures is to define this
 * special flag.
 *
 * \li GETOPT_FLAG_LICENSE
 *
 * It represents the program license.
 *
 * \li GETOPT_FLAG_COPYRIGHT
 *
 * It represents the program copyright notice.
 *
 * \param[in] help  The help string for this option.
 */
void option_info::set_help(std::string const & help)
{
    f_help = help;
}


/** \brief Set the help string for this option.
 *
 * This function is an overload which allows us to call set_help() with
 * a nullptr and not crash. We just ignore the call when that happens.
 *
 * \param[in] help  The help string for this option or nullptr.
 */
void option_info::set_help(char const * help)
{
    if(help != nullptr)
    {
        set_help(std::string(help));
    }
}


/** \brief Get the help string.
 *
 * This function returns the help string for this command line option.
 *
 * \warning
 * Note that when a special flag is set, this string may represent something
 * else that a help string.
 *
 * \return The help string of this argument.
 */
std::string const & option_info::get_help() const
{
    return f_help;
}


/** \brief Set the validator for this option.
 *
 * This function parses the specified name and optional parameters and
 * create a corresponding validator for this option.
 *
 * The \p name_and_params string can be defined as one of:
 *
 * \code
 *     <validator-name>
 *     <validator-name>()
 *     <validator-name>(<param1>)
 *     <validator-name>(<param1>, <param2>, ...)
 * \endcode
 *
 * The list of parameters is optional. There may be no, just one,
 * or any number of parameters. How the parameters are parsed is left
 * to the validator to decide.
 *
 * If the input string is empty, the current validator, if one is
 * installed, gets removed.
 *
 * \note
 * If the option_info already has a set of values, they get validated
 * against the new validator. Any value which does not validate gets
 * removed at once. The validation process also generates an error
 * when an invalid error is found. Note that it is expected that you
 * will setup a validator before you start parsing data so this feature
 * should seldom be used.
 *
 * \param[in] name_and_params  The validator name and parameters.
 *
 * \return true if the validator was installed and all existing values were
 *         considered valid.
 */
bool option_info::set_validator(std::string const & name_and_params)
{
    return set_validator(validator::create(name_and_params));
}


/** \brief Set the validator for this option.
 *
 * Options may be assigned a validator. Without a validator, any value
 * is considered valid.
 *
 * A value is checked when you call the validates() function. The function
 * returns true if the value is considered valid. False in all other cases.
 *
 * You can define your own validators and add them to the library list of
 * available validators before using the library in order to get your
 * options to use said validators.
 *
 * \note
 * If the option_info already has a set of values, they get validated
 * against the new validator. Any value which does not validate gets
 * removed at once. The validation process also generates an error
 * when an invalid error is found. Note that it is expected that you
 * will setup a validator before you start parsing data so this feature
 * should seldom be used.
 *
 * \param[in] validator  A pointer to a validator.
 *
 * \return true if the validator was installed and all existing values were
 *         considered valid.
 */
bool option_info::set_validator(validator::pointer_t validator)
{
    f_validator = validator;

    // make sure that all existing values validate against this
    // new validator
    //
    std::size_t const size(f_value.size());
    bool const r(validate_all_values());
    if(size != f_value.size())
    {
        value_changed(0);
    }
    return r;
}


/** \brief Clear the validator.
 *
 * This function removes the existing validator by resetting the pointer
 * back to nullptr.
 *
 * \param[in] null_ptr  Ignored.
 *
 * \return Always true since no validator means any existing values would
 *         be considered valid.
 */
bool option_info::set_validator(std::nullptr_t null_ptr)
{
    snapdev::NOT_USED(null_ptr);

    f_validator.reset();

    return true;
}


/** \brief Check a value validity.
 *
 * This function us used internally to verify values that get added at
 * the time they get added. It runs the validator::validate() function
 * and returns true if the value is considered valid. When the value
 * does not validate, it returns false and removes the value from the
 * f_value vector. This means no invalid values are ever kept in an
 * option_info object.
 *
 * An option without a validator has values that are always valid.
 * Also, an empty value is always considered valid.
 *
 * \note
 * This function is private since there is no need for the user of
 * the option_info to ever call it (i.e. it automatically gets called
 * any time a value gets added to the f_value vector.)
 *
 * \param[in] idx  The value to check.
 *
 * \return true if the value is considered valid, false otherwise.
 */
bool option_info::validates(int idx)
{
    if(static_cast<size_t>(idx) >= f_value.size())
    {
        throw getopt_undefined(                                         // LCOV_EXCL_LINE
                      "option_info::validates(): no value at index "    // LCOV_EXCL_LINE
                    + std::to_string(idx)                               // LCOV_EXCL_LINE
                    + " (idx >= "                                       // LCOV_EXCL_LINE
                    + std::to_string(f_value.size())                    // LCOV_EXCL_LINE
                    + ") for --"                                        // LCOV_EXCL_LINE
                    + f_name                                            // LCOV_EXCL_LINE
                    + " so you can't get this value.");                 // LCOV_EXCL_LINE
    }

    // the value is considered valid when:
    //   * there is no validator
    //   * if the value is empty
    //   * when the value validate against the specified validator
    //
    if(f_validator == nullptr
    || f_value[idx].empty()
    || f_validator->validate(f_value[idx]))
    {
        return true;
    }

    cppthread::log << cppthread::log_level_t::error
                   << "input \""
                   << f_value[idx]
                   << "\" given to parameter --"
                   << f_name
                   << " is not considered valid."
                   << cppthread::end;

    // get rid of that value since it does not validate
    //
    f_value.erase(f_value.begin() + idx);
    if(f_value.empty())
    {
        f_source = option_source_t::SOURCE_UNDEFINED;
    }

    return false;
}


/** \brief Retrieve a pointer to the validator.
 *
 * The validator of an option may be used for additional tasks such as
 * converting the value to a specific type (i.e. a string to an
 * integer, for example.)
 *
 * This function allows you to retrieve the validator to be able to
 * make use of those functions. You will have to use
 * std::dynamic_cast_pointer<>() to change the type of validator to
 * the specialized validator of this option. If that returns a null
 * pointer, then the option is not using that type of validator.
 *
 * \todo
 * Add a template function that does the cast for the caller.
 *
 * \return A pointer to this option validator.
 */
validator::pointer_t option_info::get_validator() const
{
    return f_validator;
}


/** \brief Set the alias option.
 *
 * After loading all the options, we run the link_aliases() function which
 * makes sure that all the options that are marked as an alias are
 * properly linked.
 *
 * \param[in] alias  The final destination of this option.
 */
void option_info::set_alias_destination(option_info::pointer_t destination)
{
    if(destination->has_flag(GETOPT_FLAG_ALIAS))
    {
        throw getopt_invalid(
                "option_info::set_alias(): you can't set an alias as"
                " an alias of another option.");
    }

    f_alias_destination = destination;
}


/** \brief Get a link to the destination alias.
 *
 * This function returns a pointer to the aliased option.
 *
 * \return The alias or a nullptr.
 */
option_info::pointer_t option_info::get_alias_destination() const
{
    return f_alias_destination;
}


/** \brief Set the list of separators.
 *
 * Options marked with the GETOPT_FLAG_MULTIPLE flag
 * get their value cut by separators when such is found in an
 * environment variable or a configuration file.
 *
 * This function saves the list of separators in a vector.
 *
 * \todo
 * At the moment, this is only applied when the parameter is specified with
 * the long option and an equal sign, as in: `--tags=a,b,c,d`. I thinks that
 * it should also work without the equal sign.
 *
 * \param[in] separators  The list of separators to be used for this argument.
 */
void option_info::set_multiple_separators(char const * const * separators)
{
    f_multiple_separators.clear();
    if(separators == nullptr)
    {
        return;
    }

    for(; *separators != nullptr; ++separators)
    {
        f_multiple_separators.push_back(*separators);
    }
}


/** \brief Set the list of separators.
 *
 * Options marked with the GETOPT_FLAG_MULTIPLE flag
 * get their value cut by separators when such is found in an
 * environment variable or a configuration file.
 *
 * This function saves the specified list of separators.
 *
 * \todo
 * See the other set_multiple_separators() function about the issue of
 * the separators not being used in all cases.
 *
 * \param[in] separators  The list of separators to be used for this argument.
 */
void option_info::set_multiple_separators(string_list_t const & separators)
{
    f_multiple_separators = separators;
}


/** \brief Retrieve the list of separators for this argument.
 *
 * This function returns a reference to the list of separators of this
 * option. It is expected to be used when a value is found in a
 * configuration file or a command line in an environment variable.
 * Parameters on the command line are already broken down by the
 * shell and we do not do any further manipulation with those.
 *
 * \warning
 * As mentioned in the set_multiple_separators() function, the separators
 * are only used when parsing a long parameter using the equal sign notation
 * (i.e. `--tags=a,b,c,d`). It also works in environment variables. I am
 * thinking that the `--tags a,b,c,d` should probably work the same way
 * though because otherwise many people will have a surprise.
 *
 * \return A reference to the list of separators used to cut multiple
 *         arguments found in a configuration file or an environment
 *         variable.
 */
string_list_t const & option_info::get_multiple_separators() const
{
    return f_multiple_separators;
}


/** \brief Assign variables to this option info.
 *
 * The getopt object holds a set of variables which is can pass down to
 * the option info. If defined, then the get_value() function returns
 * a processed value (a.k.a. the `${...}` references in that value are
 * replaced by their corresponding value).
 *
 * \param[in] vars  A pointer to a list of variables.
 */
void option_info::set_variables(variables::pointer_t vars)
{
    f_variables = vars;
}


/** \brief Retrieve the list of variables held by this option info.
 *
 * This option info object may replace variables in values (see get_value()
 * for details) using this list of variables. Option info objects created
 * by the getopt class always have this pointer set, although the list of
 * variables may be empty.
 *
 * \return A pointer to the list of variables found in the option info object.
 */
variables::pointer_t option_info::get_variables() const
{
    return f_variables;
}


/** \brief Check whether one of the values matches the input.
 *
 * This function searches the set of existing values in this option_info
 * and if found returns true.
 *
 * \note
 * It is possible to add the same value multiple times. However, there are
 * cases where you may not want to have the same value more than once.
 * This function can be used to try to not do that.
 *
 * \param[in] value  The value to search in this option.
 *
 * \return true if the value is already defined in this option_info.
 *
 * \sa set_value()
 */
bool option_info::has_value(std::string const & value) const
{
    auto const it(std::find(f_value.begin(), f_value.end(), value));
    return it != f_value.end();
}


/** \brief Add a value to this option.
 *
 * Whenever an option is found it may be followed by one or more values.
 * This function is used to add these values to this option.
 *
 * Later you can use the size() function to know how many values were
 * added and the get_value() to retrieve any one of these values.
 *
 * \warning
 * This function sets the value at offset 0 if it is already defined and
 * the GETOPT_FLAG_MULTIPLE flag is not set in this option. In other words,
 * you can't use this function to add multiple values if this option does
 * not support that feature.
 *
 * \param[in] value  The value to add to this option.
 * \param[in] option_keys  The set of keys found at the end of the option name.
 * \param[in] source  Where the value comes from.
 *
 * \return true when the value was accepted (no error occurred).
 *
 * \sa set_value()
 */
bool option_info::add_value(
      std::string const & value
    , string_list_t const & option_keys
    , option_source_t source)
{
    return set_value(
              has_flag(GETOPT_FLAG_MULTIPLE)
                    ? f_value.size()
                    : 0
            , value
            , option_keys
            , source);
}


/** \brief Replace a value.
 *
 * This function is generally used to replace an existing value. If the
 * index is set to the size of the existing set of values, then a new
 * value is saved in the vector.
 *
 * This is particularly useful if you want to edit a configuration file.
 *
 * If the option comes with a validator, then the value gets checked
 * against that validator. If that results in an error, the value is
 * not added to the vector so an invalid value will never be returned
 * by the option_info class.
 *
 * The value does not get added when it currently is locked or when
 * it does not validate as per the validator of this option_info.
 *
 * \exception getopt_exception_undefined
 * If the index is out of range (too large or negative), then this
 * exception is raised.
 *
 * \param[in] idx  The position of the value to update.
 * \param[in] value  The new value.
 * \param[in] source  Where the value comes from.
 *
 * \return true if the set_value() added the value.
 *
 * \sa add_value()
 * \sa validates()
 * \sa lock()
 * \sa unlock()
 */
bool option_info::set_value(
      int idx
    , std::string const & value
    , string_list_t const & option_keys
    , option_source_t source)
{
    if(source == option_source_t::SOURCE_UNDEFINED)
    {
        throw getopt_logic_error(
                  "option_info::set_value(): called with SOURCE_UNDEFINED ("
                + std::to_string(static_cast<int>(source))
                + ").");
    }

    if(has_flag(GETOPT_FLAG_LOCK))
    {
        return false;
    }

    if(source == option_source_t::SOURCE_DIRECT
    && !has_flag(GETOPT_FLAG_DYNAMIC_CONFIGURATION))
    {
        cppthread::log << cppthread::log_level_t::error
                       << "option \"--"
                       << f_name
                       << "\" can't be directly updated."
                       << cppthread::end;
        return false;
    }

    bool const multiple(has_flag(GETOPT_FLAG_MULTIPLE));
    if(multiple)
    {
        if(static_cast<size_t>(idx) > f_value.size())
        {
            throw getopt_logic_error(
                      "option_info::set_value(): no value at index "
                    + std::to_string(idx)
                    + " and it is not the last available index + 1 (idx > "
                    + std::to_string(f_value.size())
                    + ") so you can't set this value (try add_value() maybe?).");
        }
    }
    else
    {
        if(idx != 0)
        {
            throw getopt_logic_error(
                          "option_info::set_value(): single value option \"--"
                        + f_name
                        + "\" does not accepts index "
                        + std::to_string(idx)
                        + " which is not 0.");
        }
    }

    f_source = source;
    f_integer.clear();
    f_double.clear();

    bool r(true);
    if(option_keys.empty())
    {
        if(static_cast<size_t>(idx) == f_value.size())
        {
            f_value.push_back(value);
        }
        else
        {
            if(f_value[idx] == value)
            {
                // no change, we can return as is
                //
                // note: we know that the value is valid here since the
                //       validates() function removes invalid values from
                //       the f_value array
                //
                return true;
            }
            f_value[idx] = value;
        }

        if(validates(idx))
        {
            value_changed(idx);
        }
        else
        {
            r = false;
        }
    }
    else
    {
        bool changed(false);
        bool const append(multiple && idx >= static_cast<int>(f_value.size()));
        for(auto const & k : option_keys)
        {
            bool new_value(true);
            std::string const v(k + value);
            idx = append ? -1 : find_value_index_by_key(k);
            if(idx == -1)
            {
                f_value.push_back(v);
                changed = true;
            }
            else
            {
                if(f_value[idx] == v)
                {
                    new_value = false;
                }
                else
                {
                    changed = true;
                    f_value[idx] = v;
                }
            }
            if(new_value)
            {
                if(validates(idx))
                {
                    value_changed(idx);
                }
                else
                {
                    r = false;
                }
            }
        }
        if(!changed)
        {
            return true;
        }
    }

    return r;
}


/** \brief Set a multi-value at once.
 *
 * This function views the \p value parameter as a multi-value parameter
 * which it breaks down in multiple parameters and add the results to this
 * option_info object as the current value(s).
 *
 * To separate the values, the function makes use of the separators as
 * set by one the set_multiple_separators() functions.
 *
 * The resulting values must not be the empty string. Empty strings are
 * ignored. So if the separator is a comma and you write:
 *
 * \code
 *     foo,,,bar
 * \endcode
 *
 * The result includes "foo" and "bar" and no empty strings.
 *
 * The value can be quoted in which case it can include any of the
 * separators.
 *
 * \code
 *     "foo, bar",'bar, foo'
 * \endcode
 *
 * The result includes "foo, bar" and "bar, foo" (without the quotes
 * which get removed in the process).
 *
 * \note
 * The function has the side effect of clearing any existing parameters
 * first. So only the newly defined parameters in \p value will be set
 * in the option once the function returns.
 *
 * \param[in] value  The multi-value to save in this option.
 * \param[in] option_keys  The keys to which the values are applied.
 * \param[in] source  Where the value comes from.
 *
 * \return true if all the values in \p value were considered valid.
 *
 * \sa add_value()
 * \sa set_value()
 */
bool option_info::set_multiple_values(
      std::string const & value
    , string_list_t const & option_keys
    , option_source_t source)
{
    if(!has_flag(GETOPT_FLAG_ARRAY)
    && option_keys.size() != 0)
    {
        throw getopt_logic_error(
                 "option_info::set_multiple_value(): parameter --"
               + f_name
               + " does not support array keys.");
    }

    if(source == option_source_t::SOURCE_UNDEFINED)
    {
        throw getopt_logic_error(
                  "option_info::set_multiple_values(): called with SOURCE_UNDEFINED ("
                + std::to_string(static_cast<int>(source))
                + ").");
    }

    string_list_t result;
    split_string(unquote(value, "[]"), result, f_multiple_separators);

    if(!has_flag(GETOPT_FLAG_MULTIPLE)
    && result.size() > 1)
    {
        throw getopt_logic_error(
                 "option_info::set_multiple_value(): parameter --"
               + f_name
               + " expects zero or one parameter. The set_multiple_value() function should not be called with parameters that only accept one value.");
    }

    if(!option_keys.empty())
    {
        string_list_t keyed_result;
        for(auto const & k : option_keys)
        {
            for(auto & r : result)
            {
                // note: the keys are expected to already include the ending ':'
                //
                keyed_result.push_back(k + r);
            }
        }
        result.swap(keyed_result);
    }

    f_source = source;
    f_value.swap(result);
    f_integer.clear();
    f_double.clear();

    bool const r(validate_all_values());

    if(f_value != result)
    {
        // TBD: should we not call this function with all instances?
        //      i.e. for(int idx(0); idx < f_value.size(); ++idx) ...
        //      and check each value in f_value with the old value in
        //      the result variable (knowing that result may be smaller)
        //
        value_changed(0);
    }

    return r;
}


/** \brief Validate all the values of this option_info object.
 *
 * Whenever you change the validator of an option_info, or change
 * all the values with set_multiple_value(), all the values get
 * verified using this function. The function removes any value
 * which does not validate according to the current validator.
 *
 * \note
 * Keep in mind that an empty value is always considered valid,
 * no matter what the validator is. This is because when you
 * use an option without a value (i.e. `--order` instead of
 * `--order asc`) the value is set to the empty string unless
 * there is a default. This allows you to know that the
 * option was used without a value, which is useful for some
 * options.
 *
 * \return true if all the values were considered valid.
 */
bool option_info::validate_all_values()
{
    bool all_valid(true);
    if(f_validator != nullptr)
    {
        for(size_t idx(0); idx < f_value.size(); )
        {
            if(!validates(idx))
            {
                // the value was removed, so do not increment `idx`
                //
                all_valid = false;
            }
            else
            {
                ++idx;
            }
        }
    }

    return all_valid;
}


/** \brief Check whether a value is defined.
 *
 * When parsing the options on the command line or a configuration file,
 * values get added to the various existing option_info. If a special
 * "*" option is also defined, then any value found on the command line
 * or the configuration file are returned.
 *
 * To know whether this or that option was defined with a value, use
 * this function. Even an option which doesn't come with a parameter
 * will get an is_defined() returning true once it was found on the
 * command line. The value will be the empty string.
 *
 * \return true if that option was found on the command line, in the
 *         environment variable, or in the configuration file.
 */
bool option_info::is_defined() const
{
    return !f_value.empty();
}


/** \brief Return the source of this option info.
 *
 * This function returns the source of this option, i.e. whether it came
 * from the command line, the environment variable, a configuration file,
 * or some other source that you can define.
 *
 * The source is similar to a priority in the sense that a source with a
 * higher number cannot overwrite the value of a smaller source. The source
 * is set at the same time as you set the option. The mechanism may not be
 * working exactly as expected when trying to add options from different
 * sources.
 *
 * \note
 * In the old version, the value would be the value set with the last
 * set_value() command. That worked because we did not try to support
 * fully dynamic options. Now we want to have the ability to set an
 * option on the command line and that has to prevent the set from
 * a dynamic source. Since the dynamic source would do the set_value()
 * at a later time, just the order is not enough to know whether the
 * dynamic source has permission to overwrite that value.
 *
 * \return The source of the option info.
 */
option_source_t option_info::source() const
{
    return f_source;
}


/** \brief Whether the sources should be traced.
 *
 * This is a global flag that you can set before calling any getopt functions
 * so that way you can make sure that you get a full trace of all the
 * sources for all your options. Then you can use the --show-sources
 * command line options to see the resulting data.
 *
 * \note
 * This option is costly since it saves a lot of data, which is why we have
 * it as an option. If the getopt() function detects in the argv passed to
 * it a "--show-sources" option, then it will automatically call this
 * function with true, even before it starts parsing anything. The flag is
 * false by default.
 *
 * \param[in] trace  Whether the sources should be traced.
 */
void option_info::set_trace_sources(bool trace)
{
    g_trace_sources = trace;
}


/** \brief Get the trace of this option.
 *
 * An option can be marked for tracing. This allows you to see exactly
 * which value came from which source. We currently support multiple
 * sources such as the command line, environment variable, direct,
 * dynamic, configuration files.
 *
 * \return An array of strings representing the source of each value
 * in the order they were set in this option_info.
 */
string_list_t const & option_info::trace_sources() const
{
    return f_trace_sources;
}


/** \brief Save the filename of the current configuration file.
 *
 * While parsing a configuration file, this function gets called to
 * set the name which is used to generate the trace of the source
 * of all the configuration data.
 */
void option_info::set_configuration_filename(std::string const & filename)
{
    g_configuration_filename = filename;
}


/** \brief Retrieve the number of values defined for this option.
 *
 * This function returns the number of values that were found for this
 * option.
 *
 * If the option is marked as GETOPT_FLAG_MULTIPLE, then this function
 * may return 0 or more. Without that flag, this function only returns
 * 0 or 1.
 *
 * You must use the size() parameter to know how many items are defined
 * and call the get_value() with a correct `idx` parameter (i.e. a value
 * between 0 and `size() - 1`.)
 *
 * \return The number of values defined in this option.
 */
size_t option_info::size() const
{
    return f_value.size();
}


/** \brief Retrieve the value.
 *
 * This function returns the value for this option. By default, set the
 * \p idx parameter to zero.
 *
 * The number of values is defined by the size() function.
 *
 * The is_defined() function returns true if at least one value is defined.
 * It is a good idea to check first otherwise you will get an exception.
 *
 * If the parameter is marked as one that can be processed through the
 * variables::process_value() function and the variables were defined
 * with set_variables(), then the value will be processed for variables
 * unless you set the \p raw parameter to true.
 *
 * \exception getopt_exception_undefined
 * If the \p idx parameter is too large or no value was found for this
 * option, then this function raises an invalid error.
 *
 * \param[in] idx  The index of the parameter to retrieve.
 * \param[in] raw  Whether to allow the variable processing or not.
 *
 * \return The value at \p idx.
 */
std::string option_info::get_value(int idx, bool raw) const
{
    if(static_cast<size_t>(idx) >= f_value.size())
    {
        throw getopt_undefined(
                      "option_info::get_value(): no value at index "
                    + std::to_string(idx)
                    + " (idx >= "
                    + std::to_string(f_value.size())
                    + ") for --"
                    + f_name
                    + " so you can't get this value.");
    }

    if(!raw
    && f_variables != nullptr
    && has_flag(GETOPT_FLAG_PROCESS_VARIABLES))
    {
        return f_variables->process_value(f_value[idx]);
    }
    else
    {
        return f_value[idx];
    }
}


/** \brief Get the index at which a value with the given key is defined.
 *
 * This function searches a value with the specified \p key and return
 * the index where it was found.
 *
 * If the function does not find a value starting with \p key, then it
 * returns -1.
 *
 * The function can be called with the start \p idx set to a value other
 * than zero in which case the search starts at that index. It is valid
 * to call the function with \p idx larger or equal to the number of
 * values defined.
 *
 * \exception getopt_logic_error
 * If the \p idx parameter is negative, this exception is raised.
 *
 * \exception getopt_undefined
 * If no values were defined, this exception is raised. To avoid receiving
 * the exception, make sure to first test with is_defined(). Note that if
 * the requested key is not found, the function simply returns an empty
 * string opposed to raising an exception.
 *
 * \param[in] key  The key to search for.
 * \param[in] idx  Start from this index.
 *
 * \return The index at which that value is defined or -1 when not found.
 */
int option_info::find_value_index_by_key(std::string key, int idx) const
{
    if(idx < 0)
    {
        throw getopt_logic_error("idx cannot be negative in find_value_index_by_key()");
    }
    if(f_value.empty())
    {
        throw getopt_undefined(
                      "option_info::find_value_index_by_key(): --"
                    + f_name
                    + " has no values defined.");
    }

    if(key.back() != ':')
    {
        key += ':';
    }
    int const max(f_value.size());
    for(; idx < max; ++idx)
    {
        if(f_value[idx].rfind(key, 0) == 0)
        {
            return idx;
        }
    }

    return -1;
}


/** \brief Get the value as a long.
 *
 * This function returns the value converted to a `long`.
 *
 * If the value does not represent a valid long value, an error is
 * emitted through the logger.
 *
 * The value will be parsed through the variables if defined and this
 * parameter allows it. This means the value may be a variable reference
 * instead of an actually value (i.e. `${one}`)
 *
 * \note
 * The function will transform all the values in case this is a
 * GETOPT_FLAG_MULTIPLE option and cache the results.
 * Calling the function many times with the same index is very fast
 * after the first time.
 *
 * \exception getopt_exception_undefined
 * If the value was not defined, the function raises this exception.
 *
 * \param[in] idx  The index of the value to retrieve as a long.
 *
 * \return The value at \p idx converted to a long or -1 on error.
 */
long option_info::get_long(int idx) const
{
    if(static_cast<size_t>(idx) >= f_value.size())
    {
        throw getopt_undefined(
                      "option_info::get_long(): no value at index "
                    + std::to_string(idx)
                    + " (idx >= "
                    + std::to_string(f_value.size())
                    + ") for --"
                    + f_name
                    + " so you can't get this value.");
    }

    // since we may change the f_integer vector between threads,
    // add protection (i.e. most everything else is created at the
    // beginning so in the main thread)
    //
    cppthread::guard lock(get_global_mutex());

    if(f_integer.size() != f_value.size())
    {
        // we did not yet convert to integers do that now
        //
        size_t const max(f_value.size());
        for(size_t i(f_integer.size()); i < max; ++i)
        {
            std::int64_t v;
            if(!validator_integer::convert_string(get_value(i), v))
            {
                f_integer.clear();

                cppthread::log << cppthread::log_level_t::error
                               << "invalid number ("
                               << f_value[i]
                               << ") in parameter --"
                               << f_name
                               << " at offset "
                               << i
                               << "."
                               << cppthread::end;
                return -1;
            }
            f_integer.push_back(v);
        }
    }

    return f_integer[idx];
}


/** \brief Get the value as a double.
 *
 * This function returns the value converted to a `double`.
 *
 * If the value does not represent a valid double value, an error is
 * emitted through the logger.
 *
 * The value will be parsed through the variables if defined and this
 * parameter allows it. This means the value may be a variable reference
 * instead of an actually value (i.e. `${pi}`)
 *
 * \note
 * The function will transform all the values in case this is a
 * GETOPT_FLAG_MULTIPLE option and cache the results.
 * Calling the function many times with the same index is very fast
 * after the first time.
 *
 * \exception getopt_exception_undefined
 * If the value was not defined, the function raises this exception.
 *
 * \param[in] idx  The index of the value to retrieve as a double.
 *
 * \return The value at \p idx converted to a double or -1.0 on error.
 */
double option_info::get_double(int idx) const
{
    if(static_cast<size_t>(idx) >= f_value.size())
    {
        throw getopt_undefined(
                      "option_info::get_double(): no value at index "
                    + std::to_string(idx)
                    + " (idx >= "
                    + std::to_string(f_value.size())
                    + ") for --"
                    + f_name
                    + " so you can't get this value.");
    }

    // since we may change the f_integer vector between threads,
    // add protection (i.e. most everything else is created at the
    // beginning so in the main thread)
    //
    cppthread::guard lock(get_global_mutex());

    if(f_double.size() != f_value.size())
    {
        // we did not yet convert to doubles do that now
        //
        size_t const max(f_value.size());
        for(size_t i(f_double.size()); i < max; ++i)
        {
            double v;
            if(!validator_double::convert_string(get_value(i), v))
            {
                f_double.clear();

                cppthread::log << cppthread::log_level_t::error
                               << "invalid number ("
                               << f_value[i]
                               << ") in parameter --"
                               << f_name
                               << " at offset "
                               << i
                               << "."
                               << cppthread::end;
                return -1;
            }
            f_double.push_back(v);
        }
    }

    return f_double[idx];
}


/** \brief Lock this value.
 *
 * This function allows for locking a value so further reading of data
 * from different sources will not overwrite it.
 *
 * When parsing the data we have multiple levels. Here are these levels
 * in priority order (first option found is the one we keep):
 *
 * \li Command line options
 * \li Environment Variables
 * \li Configuration File: Local (`./\<name>.conf`)
 * \li Configuration File: User's (`~/.config/\<proc>/\<name>.conf`)
 * \li Configuration File: Project sub-folder (`/etc/\<proc>/\<proc>.d/\<ohter-name>.conf`)
 * \li Configuration File: Project folder (`/etc/\<proc>/\<other-name>.conf`)
 * \li Configuration File: System sub-folder (`/etc/\<proc>/\<name>.conf`)
 * \li Configuration File: System folder (`/etc/\<proc>/\<name>.conf`)
 *
 * \note
 * Most of our packages do not have a Project and a System set of
 * configuration files. Often they will have just the System files.
 *
 * We use this lock because we want to support multiple values so just
 * detecting that a value is set to not add more options is not a good
 * test. Instead we lock the values that are set before moving to the
 * next level.
 *
 * \param[in] always  Always lock that option, whether it is defined or not.
 */
void option_info::lock(bool always)
{
    if(!always)
    {
        if(!is_defined())
        {
            return;
        }
    }

    add_flag(GETOPT_FLAG_LOCK);
}


/** \brief Unlock this value.
 *
 * This function does the opposite of the lock() function. It allows for
 * the value to be updated again.
 *
 * Once the getpot object is done parsing all the input, it unlocks all
 * the values using this function. The unlock is always unconditional.
 */
void option_info::unlock()
{
    remove_flag(GETOPT_FLAG_LOCK);
}


/** \brief Reset this value.
 *
 * This function clears the value so it is marked as undefined again.
 *
 * To reuse the same getopt object multiple times, you can use the
 * reset() function which clears the values. Then you can parse a
 * new set of argc/argv parameters.
 */
void option_info::reset()
{
    if(is_defined())
    {
        f_source = option_source_t::SOURCE_UNDEFINED;
        f_value.clear();
        f_integer.clear();
        f_double.clear();

        value_changed(0);
    }
}


/** \brief Add a callback to call on a change to this value.
 *
 * Since we now officially support dynamically setting option values, we
 * decided to add a callback mechanism that lets you know that an option
 * changed. That way you can react to the change as soon as possible instead
 * of having to poll for the value once in a while.
 *
 * \param[in] c  The callback. Usually an std::bind() call.
 *
 * \return The new callback identifier.
 */
option_info::callback_id_t option_info::add_callback(callback_t const & c)
{
    cppthread::guard lock(get_global_mutex());

    ++f_next_callback_id;
    f_callbacks.emplace_back(f_next_callback_id, c);
    return f_next_callback_id;
}


/** \brief Remove a callback.
 *
 * This function is the opposite of the add_callback(). It removes a callback
 * that you previously added. This is useful if you are interested in hearing
 * about the value when set but are not interested at all about future
 * changes.
 *
 * \param[in] id  The id returned by the add_callback() function.
 */
void option_info::remove_callback(callback_id_t id)
{
    cppthread::guard lock(get_global_mutex());

    auto it(std::find_if(
              f_callbacks.begin()
            , f_callbacks.end()
            , [id](auto e)
            {
                return e.f_id == id;
            }));
    if(it != f_callbacks.end())
    {
        f_callbacks.erase(it);
    }
}


/** \brief Call whenever the value changed so we can handle callbacks.
 *
 * This function is called on a change of the internal values.
 *
 * The function is used to call the callbacks that were added to this
 * option_info object. The function first copies the existing list of
 * callbacks so you can safely update the list from within a callback.
 *
 * \warning
 * Destroying your advgetopt::getopt option is not safe while a callback
 * is running.
 *
 * \param[in] idx  This represents the index of the value that last changed
 * (currently poor attempt to fix this issue).
 */
void option_info::value_changed(int idx)
{
    trace_source(idx);

    callback_vector_t callbacks;
    callbacks.reserve(f_callbacks.size());

    {
        cppthread::guard lock(get_global_mutex());
        callbacks = f_callbacks;
    }

    for(auto e : callbacks)
    {
        e.f_callback(*this);
    }
}



/** \brief Remember the source information at of this last change.
 *
 * The getopt class supports a flag which turns on the trace mode. This
 * allows it to memorize where the values came fram. This includes the
 * source and if the source is a configuration file, the path to that
 * configuration file.
 */
void option_info::trace_source(int idx)
{
    if(!g_trace_sources)
    {
        return;
    }

    std::string s;
    switch(f_source)
    {
    case option_source_t::SOURCE_COMMAND_LINE:
        s = "command-line";
        break;

    case option_source_t::SOURCE_CONFIGURATION:
        s = "configuration=\"" + g_configuration_filename + "\"";
        break;

    case option_source_t::SOURCE_DIRECT:
        s = "direct";
        break;

    case option_source_t::SOURCE_DYNAMIC:
        s = "dynamic";
        break;

    case option_source_t::SOURCE_ENVIRONMENT_VARIABLE:
        s = "environment-variable";
        break;

    case option_source_t::SOURCE_UNDEFINED:
        // this happens on a reset or all the values were invalid
        //
        f_trace_sources.push_back(f_name + " [*undefined-source*]");
        return;

    }

    if(f_value.empty())
    {
        // this should never ever happen
        // (if f_value is empty then f_source == SOURCE_UNDEFINED)
        //
        f_trace_sources.push_back(f_name + " [*undefined-value*]");     // LCOV_EXCL_LINE
    }
    else
    {
        // TODO: change the algorithm, if the option supports
        //
        if(!has_flag(GETOPT_FLAG_MULTIPLE)
        || static_cast<std::size_t>(idx) >= f_value.size())
        {
            f_trace_sources.push_back(f_name + "=" + f_value[0] + " [" + s + "]");
        }
        else
        {
            f_trace_sources.push_back(f_name + "[" + std::to_string(idx) + "]=" + f_value[idx] + " [" + s + "]");
        }
    }
}


}   // namespace advgetopt
// vim: ts=4 sw=4 et
