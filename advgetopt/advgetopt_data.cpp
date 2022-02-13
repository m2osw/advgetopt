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
#include    "advgetopt/validator_double.h"
#include    "advgetopt/validator_integer.h"
#include    "advgetopt/version.h"


// cppthread lib
//
#include    <cppthread/log.h>


// C lib
//
#include    <string.h>


// last include
//
#include    <snapdev/poison.h>




namespace advgetopt
{





/** \brief Check whether a parameter is defined.
 *
 * This function returns true if the specified parameter is found as part of
 * the command line options.
 *
 * You must specify the long name of the option. So a `--verbose` option can
 * be checked with:
 *
 * \code
 *   if(is_defined("verbose")) ...
 * \endcode
 *
 * For options that come with a short name, you may also specify the short
 * name. This is done with a string in this case. It can be a UTF-8
 * character. The short name is used if the string represents exactly one
 * Unicode character. So the following is equivalent to the previous
 * example, assuming your verbose definition has `v` as the short name:
 *
 * \code
 *   if(is_defined("v")) ...
 * \endcode
 *
 * \note
 * This function returns true when the option was found on the command line,
 * the environment variable, or a configuration file. It returns false if
 * the option is defined, but was not specified anywhere by the client using
 * your program. Also, specifying the option in one of those three locations
 * when not allowed at that location will not result in this flag being raised.
 *
 * \param[in] name  The long name or short name of the option to check.
 *
 * \return true if the option was defined in a configuration file, the
 *         environment variable, or the command line.
 */
bool getopt::is_defined(std::string const & name) const
{
    is_parsed();

    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->is_defined();
    }

    return false;
}


/** \brief Retrieve the number of arguments.
 *
 * This function returns the number of arguments that were specified after
 * the named option.
 *
 * The function returns zero if the argument was never specified on the
 * command line. If the option accepts exactly one parameter (i.e. not
 * marked as a multiple arguments option: GETOPT_FLAG_MULTIPLE) then
 * the function returns either zero (not specified) or one (specified
 * at least once.)
 *
 * \param[in] name  The name of the option to check.
 *
 * \return The number of arguments specified on the command line or zero.
 */
size_t getopt::size(std::string const & name) const
{
    is_parsed();

    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->size();
    }
    return 0;
}


/** \brief Check whether an option has a default value.
 *
 * Some parameters may be given a default. This function is used to
 * detect whether such a default value is defined.
 *
 * \note
 * This function is particularly useful in the event the default value
 * may be an empty string.
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if this function is
 * called with an empty \p name.
 *
 * \param[in] name  The name of the parameter of which you want to know
 *                  whether it has a default value or not.
 *
 * \return true if the default value was defined (even if an empty string.)
 */
bool getopt::has_default(std::string const & name) const
{
    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->has_default();
    }

    return false;
}


/** \brief Get the default value for this option.
 *
 * When an option is not defined, you may use this function to retrieve its
 * default instead. This is actually done automatically when you call the
 * get_string() or get_long() functions.
 *
 * An option without a default has this function returning nullptr.
 *
 * \note
 * Whether an option has a default value should be checked with the
 * has_default() function which returns true when the default value
 * was defined. An option with an empty string as the default is
 * a valid case which cannot be detected otherwise.
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if this function is
 * called with an empty \p name.
 *
 * \param[in] name  The name of the parameter of which you want to retrieve
 *                  the default value.
 *
 * \return The default value or an empty string if no value is defined.
 */
std::string getopt::get_default(std::string const & name) const
{
    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->get_default();
    }

    return std::string();
}


/** \brief This function retrieves an argument as a long value.
 *
 * This function reads the specified argument from the named option and
 * transforms it to a long value. It then checks the result against the
 * specified minimum and maximum range.
 *
 * The function name represents an argument that needs to be defined. You
 * can test whether it was defined on the command line with the is_defined()
 * function. The index must be between 0 and 'size() - 1' inclusive. If
 * the item was not defined, then size() returns zero and you cannot call
 * this function.
 *
 * The function does not check the validity of the minimum and maximum
 * parameters. If \p min \> \p max is true then the function will always
 * fail with a call to usage() as no value can be defined between \p min
 * and \p max in that case. The minimum and maximum values are inclusive,
 * so a range of 1 to 9 is defined with exactly 1 and 9 in min and max.
 * For example, the z library compression could be retrieved with:
 *
 * \code
 * int level(6); // default to 6
 * if(opt.is_defined("zlevel"))
 * {
 *   zlevel = opt.get_long("zlevel", 0, 1, 9);
 * }
 * \endcode
 *
 * Note that the function can be used to read unsigned numbers, however
 * at this point getopt does not really support negative numbers (i.e. because
 * -\<number> is viewed as an option.)
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if \p name was not
 * found on the command line and it has no default, or if \p idx is
 * out of bounds.
 *
 * \param[in] name  The name of the option to retrieve.
 * \param[in] idx  The index of the argument to retrieve.
 * \param[in] min  The minimum value that will be returned (inclusive).
 * \param[in] max  The maximum value that will be returned (inclusive).
 *
 * \return The argument as a long.
 */
long getopt::get_long(std::string const & name, int idx, long min, long max) const
{
    is_parsed();

    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        throw getopt_logic_error(
                  "there is no --"
                + name
                + " option defined.");
    }

    long result(0.0);
    if(!opt->is_defined())
    {
        std::string const d(opt->get_default());
        if(d.empty())
        {
            throw getopt_logic_error(
                      "the --"
                    + name
                    + " option was not defined on the command line and it has no or an empty default.");
        }
        if(!validator_integer::convert_string(d, result))
        {
            // here we throw because this default value is defined in the
            // options of the tool and not by the user
            //
            throw getopt_logic_error(
                      "invalid default number \""
                    + d
                    + "\" for option --"
                    + name);
        }
    }
    else
    {
        result = opt->get_long(idx);
    }

    // TODO: replace with validators
    //
    if(result < min || result > max)
    {
        cppthread::log << cppthread::log_level_t::error
                       << result
                       << " is out of bounds ("
                       << min
                       << ".."
                       << max
                       << " inclusive) in parameter --"
                       << name
                       << "."
                       << cppthread::end;
        result = -1;
    }

    return result;
}


/** \brief This function retrieves an argument as a double value.
 *
 * This function reads the specified argument from the named option and
 * transforms it to a double value. It then checks the result against the
 * specified minimum and maximum range.
 *
 * The function name represents an argument that needs to be defined. You
 * can test whether it was defined on the command line with the is_defined()
 * function. The index must be between 0 and 'size() - 1' inclusive. If
 * the item was not defined, then size() returns zero and you cannot call
 * this function.
 *
 * The function does not check the validity of the minimum and maximum
 * parameters. If \p min \> \p max is true then the function will always
 * fail with a call to usage() as no value can be defined between \p min
 * and \p max in that case. The minimum and maximum values are inclusive,
 * so a range of 1 to 9 is defined with exactly 1 and 9 in min and max.
 * For example, the z library compression could be retrieved with:
 *
 * \code
 * int level(6); // default to 6
 * if(opt.is_defined("zlevel"))
 * {
 *   zlevel = opt.get_double("zlevel", 0, 1, 9);
 * }
 * \endcode
 *
 * Note that the function can be used to read unsigned numbers, however
 * at this point getopt does not really support negative numbers (i.e. because
 * -\<number> is viewed as an option.)
 *
 * \todo
 * Fix example with a parameter which makes sense (i.e. accepts doubles).
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if \p name was not
 * found on the command line and it has no default, or if \p idx is
 * out of bounds.
 *
 * \param[in] name  The name of the option to retrieve.
 * \param[in] idx  The index of the argument to retrieve.
 * \param[in] min  The minimum value that will be returned (inclusive).
 * \param[in] max  The maximum value that will be returned (inclusive).
 *
 * \return The argument as a long.
 */
double getopt::get_double(std::string const & name, int idx, double min, double max) const
{
    is_parsed();

    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        throw getopt_logic_error(
                  "there is no --"
                + name
                + " option defined.");
    }

    double result(0);
    if(!opt->is_defined())
    {
        std::string const d(opt->get_default());
        if(d.empty())
        {
            throw getopt_logic_error(
                      "the --"
                    + name
                    + " option was not defined on the command line and it has no or an empty default.");
        }
        if(!validator_double::convert_string(d, result))
        {
            // here we throw because this default value is defined in the
            // options of the tool and not by the user
            //
            throw getopt_logic_error(
                      "invalid default number \""
                    + d
                    + "\" for option --"
                    + name);
        }
    }
    else
    {
        result = opt->get_double(idx);
    }

    // TODO: replace with validators
    //
    if(result < min || result > max)
    {
        cppthread::log << cppthread::log_level_t::error
                       << result
                       << " is out of bounds ("
                       << min
                       << ".."
                       << max
                       << " inclusive) in parameter --"
                       << name
                       << "."
                       << cppthread::end;
        result = -1.0;
    }

    return result;
}


/** \brief Get the content of an option as a string.
 *
 * Get the content of the named parameter as a string. Command line options
 * that accept multiple arguments accept the \p idx parameter to
 * specify which item you are interested in.
 *
 * Note that the option must have been specified on the command line or have
 * a default value. For options that do not have a default value, you want
 * to call the is_defined() function first.
 *
 * \note
 * If the function returns the default value, it gets returned as is. i.e.
 * it won't be passed through the variable processing function.
 *
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if \p name was not
 * found on the command line and it has no default, or if \p idx is
 * out of bounds.
 *
 * \param[in] name  The name of the option to read.
 * \param[in] idx  The zero based index of a multi-argument command line option.
 * \param[in] raw  Whethre to return the value without replacing the variables.
 *
 * \return The option argument as a string.
 */
std::string getopt::get_string(
      std::string const & name
    , int idx
    , bool raw) const
{
    is_parsed();

    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        throw getopt_logic_error(
                  "there is no --"
                + name
                + " option defined.");
    }

    if(!opt->is_defined())
    {
        if(opt->has_default())
        {
            return opt->get_default();
        }
        throw getopt_logic_error(
                  "the --"
                + name
                + " option was not defined on the command line and it has no default.");
    }

    // it was defined, but if REQUIRED is not set and the value is empty
    // then we want to return the default if it has such defined
    //
    if(opt->has_default()
    && !opt->has_flag(GETOPT_FLAG_REQUIRED))
    {
        return opt->get_default();
    }

    return opt->get_value(idx, raw);
}


/** \brief Retrieve the value of an argument.
 *
 * This operator returns the value of an argument just like the get_string()
 * does when the argument is defined. When the argument is not defined and it
 * has no default, it returns an empty string instead of throwing.
 *
 * The function is only capable of returning the very first value. If this
 * argument has the GETOPT_FLAG_MULTIPLE flag set, you probably want to use
 * the get_string() instead.
 *
 * \param[in] name  The name of the option to retrieve.
 *
 * \return The value of that option or an empty string if not defined.
 */
std::string getopt::operator [] (std::string const & name) const
{
    is_parsed();

    if(name.empty())
    {
        throw getopt_logic_error("argument name cannot be empty.");
    }

    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        return std::string();
    }

    if(!opt->is_defined())
    {
        if(opt->has_default())
        {
            return opt->get_default();
        }
        return std::string();
    }

    return opt->get_value(0);
}


/** \brief Access a parameter in read and write mode.
 *
 * This function allows you to access an argument which may or may not
 * yet exist.
 *
 * The return value is a reference to that parameter. You can read
 * and write to the reference.
 *
 * A non-existant argument is created only if necessary. That is,
 * only if you actually use an assignment operator as follow:
 *
 * \code
 *      // straight assignment:
 *      opt["my-var"] = "123";
 *
 *      // or concatenation:
 *      opt["my-var"] += "append";
 * \endcode
 *
 * In read mode and unless you defined a default, a non-existant argument
 * is viewed as an empty string or 0 if retrieved as a long:
 *
 * \code
 *      // if non-existant you get an empty string:
 *      std::string value = opt["non-existant"];
 *
 *      // if non-existant you get zero:
 *      long value = opt["non-existant"].get_long();
 * \endcode
 *
 * The get_long() function may generate an error if the parameter is not
 * a valid integer. Also when a default is defined, it tries to convert
 * the default value to a number and if that fails an error is generated.
 *
 * \note
 * This operator only allows you to access the very first value of
 * this option. If the option is marked with GETOPT_FLAG_MULTIPLE,
 * you may want to use the get_option() function and then handle
 * the option multiple values manually with the option_info::get_value()
 * and option_info::set_value().
 *
 * \warning
 * If the option is an alias and the destination is not defined you
 * can still get an exception raised.
 *
 * \param[in] name  The name of the option to access.
 *
 * \return A reference to this option with support for many std::string like
 *         operators.
 */
option_info_ref getopt::operator [] (std::string const & name)
{
    is_parsed();

    if(name.empty())
    {
        throw getopt_logic_error("argument name cannot be empty.");
    }

    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        if(name.length() == 1)
        {
            throw getopt_logic_error("argument name cannot be one letter if it does not exist in operator [].");
        }

        // The option doesn't exist yet, create it
        //
        opt = std::make_shared<option_info>(name);
        opt->set_variables(f_variables);
        opt->add_flag(GETOPT_FLAG_DYNAMIC_CONFIGURATION);
        f_options_by_name[name] = opt;
    }

    return option_info_ref(opt);
}


/** \brief Generate a string describing whether we're using the sanitizer.
 *
 * This function determines whether this library was compiled with the
 * sanitizer extension. If so, then it will return detail about which
 * feature was compiled in.
 *
 * If no sanitizer options were compiled in, then it returns a
 * message saying so.
 *
 * \return A string with details about the sanitizer.
 */
std::string getopt::sanitizer_details()
{
    std::string result;
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
#if defined(__SANITIZE_ADDRESS__)
    result += "The address sanitizer is compiled in.\n";
#endif
#if defined(__SANITIZE_THREAD__)
    result += "The thread sanitizer is compiled in.\n";
#endif
#else
    result += "The address and thread sanitizers are not compiled in.\n";
#endif
    return result;
}


/** \brief Process the system options.
 *
 * If you have the GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS flag turned on,
 * then several options are automatically added to your list of supported
 * options, such as `--version`.
 *
 * This function processes these options if any were used by the client.
 *
 * If the function finds one or more system flags as being defined, it
 * returns a non-zero set of SYSTEM_OPTION_... flags. This can be useful
 * to decide whether to continue processing or not.
 *
 * We define a set of flags that can help you decide whether to continue
 * or exit. In most cases, we propose that you exit your program if any
 * one of the options was a command. This is done like so:
 *
 * \code
 * advgetopt::flag_t const r(process_system_options(stdout));
 * if((r & SYSTEM_OPTION_COMMANDS_MASK) != 0)
 * {
 *     exit(1);
 * }
 * \endcode
 *
 * You may still want to continue, though, if other flags where set,
 * even if some commands were used. For example, some tools will print
 * their version and move forward with there work (i.e. compilers often do
 * that to help with logging all the information about a build process,
 * including the version of the compiler.)
 *
 * \param[in] out  The stream where output is sent if required.
 *
 * \return non-zero set of flags if any of the system parameters were processed.
 */
flag_t getopt::process_system_options(std::basic_ostream<char> & out)
{
    flag_t result(SYSTEM_OPTION_NONE);

    // --version
    if(is_defined("version"))
    {
        if(f_options_environment.f_version == nullptr)
        {
            out << "warning: no version found." << std::endl;
        }
        else
        {
            out << f_options_environment.f_version << std::endl;
        }
        result |= SYSTEM_OPTION_VERSION;
    }

    // --has-sanitizer
    if(is_defined("has-sanitizer"))
    {
        out << sanitizer_details() << std::flush;
        result |= SYSTEM_OPTION_HELP;
    }

    // --compiler-version
    if(is_defined("compiler-version"))
    {
        out << LIBADVGETOPT_COMPILER_VERSION << std::endl;
        result |= SYSTEM_OPTION_HELP;
    }

    // --help
    if(is_defined("help"))
    {
        out << usage() << std::endl;
        result |= SYSTEM_OPTION_HELP;
    }

    // --long-help
    if(is_defined("long-help"))
    {
        out << usage(GETOPT_FLAG_SHOW_ALL) << std::endl;
        result |= SYSTEM_OPTION_HELP;
    }

    // --<group-name>-help
    //
    if(f_options_environment.f_groups != nullptr)
    {
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
                if(is_defined(option_name))
                {
                    out << usage(grp->f_group) << std::endl;
                    result |= SYSTEM_OPTION_HELP;
                }
            }
        }
    }

    // --copyright
    if(is_defined("copyright"))
    {
        if(f_options_environment.f_copyright == nullptr)
        {
            out << "warning: no copyright notice found." << std::endl;
        }
        else
        {
            out << f_options_environment.f_copyright << std::endl;
        }
        result |= SYSTEM_OPTION_COPYRIGHT;
    }

    // --license
    if(is_defined("license"))
    {
        if(f_options_environment.f_license == nullptr)
        {
            out << "warning: no license found." << std::endl;
        }
        else
        {
            out << f_options_environment.f_license << std::endl;
        }
        result |= SYSTEM_OPTION_LICENSE;
    }

    // --build-date
    if(is_defined("build-date"))
    {
        out << "Built on "
            << (f_options_environment.f_build_date == nullptr
                    ? "<no-build-date>"
                    : f_options_environment.f_build_date)
            << " at "
            << (f_options_environment.f_build_time == nullptr
                    ? "<no-build-time>"
                    : f_options_environment.f_build_time)
            << std::endl;
        result |= SYSTEM_OPTION_BUILD_DATE;
    }

    // --environment-variable-name
    if(is_defined("environment-variable-name"))
    {
        if(f_options_environment.f_environment_variable_name == nullptr
        || *f_options_environment.f_environment_variable_name == '\0')
        {
            out << f_options_environment.f_project_name
                << " does not support an environment variable."
                << std::endl;
        }
        else
        {
            out << f_options_environment.f_environment_variable_name << std::endl;
        }
        result |= SYSTEM_OPTION_ENVIRONMENT_VARIABLE_NAME;
    }

    // --configuration-filenames
    if(is_defined("configuration-filenames"))
    {
        string_list_t list(get_configuration_filenames(false, false));
        if(list.empty())
        {
            out << f_options_environment.f_project_name
                << " does not support configuration files."
                << std::endl;
        }
        else
        {
            out << "Configuration filenames:" << std::endl;
            for(auto n : list)
            {
                out << " . " << n << std::endl;
            }
        }
        result |= SYSTEM_OPTION_CONFIGURATION_FILENAMES;
    }

    // --path-to-option-definitions
    if(is_defined("path-to-option-definitions"))
    {
        if(f_options_environment.f_options_files_directory == nullptr
        || *f_options_environment.f_options_files_directory == '\0')
        {
            out << "/usr/share/advgetopt/options/" << std::endl;
        }
        else
        {
            out << f_options_environment.f_options_files_directory;
            if(f_options_environment.f_options_files_directory[strlen(f_options_environment.f_options_files_directory) - 1] != '/')
            {
                out << '/';
            }
            out << std::endl;
        }
        result |= SYSTEM_OPTION_PATH_TO_OPTION_DEFINITIONS;
    }

    // --config-dir
    if(is_defined("config-dir"))
    {
        // these are automatically used in the get_configuration_filenames()
        // function, there is nothing for us to do here
        //
        result |= SYSTEM_OPTION_CONFIG_DIR;
    }

    // --show-option-sources
    if(is_defined("show-option-sources"))
    {
        show_option_sources(out);
        result |= SYSTEM_OPTION_SHOW_OPTION_SOURCES;
    }

    return result;
}





} // namespace advgetopt
// vim: ts=4 sw=4 et
