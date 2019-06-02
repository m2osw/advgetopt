/*
 * File:
 *    advgetopt/option_info.cpp -- a replacement to the Unix getopt() implementation
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


/** \file
 * \brief Implementation of the option_info class.
 *
 * This is the implementation of the class used to define one command
 * line option.
 */

// self
//
#include "advgetopt/option_info.h"

// advgetopt lib
//
#include "advgetopt/exception.h"
#include "advgetopt/log.h"

// boost lib
//
#include <boost/algorithm/string/replace.hpp>

// C++ lib
//
//#include    <limits>


namespace advgetopt
{




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
 * \exception getopt_exception_invalid
 * The constructor raises the invalid exception if the long name is an
 * empty string since this is not allowed.
 *
 * \param[in] name  The (long) name of this option.
 * \param[in] short_name  The short name of this option (one character.)
 */
option_info::option_info(std::string const & name, short_name_t short_name)
    : f_name(boost::replace_all_copy(name, "_", "-"))
    , f_short_name(short_name)
{
    if(name.empty())
    {
        throw getopt_exception_invalid(
                      std::string("option_info::option_info(): all options must at least have a long name (short name: '")
                    + static_cast<char>(short_name) // TODO: convert a UTF-8
                    + "'.");
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
 * \param[in] flag  The flag(s) to set.
 */
void option_info::add_flag(flag_t flag)
{
    f_flags |= flag;
}


/** \brief Make sure a given flag is not set.
 *
 * This function removes the given flag from the set of flags being set.
 *
 * \param[in] flag  The flag(s) to clear.
 */
void option_info::remove_flag(flag_t flag)
{
    f_flags &= ~flag;
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
 */
bool option_info::has_default() const
{
    return (f_flags & GETOPT_FLAG_HAS_DEFAULT) != 0;
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
 * \param[in] validator  A pointer to a validator.
 */
void option_info::set_validator(validator::pointer_t validator)
{
    f_validator = validator;
}


/** \brief Check a value validity.
 *
 * This function runs the validator::validate() function and returns true
 * if the value is considered valdi.
 *
 * By default, you probably want to set the \p idx parameter to 0. If
 * your option can accept multiple parameters, then larger indexes may
 * be used to check the additional values.
 *
 * An option without a validator has values that are always valid.
 *
 * \param[in] idx  The value to check.
 *
 * \return true if the value is considered valid, false otherwise.
 */
bool option_info::validates(int idx) const
{
    return f_validator == nullptr
                    ? true
                    : f_validator->validate(f_value[idx]);
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
 * \return A pionter to this option validator.
 */
validator::pointer_t option_info::get_validator() const
{
    return f_validator;
}


/** \brief Set the minimum expected value.
 *
 * This function is used to set the minimum value of this parameter.
 *
 * Note that the minimum is not taken in account for the default value.
 *
 * \param[in] value  The minimum value of this option.
 */
void option_info::set_minimum(std::string const & value)
{
    f_minimum_value = value;
}


/** \brief Set the maximum expected value.
 *
 * This function is used to set the maximum value of this parameter.
 *
 * Note that the maximum is not taken in account for the default value.
 *
 * \param[in] value  The maximum value of this option.
 */
void option_info::set_maximum(std::string const & value)
{
    f_maximum_value = value;
}


/** \brief Define a range of acceptable values.
 *
 * This function saves the minimum and maximum values that this option
 * can accept.
 *
 * Until set to a non-empty string, these parameters are considered
 * undefined and thus the boundary won't be checked.
 *
 * \note
 * The default value does not need to be in the range to be considered
 * valid as the default value.
 *
 * \param[in] min  The minimum value acceptable.
 * \param[in] max  The maximum value acceptable.
 */
void option_info::set_range(std::string const & min, std::string const & max)
{
    f_minimum_value = min;
    f_maximum_value = max;
}


/** \brief Retrieve the minimum value.
 *
 * This function returns the minimum acceptable value for this command
 * line option.
 *
 * \return The minimum value acceptable.
 */
std::string const & option_info::get_min() const
{
    return f_minimum_value;
}


/** \brief Retrieve the maximum value.
 *
 * This function returns the maximum acceptable value for this command
 * line option.
 *
 * \return The maximum value acceptable.
 */
std::string const & option_info::get_max() const
{
    return f_maximum_value;
}


/** \brief Add an acceptable child option.
 *
 * Our system supports a tree like heiarchy of options like one can
 * defined with formats such as Yaml, XML, JSON and .ini (two levels
 * only in that case.)
 *
 * Defining sub-options allows you to define a tree like set of
 * acceptable options.
 *
 * The root is an option_info object which has children which are the
 * first level in your command line and configuration files.
 *
 * \note
 * At this time we do not support more than one level on the command
 * line and in environment variables.
 *
 * \param[in] child  An option to add as a child of this option.
 */
void option_info::add_child(option_info::pointer_t child)
{
    if(child != nullptr)
    {
        f_children_by_long_name[child->get_name()] = child;
        short_name_t c(child->get_short_name());
        if(c != NO_SHORT_NAME)
        {
            f_children_by_short_name[c] = child;
        }
    }
}


/** \brief Get a map of this option's children.
 *
 * This function returns a map of this option's children. It is useful
 * to list the options, for example when the usage() function gets called.
 *
 * The map returned is indexed by the long names. We do not give access
 * to the short name map at this point.
 *
 * \return A reference to the map of children options.
 */
option_info::map_by_name_t const & option_info::get_children() const
{
    return f_children_by_long_name;
}


/** \brief Get a map of this option's children.
 *
 * This function returns a map of this option's children. It is useful
 * to list the options, for example when the usage() function gets called.
 *
 * \param[in] name  The long name of the child to retrieve.
 *
 * \return A reference to the map of children options.
 */
option_info::pointer_t option_info::get_child(std::string const & name) const
{
    auto it(f_children_by_long_name.find(name));
    if(it == f_children_by_long_name.end())
    {
        return option_info::pointer_t();
    }

    return it->second;
}


/** \brief Get a child by name.
 *
 * This function returns a child looking for it by its short name.
 *
 * If no such child is found, the function returns a null pointer.
 *
 * \param[in] short_name  The short name of the child to retrieve.
 *
 * \return A smart pointer to the child.
 */
option_info::pointer_t option_info::get_child(short_name_t short_name) const
{
    auto it(f_children_by_short_name.find(short_name));
    if(it == f_children_by_short_name.end())
    {
        return option_info::pointer_t();
    }

    return it->second;
}


/** \brief Set the alias option.
 *
 * After loading all the options, we run the link_aliases() function which
 * makes sure that all the options that are marked as an alias are
 * properly linked.
 *
 * \param[in] alias  The final destination of this option.
 */
void option_info::set_alias(option_info::pointer_t alias)
{
    if(alias->has_flag(GETOPT_FLAG_ALIAS))
    {
        throw getopt_exception_undefined(
                "option_info::set_alias(): you can't set an alias as"
                " an alias of another option.");
    }

    f_alias = alias;
}


/** \brief Get a link to the destination alias.
 *
 * This function returns a pointer to the aliased option.
 *
 * \return The alias or a nullptr.
 */
option_info::pointer_t option_info::get_alias() const
{
    return f_alias;
}


/** \brief Set the list of separators.
 *
 * Options marked with the GETOPT_FLAG_CONFIGURATION_MULTIPLE flag
 * get their value cut by separators when such is found in an
 * environment variable or a configuration file.
 *
 * This function saves the list of separators in a vector.
 *
 * \param[in] separators  The list of separators to be used for this argument.
 */
void option_info::set_multiple_separators(char const ** separators)
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
 * Options marked with the GETOPT_FLAG_CONFIGURATION_MULTIPLE flag
 * get their value cut by separators when such is found in an
 * environment variable or a configuration file.
 *
 * This function saves the specified list of separators.
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
 * \return A reference to the list of separators used to cut multiple
 *         arguments found in a configuration file or an environment
 *         variable.
 */
string_list_t const & option_info::get_multiple_separators() const
{
    return f_multiple_separators;
}


/** \brief Add a value to this option.
 *
 * Whenever an option is found it may be followed by one or more value.
 * This function is used to add these values to this option.
 *
 * Later you can use the size() function to know how many values were
 * added and the get_value() to retrieve the value.
 *
 * \warning
 * This function sets the value at offset 0 if it is already defined and
 * the GETOPT_FLAG_MULTIPLE flag is not set in this option. In other words,
 * you can't use this function to add multiple values if this option does
 * not support that feature.
 *
 * \param[in] value  The value to add to this option.
 */
void option_info::add_value(std::string const & value)
{
    if(has_flag(GETOPT_FLAG_LOCK))
    {
        return;
    }

    if((f_flags & GETOPT_FLAG_MULTIPLE) == 0)
    {
        // always replace the existing value,
        // we can't have more than one
        //
        if(f_value.empty())
        {
            f_value.push_back(value);
        }
        else
        {
            f_value[0] = value;
            f_integer.clear();
        }
    }
    else
    {
        f_value.push_back(value);
    }
}


/** \brief Replace a value.
 *
 * This function is generally used to replace an existing value. If the
 * index is set to the size of the existing set of values, then a new
 * value is saved in the table.
 *
 * This is particularly useful if you want to edit a configuration file.
 *
 * \exception getopt_exception_undefined
 * If the index is out of range (too large or negative), then this
 * exception is raised.
 *
 * \param[in] idx  The position of the value to update.
 * \param[in] value  The new value.
 */
void option_info::set_value(int idx, std::string const & value)
{
    if(has_flag(GETOPT_FLAG_LOCK))
    {
        return;
    }

    if(static_cast<size_t>(idx) > f_value.size())
    {
        throw getopt_exception_undefined(
                      "option_info::set_value(): no value at index "
                    + std::to_string(idx)
                    + " and it is not the last available index + 1 (idx > "
                    + std::to_string(f_value.size())
                    + ") so you can't set this value (try add_value() maybe?).");
    }

    if(static_cast<size_t>(idx) == f_value.size())
    {
        f_value.push_back(value);
    }
    else
    {
        f_value[idx] = value;
    }
    f_integer.clear();
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
 * \note
 * The function has the side effect of clearing any existing parameters
 * first. So only the newly defined parameters in \p value will be set
 * in the option once the function returns.
 *
 * \todo
 * Add support for quoted values
 *
 * \param[in] value  The multi-value to save in this option.
 */
void option_info::set_multiple_value(std::string const & value)
{
    f_value.clear();

    split_string(value, f_value, f_multiple_separators);
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
 * \exception getopt_exception_undefined
 * If the \p idx parameter is too large or no value was found for this
 * option, then this function raises an invalid error.
 *
 * \return The value.
 */
std::string const & option_info::get_value(int idx) const
{
    if(static_cast<size_t>(idx) >= f_value.size())
    {
        throw getopt_exception_undefined(
                      "option_info::get_value(): no value at index "
                    + std::to_string(idx)
                    + " (idx >= "
                    + std::to_string(f_value.size())
                    + ") so you can't get this value.");
    }

    return f_value[idx];
}


/** \brief Get the value as a long.
 *
 * \param[in] idx  The index of the value to retrieve as a long.
 *
 * \return The value at \p idx converted to a long.
 */
long option_info::get_long(int idx) const
{
    if(static_cast<size_t>(idx) >= f_value.size())
    {
        throw getopt_exception_undefined(
                      "option_info::get_integer(): no value at index "
                    + std::to_string(idx)
                    + " (idx >= "
                    + std::to_string(f_value.size())
                    + ") so you can't get this value.");
    }

    if(f_integer.size() != f_value.size())
    {
        // we did not yet convert to integers do that now
        //
        for(auto const & str : f_value)
        {
            char * e;
            char const * s(str.c_str());
            f_integer.push_back(strtol(s, &e, 10));
            if(e != s + str.length())
            {
                f_integer.clear();

                log << log_level_t::error
                    << "invalid number ("
                    << str
                    << ") in parameter --"
                    << f_name
                    << "."
                    << end;
                return -1;
            }
        }
    }

    return f_integer[idx];
}


/** \brief Lock this value.
 *
 * This function allows for locking a value so further reading of data
 * will not overwrite it.
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
    f_value.clear();
    f_integer.clear();
}



}   // namespace advgetopt
// vim: ts=4 sw=4 et
