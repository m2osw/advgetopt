/*
 * File:
 *    advgetopt/advgetopt.cpp -- advanced get option implementation
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
 * \brief Advanced getopt implementation.
 *
 * The advgetopt class and implementation is an advanced library to parse
 * command line parameters from static definitions specified by the caller.
 *
 * The class supports the command line options, options found in a default
 * configuration file or in a user defined configuration file.
 *
 * The class also includes support for displaying error messages and help
 * information about all the command line arguments.
 */


/** \mainpage
 * The advanced getopt library to handle your command line tools seemlessly.
 *
 * The library offers an advanced way to parse command line arguments,
 * an environment variable, and configuration files in a seamless manner.
 * The advgetopt::getopt class is what is used everywhere for that purpose.
 *
 * The class can be used in a very restrictive mode, meaning that all
 * the parameters must clearly be defined by the application. It can also
 * be used in a dynamic way where the parameters are dynamically added
 * to the list of available options.
 *
 * The library supports multiple levels in your options. The simplest
 * is to use a scope operator like so:
 *
 * \code
 * level1::level2::level3::etc = 123
 * \endcode
 *
 * \note
 * The library understands the scope operator (::), the period (.), and
 * the slash (/) as level separator. So the following are equivalent.
 * Internally, all are changed to the scope operator (::).
 *
 * \code
 * level1::level2::level3::etc = 123
 * level1.level2.level3.etc = 123
 * level1/level2/level3/etc = 123
 * \endcode
 *
 * The labels in a .ini format (i.e. `[name]` defines sections) are viewed
 * as a first level. That name automatically get prepended to the parameters
 * appearing under them. Additional levels can be added by using the
 * scope operator, again.
 *
 * \code
 * [level1]
 * level2::level3::etc = 123
 * \endcode
 *
 * Support for other formats may be added later. For example, we could
 * read XML and JSON files. Right now, we are focused on Unix configuration
 * files and pretty much never even need two levels.
 *
 * The library is capable of writing your configuration back to file. It
 * will know when a value was modified and only the modified values get
 * saved to the read/write configuration file(s). (Either the user file
 * or the system file under the `\<proc>.d/...` sub-path.)
 */

// self
//
#include    "advgetopt/advgetopt.h"

// advgetopt lib
//
#include    "advgetopt/conf_file.h"
#include    "advgetopt/exception.h"
#include    "advgetopt/log.h"

// boost lib
//
#include <boost/algorithm/string/replace.hpp>

// C++ lib
//
#include    <algorithm>
#include    <cstdarg>
#include    <cstring>
#include    <fstream>
#include    <iostream>
#include    <iomanip>
#include    <sstream>



/** \brief The advgetopt environment to parse command line options.
 *
 * This namespace encompasses all the declarations and implementations
 * of functions used to parse and access the command line options.
 */
namespace advgetopt
{


namespace
{

/** \brief Value when no default option was defined.
 *
 * Some options may have defaults in which case their indexes are used.
 * By default, an option has no defaults and we instead use -1.
 */
const int NO_DEFAULT_OPT = -1;


/** \brief Check whether this parameter is an argument.
 *
 * An argument is defined as a command line parameter that starts with
 * a dash and is not just "-".
 *
 * Note that "--" is viewed as an argument (this function returns true)
 * and the getopt class takes it as a command meaning any other parameter
 * is not an argument.
 */
bool is_arg(char const * a)
{
    // "-" and "--" are not options; however "--" returns true
    // because after a "--" we take the data as default arguments
    return a[0] == '-' && a[1] != '\0';
}

} // no name namespace



/** \class getopt_exception
 * \brief Base exception of the advgetopt class.
 *
 * This exception is the base exception of all the advgetopt exceptions.
 * Catching this exception allows you to capture all the getopt exceptions.
 */


/** \class getopt_exception_default
 * \brief No default and no value specified.
 *
 * When a parameter is not specified and no default is available, this
 * exception is raised.
 */


/** \class getopt_exception_undefined
 * \brief Attempting to access something that is undefined.
 *
 * This exception is used when you attempt to access data that was not
 * defined. For example, if your tool tries to read parameter "version"
 * and that was not defined in the list of options, this exception is
 * raised.
 */


/** \class getopt_exception_invalid
 * \brief Attempted to use some invalid data.
 *
 * This exception is used whenever an attempt is made to access data that
 * does not make sense (is invalid.)
 *
 * For example, the table of options makes use of enumerations for different
 * parts. If one of these has a value which does not represent a valid
 * enumeration value, then this exception is raised.
 */


/** \class getopt
 * \brief Class used to parse command line options.
 *
 * This class is the one used by all the wpkg tools to parse the command line
 * options. It is very advanced and is capable to read many different types
 * of options with a letter (-h) and a word (--verbose) with no parameters,
 * one parameter, any number of parameters, and a set of "filenames" (lose
 * options that are not specific to an option.)
 */


/** \struct option
 * \brief Structure representing an option.
 *
 * When creating a getopt() object you have to pass an array of options. That
 * array is defined as a set of option structures where the last one has
 * its f_arg_mode set to end_of_options. The other parameters may still be
 * defined as the last option is used to define what the parser should do
 * with the lose options (in most cases it is named "filenames" and used
 * as an array of files, paths, windows package names, etc.)
 */


/** \brief Initialize the getopt object.
 *
 * The constructor initializes a getopt object and parse the specified
 * argv array. If defined, it also parses a configuration file and
 * an environment variable.
 *
 * The order in which parameters are parsed is important since only the
 * last value is kept:
 *
 * \li Each existing configuration file in the order defined in the vector;
 * \li The environment variable;
 * \li the argv array.
 *
 * The constructor calls the reset() function to start the parsing. It is
 * possible to call the reset() function at any time to parse a new set
 * of parameters.
 *
 * The argv array cannot be nullptr and the array cannot be empty. It must have
 * at least one entry representing the program name (argv[0]).
 *
 * The configuration_files vector can be empty in which case no configuration
 * files are read.
 *
 * The environment_variable_name can be nullptr or the empty string in which case
 * it is ignored.
 *
 * \note
 * All the data gets copied while parsed. If the argv array is deleted on
 * return, the getopt object remains valid.
 *
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  An array of strings representing arguments.
 * \param[in] opt_env  The list of options that your program supports.
 */
getopt::getopt(options_environment const & opt_env
             , int argc
             , char * argv[])
    : f_options(std::make_shared<option_info>("root"))
{
    if(argv == nullptr)
    {
        throw getopt_exception_invalid("argv pointer cannot be nullptr");
    }
    if(&opt_env == nullptr)
    {
        throw getopt_exception_invalid("opt_env pointer cannot be nullptr");
    }

    f_options_environment = opt_env;

    parse_program_name(argv);
    parse_options_info();
    parse_options_from_file();

    if(f_options->get_children().empty())
    {
        throw getopt_exception_logic("an empty list of options is not legal, you must defined at least one (i.e. --version, --help...)");
    }

    link_aliases();

    parse_configuration_files();
    parse_environment_variable();
    parse_arguments(argc, argv, false);
}


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
    for(auto & opt : f_options->get_children())
    {
        opt.second->reset();
    }
}


/** \brief This function checks for arguments in configuration files.
 *
 * Each configuration file is checked one after another. Each file that is
 * defined is loaded and each line is viewed as an option. If valid, it is
 * added to the resulting getopt list of options.
 *
 * Note that it is an error to define a command in a configuration file. If
 * that happens, an error occurs and the process stops. Technically this is
 * defined with the GETOPT_FLAG_CONFIGURATION_FILE flag in your opt table.
 *
 * The list of files is checked from beginning to end. So if a later file
 * changes an option of an earlier file, it is the one effective.
 *
 * The configuration file loader supports a project name as defined in the
 * get_project_name() function. It allows for a sub-directory to
 * be inserted between the path and the basename of the configuration
 * file. This allows for a file to be search in an extra sub-directory
 * so one can avoid changing the original definitions and only use
 * configuration files in the sub-directory. The path looks like this
 * when a project name is specified:
 *
 * \code
 *      <path>/<project name>.d/<basename>
 * \endcode
 *
 * Notice that we add a ".d" as usual in other projects under Linux.
 *
 * \exception getopt_exception_invalid
 * This function generates the getopt_exception_invalid exception whenever
 * something invalid is found in the list of options passed as the \p opts
 * parameter.
 *
 * \exception getopt_exception_default
 * The function detects whether two options are marked as the default
 * option (the one receiving parameters that are not used by another command
 * or match a command.) This exception is raised when such is detected.
 */
void getopt::parse_configuration_files()
{
    if(f_options_environment.f_configuration_files == nullptr)
    {
        return;
    }

    // load options from configuration files specified as is by caller
    //
    for(char const * const * configuration_files(f_options_environment.f_configuration_files)
      ; *configuration_files != nullptr
      ; ++configuration_files)
    {
        char const * filename(*configuration_files);
        if(*filename == '\0')
        {
            // skip empty string silently
            //
            continue;
        }

        process_configuration_file(filename);

        if(f_options_environment.f_project_name != nullptr
        && *f_options_environment.f_project_name != '\0')
        {
            std::string adjusted_filename(filename);

            std::string::size_type const pos(adjusted_filename.find_last_of('/'));
            if(pos != std::string::npos
            && pos > 0)
            {
                adjusted_filename = adjusted_filename.substr(0, pos + 1)
                                  + f_options_environment.f_project_name
                                  + ".d"
                                  + adjusted_filename.substr(pos);
            }
            else
            {
                adjusted_filename = f_options_environment.f_project_name
                                  + (".d/" + adjusted_filename);
            }

            process_configuration_file(adjusted_filename);
        }
    }
}


/** \brief Parse one specific configuration file and process the results.
 *
 * This function reads a configuration file using a conf_file object and
 * then goes through the resulting arguments and add them to the options.
 *
 * The options found in the configuration file must match an option by
 * its long name. It is not allowed to have an option which is only one
 * character in a configuration file.
 *
 * \note
 * If the filename points to a file which can't be read or does not exist,
 * then nothing happens and the function returns without an error.
 *
 * \todo
 * Extend the support by having the various flags that the conf_file
 * class supports appear in the list of configuration filenames.
 *
 * \param[in] filename  The name of the configuration file to check out.
 */
void getopt::process_configuration_file(std::string const & filename)
{
    conf_file conf(filename);

    for(auto const & param : conf.get_parameters())
    {
        // in configuration files we only allow long arguments
        //
        option_info::pointer_t opt(get_option(param.first));
        if(opt == nullptr)
        {
            if((f_options_environment.f_environment_flags & GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS) == 0
            || param.first.length () == 1)
            {
                log << log_level_t::error
                    << "unknown option \""
                    << param.first
                    << "\" found in configuration file \""
                    << filename
                    << "\"."
                    << end;
                continue;
            }
            else
            {
                // add a new parameter dynamically
                //
                opt = std::make_shared<option_info>(param.first);

                opt->set_flags(GETOPT_FLAG_CONFIGURATION_FILE | GETOPT_FLAG_DYNAMIC);

                // consider the first definition as the default
                // (which is likely in our environment)
                //
                opt->set_default(param.second);

                f_options->add_child(opt);
            }
        }
        else
        {
            if(!opt->has_flag(GETOPT_FLAG_CONFIGURATION_FILE))
            {
                // in configuration files we are expected to use '_' so
                // print an error with such
                //
                log << log_level_t::error
                    << "option \""
                    << boost::replace_all_copy(param.first, "-", "_")
                    << "\" is not supported in configuration files (found in \""
                    << filename
                    << "\")."
                    << end;
                continue;
            }
        }

        if(opt != nullptr)
        {
            if(opt->has_flag(GETOPT_FLAG_MULTIPLE))
            {
                opt->set_multiple_value(param.second);
            }
            else if(!param.second.empty())
            {
                opt->set_value(0, param.second);
            }
            else if(opt->has_flag(GETOPT_FLAG_REQUIRED))
            {
                log << log_level_t::error
                    << "option \""
                    << boost::replace_all_copy(param.first, "-", "_")
                    << "\" must be given a value in configuration file \""
                    << filename
                    << "\"."
                    << end;
                continue;
            }
            else
            {
                opt->set_value(0, std::string()); // "" is enough, it is defined
            }
        }
    }
}


/** \brief Check for an environment variable.
 *
 * If the name of an environment variable is specified in the option
 * environment structure, then it is read as a command line string.
 * This function parses the string in an array of strings and then parses
 * it as an argv array (just like the argv parameter defined in a main()
 * function).
 *
 * Since the environment variable is checked after the configuration files,
 * the options defined in the variable can change the definitions from
 * the configuration files.
 *
 * Like in the configuration files, only options can be specified in the
 * environment variable and commands generate an error. The system knows
 * since options that can be included in the environment variable are
 * marked by the GETOPT_FLAG_ENVIRONMENT_VARIABLE flag. In other words,
 * you may allow options to appear on the command line, in configuration
 * files, in environment variables or a mix of all of these locations.
 */
void getopt::parse_environment_variable()
{
    if(f_options_environment.f_environment_variable_name == nullptr
    || *f_options_environment.f_environment_variable_name == '\0')
    {
        // no name
        //
        return;
    }

    char const * s(getenv(f_options_environment.f_environment_variable_name));
    if(s == nullptr)
    {
        // no environment variable with that name
        //
        return;
    }

    parse_string(s, true);
}


/** \brief Parse a string similar to a command line argument.
 *
 * This function parses a line of command line argument from a string.
 * Especially, it is used to parse the environment variable which is
 * a string of arguments.
 *
 * This can be used to parse the command line string as received under
 * MS-Windows (i.e. an unparsed one long string of arguments, where
 * you also need to do the glob() calls.)
 *
 * This function actually transforms the input string in an array of
 * strings and then calls the parse_arguments() function.
 *
 * \note
 * The input allows for an empty string in which case pretty much nothing
 * happens.
 *
 * \param[in] str  The string that is going to be parsed.
 * \param[in] only_environment_variable  Whether only options marked with
 *            the GETOPT_FLAG_ENVIRONMENT_VARIABLE flag are accepted.
 */
void getopt::parse_string(std::string const & str, bool only_environment_variable)
{
    // this is exactly like the command line only in an environment variable
    // so parse the parameters just like the shell
    //
    std::vector<std::string> args;
    std::string a;
    char const * s(str.c_str());
    while(*s != '\0')
    {
        if(isspace(*s))
        {
            if(!a.empty())
            {
                args.push_back(a);
                a.clear();
            }
            do
            {
                ++s;
            }
            while(isspace(*s));
        }
        else if(*s == '"' || *s == '\'')
        {
            // support quotations and remove them from the argument
            char const quote(*s++);
            while(*s != '\0' && *s != quote)
            {
                a += *s++;
            }
            if(*s != '\0')
            {
                ++s;
            }
        }
        else
        {
            a += *s++;
        }
    }

    if(!a.empty())
    {
        args.push_back(a);
    }

    if(args.empty())
    {
        // nothing extra to do
        //
        return;
    }

    // TODO: expand the arguments that include unquoted '*', '?', '[...]'
    //       (note that we remove the quoates at the moment so we'd have
    //       to keep track of that specific problem...)

    // the argv array has to be a null terminated bare string pointers
    //
    std::vector<char *> sub_argv;

    sub_argv.resize(args.size() + 2);

    // argv[0] is the program name
    //
    sub_argv[0] = const_cast<char *>(f_program_fullname.c_str());

    // the other arguments are from the variable
    //
    for(size_t idx(0); idx < args.size(); ++idx)
    {
        sub_argv[idx + 1] = const_cast<char *>(args[idx].c_str());
    }

    // this is probably already a nullptr
    //
    sub_argv[args.size() + 1] = nullptr;

    // now convert those parameters in values
    //
    parse_arguments(static_cast<int>(args.size() + 1), &sub_argv[0], only_environment_variable);
}


/** \brief Transform the argv[0] parameter in the program name.
 *
 * This function is transforms the first command line parameter in a program
 * name. It will define two versions, the basename and the fullname.
 *
 * \param[in] argv  The arguments vector.
 */
void getopt::parse_program_name(char * argv[])
{
    f_program_fullname = argv[0];
    std::string::size_type p(f_program_fullname.find_last_of('/'));
    if(p == std::string::npos)
    {
        // MS-Windows path uses \ instead of /
        //
        p = f_program_fullname.find_last_of('\\');
    }
    if(p != std::string::npos)
    {
        // remove the path
        //
        f_program_name = f_program_fullname.substr(p + 1);
    }
    else
    {
        f_program_name = f_program_fullname;
    }
}


/** \brief Parse the options to option_info objects.
 *
 * This function transforms an array of options in a vector of option_info
 * objects.
 */
void getopt::parse_options_info()
{
    if(f_options_environment.f_options == nullptr)
    {
        return;
    }

    for(option const * opts(f_options_environment.f_options)
      ; (opts->f_flags & GETOPT_FLAG_END) == 0
      ; ++opts)
    {
        if(opts->f_name == nullptr
        || opts->f_name[0] == '\0')
        {
            throw getopt_exception_logic("option long name missing or empty.");
        }
        if(opts->f_name[1] == '\0')
        {
            throw getopt_exception_logic("a long name option must be at least 2 characters.");
        }

        if(get_option(opts->f_name) != nullptr)
        {
            throw getopt_exception_logic(
                      std::string("option named \"")
                    + opts->f_name
                    + "\" found twice.");
        }
        if(get_option(opts->f_short_name) != nullptr)
        {
            throw getopt_exception_logic(
                      std::string("option with short name \"")
                    + static_cast<char>(opts->f_short_name)
                    + "\" found twice.");
        }

        option_info::pointer_t o(std::make_shared<option_info>(opts->f_name, opts->f_short_name));

        o->set_flags(opts->f_flags);
        o->set_default(opts->f_default);
        o->set_help(opts->f_help);
        o->set_multiple_separators(opts->f_multiple_separators);

        if(o->is_default_option())
        {
            if(f_default_option != nullptr)
            {
                throw getopt_exception_logic("two default options found after check of long names duplication.");
            }

            f_default_option = o;
        }

        f_options->add_child(o);
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
 *     minimum=<minimum value>
 *     maximum=<maximum value>
 *     help=<help sentence>
 *     validator=<validator name>|/<regex>/<flags>
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

    if(f_options_environment.f_options_files_directory == nullptr
    || f_options_environment.f_options_files_directory[0] == '\0')
    {
        filename = "/usr/share/advgetopt/";
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


    conf_file conf(filename);
    conf_file::sections_t const & sections(conf.get_sections());
    for(auto & s : sections)
    {
        std::string::size_type pos(s.first.find("::"));
        if(pos != std::string::npos)
        {
            log << log_level_t::error
                << "section \""
                << s.first
                << "\" includes a section separator (::) in \""
                << filename
                << "\". We only support one level."
                << end;
            continue;
        }

        std::string const parameter_name(s.first);
        std::string const short_name(conf.get_parameter(parameter_name + "::shortname"));
        if(short_name.length() > 1)
        {
            log << log_level_t::error
                << "option \""
                << s.first
                << "\" has an invalid short name in \""
                << filename
                << "\", it can't be more than one character."
                << end;
            continue;
        }
        short_name_t sn('\0');
        if(short_name.length() == 1)
        {
            sn = short_name[0];
        }

        option_info::pointer_t opt(std::make_shared<option_info>(parameter_name, sn));

        std::string const default_name(parameter_name + "::default");
        if(conf.has_parameter(default_name))
        {
            std::string const default_value(conf.get_parameter(default_name));
            opt->set_default(unquote(default_value));
        }

        opt->set_minimum(conf.get_parameter(parameter_name + "::minimum"));
        opt->set_maximum(conf.get_parameter(parameter_name + "::maximum"));
        opt->set_help(conf.get_parameter(parameter_name + "::help"));

        std::string const validator_name(parameter_name + "::validator");
        if(validator_name.length() >= 2
        && validator_name[0] == '/')
        {
            // the regex is a special case since we do not use the name
            // to find the validator
            //
            validator_regex::pointer_t v(std::make_shared<validator_regex>(validator_name));
            opt->set_validator(v);
        }
        else if(!validator_name.empty())
        {
            std::string::size_type const param(validator_name.find('('));
            std::string name(validator_name);
            std::string data;
            if(param != std::string::npos)
            {
                if(validator_name.back() != ')')
                {
                    log << log_level_t::error
                        << "option \""
                        << s.first
                        << "\" has an invalid validator parameter definition in \""
                        << validator_name
                        << "\", the ')' is missing."
                        << end;
                    continue;
                }
                name = validator_name.substr(0, param);
                data = unquote(validator_name.substr(param + 1, validator_name.length() - param - 2));
            }
            validator::pointer_t v(validator::create(validator_name, data));
            opt->set_validator(v);
        }

        if(conf.has_parameter(parameter_name + "::alias"))
        {
            opt->add_flag(GETOPT_FLAG_ALIAS);
        }

        std::string const allowed_name(parameter_name + "::allowed");
        if(conf.has_parameter(allowed_name))
        {
            std::string const allowed_list(conf.get_parameter(allowed_name));
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

        if(conf.has_parameter(parameter_name + "::show-usage-on-error"))
        {
            opt->add_flag(GETOPT_FLAG_SHOW_USAGE_ON_ERROR);
        }

        if(conf.has_parameter(parameter_name + "::no-arguments"))
        {
            opt->add_flag(GETOPT_FLAG_FLAG);
        }

        if(conf.has_parameter(parameter_name + "::multiple"))
        {
            opt->add_flag(GETOPT_FLAG_MULTIPLE);
        }

        if(conf.has_parameter(parameter_name + "::required"))
        {
            opt->add_flag(GETOPT_FLAG_REQUIRED);
        }

        f_options->add_child(opt);
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
    auto const & children(f_options->get_children());
    for(auto & c : children)
    {
        if(c.second->has_flag(GETOPT_FLAG_ALIAS))
        {
            std::string const & alias_name(c.second->get_default());
            if(alias_name.empty())
            {
                throw getopt_exception_invalid(
                          "the default value of your alias cannot be an empty string for \""
                        + c.first
                        + "\".");
            }

            option_info::pointer_t alias(get_option(alias_name));
            if(alias == nullptr)
            {
                throw getopt_exception_invalid(
                          "no option named \""
                        + alias_name
                        + "\" to satify the alias of \""
                        + c.first
                        + "\".");
            }

            c.second->set_alias(alias);
        }
    }
}


/** \brief Parse an array of arguments.
 *
 * This function accepts an array of arguments as received by the main()
 * function. By default, though, you pass the argc/argv parameters to
 * the getopt() constructor which automatically calls this function.
 *
 * This functin is public so you can call it with additional lists of
 * arguments. If that list of arguments comes as a string, you may want
 * to call the parse_string() function instead. It will transform your
 * string in a list of parameters for you.
 *
 * When the \p only_environment_variable parameter is set to true, then
 * it is considered that the input arguments were found in an environment
 * variables and they are only accepted if the corresponding option
 * definition includes the GETOPT_FLAG_ENVIRONMENT_VARIABLE flag.
 *
 * When the \p only_environment_variable parameter is set to false, the
 * arguments are viewed as command line arguments and the corresponding
 * options must include the GETOPT_FLAG_COMMAND_LINE flag.
 *
 * Variables get overridden by the newest values found in the list of
 * arguments.
 *
 * Note that the command line arguments are the only ones that should
 * include a command (opposed to an option that alters the behavior of
 * your commands.) However, the advgetopt system expects you to properly
 * define what can be used in a configuration file, in an environment
 * variable, or directly on the command line. It is not in charge of
 * that part in itself.
 *
 * \note
 * The code may find some errors in the tables passed to the advgetopt
 * environment (i.e. a duplicate definition.) When such errors are
 * detected, an exception is raised. Errors found on the command line
 * generate a log message. If you setup a callback, you can then decide
 * to either call exit(1) or raise your own exception.
 *
 * \note
 * The function does NOT check whether the list of arguments (argv) is
 * terminated by nullptr. The argc parameter must be correct.
 *
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  The argument strings terminated by a nullptr.
 * \param[in] only_environment_variable  Accept command line arguments (false)
 *            or environment variable arguments (true).
 */
void getopt::parse_arguments(int argc
                           , char * argv[]
                           , bool only_environment_variable)
{
    for(int i(1); i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            if(argv[i][1] == '-')
            {
                if(argv[i][2] == '\0')
                {
                    // end of options, skip the '--' and then anything else
                    // is taken as "filenames" (or whatever the tool expects)
                    //
                    if(f_default_option == nullptr)
                    {
                        log << log_level_t::error
                            << "no default options defined; thus -- is not accepted by this program."
                            << end;
                        break;
                    }

                    if(only_environment_variable)
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                        {
                            log << log_level_t::error
                                << "option -- is not supported in the environment variable."
                                << end;
                            break;
                        }
                    }
                    else
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_COMMAND_LINE))
                        {
                            log << log_level_t::error
                                << "option -- is not supported in the environment variable."
                                << end;
                            break;
                        }
                    }

                    // in this case we do NOT test whether an argument uses
                    // a dash (-) we take them all as default options
                    //
                    while(i + 1 < argc)
                    {
                        ++i;
                        add_option(f_default_option, argv[i]);
                    }
                }
                else
                {
                    // a long option, check that it is defined in the
                    // programmer defined options
                    //
                    option_info::pointer_t opt(get_option(argv[i] + 2));
                    if(opt == nullptr)
                    {
                        log << log_level_t::error
                            << "option "
                            << argv[i]
                            << " is not supported."
                            << end;
                        break;
                    }
                    if(only_environment_variable)
                    {
                        if(!opt->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                        {
                            log << log_level_t::error
                                << "option "
                                << argv[i]
                                << " is not supported in the environment variable."
                                << end;
                            break;
                        }
                    }
                    else
                    {
                        if(!opt->has_flag(GETOPT_FLAG_COMMAND_LINE))
                        {
                            log << log_level_t::error
                                << "option "
                                << argv[i]
                                << " is not supported on the command line."
                                << end;
                            break;
                        }
                    }
                    add_options(opt, i, argc, argv);
                }
            }
            else
            {
                if(argv[i][1] == '\0')
                {
                    // stdin/stdout (a '-' by itself)
                    //
                    if(f_default_option == nullptr)
                    {
                        log << log_level_t::error
                            << "no default options defined; thus - is not accepted by this program."
                            << end;
                        break;
                    }
                    if(only_environment_variable)
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                        {
                            log << log_level_t::error
                                << "option - is not supported in the environment variable."
                                << end;
                            break;
                        }
                    }
                    else
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_COMMAND_LINE))
                        {
                            log << log_level_t::error
                                << "option - is not supported in the environment variable."
                                << end;
                            break;
                        }
                    }

                    // this is similar to a default option by itself
                    //
                    add_option(f_default_option, argv[i]);
                }
                else
                {
                    // short option(s)
                    //
                    // i gets incremented by add_options() so we have to
                    // keep a copy of `k`
                    //
                    int const k(i);
                    int const max(static_cast<int>(strlen(argv[k])));
                    for(int j = 1; j < max; ++j)
                    {
                        // TODO: add support for UTF-32 characters
                        //
                        option_info::pointer_t opt(get_option(argv[k][j]));
                        if(opt == nullptr)
                        {
                            log << log_level_t::error
                                << "option -"
                                << argv[k][j]
                                << " is not supported."
                                << end;
                            break;
                        }
                        if(only_environment_variable)
                        {
                            if(!opt->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                            {
                                log << log_level_t::error
                                    << "option -"
                                    << argv[k][j]
                                    << " is not supported in the environment variable."
                                    << end;
                                break;
                            }
                        }
                        else
                        {
                            if(!opt->has_flag(GETOPT_FLAG_COMMAND_LINE))
                            {
                                log << log_level_t::error
                                    << "option -"
                                    << argv[k][j]
                                    << " is not supported on the command line."
                                    << end;
                                break;
                            }
                        }
                        add_options(opt, i, argc, argv);
                    }
                }
            }
        }
        else
        {
            // direct entry (filename or whatever the tool expects as a default)
            //
            if(f_default_option == nullptr)
            {
                log << log_level_t::error
                    << "no default options defined; we do not know what to do of \""
                    << argv[i]
                    << "\"; standalone parameters are not accepted by this program."
                    << end;
                break;
            }
            if(only_environment_variable)
            {
                if(!f_default_option->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                {
                    log << log_level_t::error
                        << "default options are not supported in the environment variable."
                        << end;
                    break;
                }
            }
            else
            {
                if(!f_default_option->has_flag(GETOPT_FLAG_COMMAND_LINE))
                {
                    log << log_level_t::error
                        << "default options are not supported on the command line."
                        << end;
                    break;
                }
            }
            add_option(f_default_option, argv[i]);
        }
    }
}


/** \brief Check whether a parameter is defined.
 *
 * This function returns true if the specified parameter is found as part of
 * the command line options.
 *
 * You must specify the long name of the option if one is defined. Otherwise
 * the name is the short name. So a --verbose option can be checked with:
 *
 * \code
 *   if(is_defined("verbose")) ...
 * \endcode
 *
 * However, if the option was defined as:
 *
 * \code
 * advgetopt::option options[] =
 * {
 *    [...]
 *    {
 *       'v',
 *       0,
 *       nullptr,
 *       nullptr,
 *       "increase verbosity",
         advgetopt::getopt::no_argument
 *    },
 *    [...]
 * };
 * \endcode
 *
 * then the previous call would fail because "verbose" does not exist in your
 * table. However, the option is accessible by its short name as a fallback
 * when it does not have a long name:
 *
 * \code
 *   if(is_defined("v")) ...
 * \endcode
 *
 * \param[in] name  The long (or short if long is undefined) name of the
 *                  option to check.
 *
 * \return true if the option was defined in a configuration file, the
 *         environment variable, or the command line.
 */
bool getopt::is_defined(std::string const & name) const
{
    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->is_defined();
    }

    return false;
}


/** \brief Retrieve an option by name.
 *
 * This function retrieves an option by name. The function handles the
 * special case of the default option. This means "--" can always be
 * used to access the default option, whever the name given to that
 * option in the declaration of your options.
 *
 * Of course, if no default is defined, then "--" returns a nullptr.
 *
 * \param[in] name  The name of the option to retrieve.
 *
 * \return The pointer to the named option or nullptr if not found.
 */
option_info::pointer_t getopt::get_option(std::string const & name) const
{
    // we need a special case when looking for the default option
    // because the name may not be "--" in the option table
    // (i.e. you could call your default option "filenames" for
    // example.)
    //
    if(name.length() == 2
    && name[0] == '-'
    && name[1] == '-')
    {
        return f_default_option;
    }

    if(name.length() == 1)
    {
        return f_options->get_child(name[0]);
    }
    else
    {
        return f_options->get_child(name);
    }
}


/** \brief Get an option using its short name.
 *
 * This function searches for an option given its short name.
 *
 * \param[in] short_name  The short name of the option to look for.
 *
 * \return The pointer to the option or nullptr if not found.
 */
option_info::pointer_t getopt::get_option(short_name_t short_name) const
{
    return f_options->get_child(short_name);
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
    option_info::pointer_t opt(get_option(name));
    if(opt != nullptr)
    {
        return opt->size();
    }
    return 0;
}


/** \brief Get the default value for this option.
 *
 * When an option is not defined, you may use this function to retrieve its
 * default instead. This is actually done automatically when you call the
 * get_string() or get_long() functions.
 *
 * An option without a default has this function returning nullptr.
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
    if(name.empty())
    {
        throw getopt_exception_undefined("command line name cannot be empty.");
    }

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
long getopt::get_long(std::string const & name, int idx, long min, long max)
{
    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        throw getopt_exception_undefined(
                  "there is no \"--"
                + name
                + "\" option defined.");
    }

    long result(0);
    int max_idx = opt->size();
    if(max_idx == 0)
    {
        std::string const d(opt->get_default());
        if(d.empty())
        {
            throw getopt_exception_undefined(
                      "the \"--"
                    + name
                    + "\" option was not defined on the command line.");
        }
        char * end;
        char const * str(d.c_str());
        result = strtol(str, &end, 10);
        if(end != str + d.length())
        {
            // here we throw because this default value is defined in the
            // options of the tool and not by the user
            //
            throw getopt_exception_invalid(
                      "invalid default number \""
                    + d
                    + "\" in parameter --"
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
        log << log_level_t::error
            << result
            << " is out of bounds ("
            << min
            << ".."
            << max
            << " inclusive) in parameter --"
            << name
            << "."
            << end;
        result = -1;
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
 * \exception getopt_exception_undefined
 * The getopt_exception_undefined exception is raised if \p name was not
 * found on the command line and it has no default, or if \p idx is
 * out of bounds.
 *
 * \param[in] name  The name of the option to read.
 * \param[in] idx  The zero based index of a multi-argument command line option.
 *
 * \return The option argument as a string.
 */
std::string getopt::get_string(std::string const & name, int idx) const
{
    option_info::pointer_t opt(get_option(name));
    if(opt == nullptr)
    {
        throw getopt_exception_undefined(
                  "there is no \"--"
                + name
                + "\" option defined.");
    }

    if(!opt->is_defined())
    {
        if(opt->has_default())
        {
            return opt->get_default();
        }
        throw getopt_exception_undefined(
                  "the --"
                + name
                + " option was not defined on the command line and it has no default.");
    }

    return opt->get_value(idx);
}


/** \brief Change the %p in help strings in the program name.
 *
 * Because the same set of options may be used by different programs,
 * having the usage directly in the list of options may break the
 * usage name. So here we offer a dynamic way of changing the help
 * string with "%p". For example:
 *
 * \code
 *    "Usage: %p [-opt] filename ..."
 * \endcode
 *
 * \li "%%" -- print out a percent
 * \li "%a" -- print out the project name (a.k.a. application name)
 * \li "%b" -- print out the build date
 * \li "%c" -- print out the copyright notice
 * \li "%d" -- print out the first directory with configuration files.
 * \li "%*d" -- print out the complete list of directories with configuration
 * files.
 * \li "%f" -- print out the first configuration path and filename.
 * \li "%*f" -- print out all the configuration full paths.
 * \li "%l" -- print out the license
 * \li "%p" -- print out the program basename
 * \li "%*p" -- print out the full program name
 * \li "%t" -- print out the build time
 * \li "%v" -- print out the version
 *
 * \param[in] help  A string that may include '%p'.
 *
 * \return The string with any '%p' replaced with the program name.
 */
std::string getopt::process_help_string( char const * help ) const
{
    if(help == nullptr)
    {
        return std::string();
    }

    std::string result;

    while(help[0] != '\0')
    {
        if(help[0] == '%')
        {
            switch(help[1])
            {
            case '%':
                result += '%';
                help += 2;
                break;

            case '*':
                switch(help[2])
                {
                case 'd':
                    if(f_options_environment.f_configuration_directories != nullptr)
                    {
                        bool first(true);
                        for(char const * const * directories(f_options_environment.f_configuration_directories)
                          ; *directories != nullptr
                          ; ++directories)
                        {
                            if(first)
                            {
                                first = false;
                            }
                            else
                            {
                                result += ", ";
                            }
                            result += *directories;
                        }
                    }
                    help += 3;
                    break;

                case 'f':
                    if(f_options_environment.f_configuration_files != nullptr)
                    {
                        bool first(true);
                        for(char const * const * filenames(f_options_environment.f_configuration_files)
                          ; *filenames != nullptr
                          ; ++filenames)
                        {
                            if(first)
                            {
                                first = false;
                            }
                            else
                            {
                                result += ", ";
                            }
                            result += *filenames;
                        }
                    }
                    help += 3;
                    break;

                case 'p':
                    result += f_program_fullname;
                    help += 3;
                    break;

                }
                break;

            case 'a':
                result += f_options_environment.f_project_name;
                help += 2;
                break;

            case 'b':
                result += f_options_environment.f_build_date;
                help += 2;
                break;

            case 'c':
                result += f_options_environment.f_copyright;
                help += 2;
                break;

            case 'd':
                if(f_options_environment.f_configuration_directories != nullptr)
                {
                    if(*f_options_environment.f_configuration_directories != nullptr)
                    {
                        result += *f_options_environment.f_configuration_directories;
                    }
                }
                help += 2;
                break;

            case 'f':
                if(f_options_environment.f_configuration_files != nullptr)
                {
                    if(*f_options_environment.f_configuration_files != nullptr)
                    {
                        result += *f_options_environment.f_configuration_files;
                    }
                }
                help += 2;
                break;

            case 'l':
                result += f_options_environment.f_license;
                help += 2;
                break;

            case 'p':
                result += f_program_name;
                help += 2;
                break;

            case 't':
                result += f_options_environment.f_build_time;
                help += 2;
                break;

            case 'v':
                result += f_options_environment.f_version;
                help += 2;
                break;

            }
        }
        else
        {
            result += help[0];
            ++help;
        }
    }

    return result;
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
std::string getopt::usage( flag_t show ) const
{
    std::stringstream ss;

    // ignore all the non-show flags
    //
    show &= GETOPT_FLAG_SHOW_USAGE_ON_ERROR
          | GETOPT_FLAG_SHOW_ALL
          | GETOPT_FLAG_SHOW_GROUP1
          | GETOPT_FLAG_SHOW_GROUP2;

    ss << breakup_line(process_help_string(f_options_environment.f_help_header), 0, 80);

    std::string save_default;
    std::string save_help;

    for(auto const & opt : f_options->get_children())
    {
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
            else if(opt.second->has_flag(GETOPT_FLAG_SHOW_GROUP1 | GETOPT_FLAG_SHOW_GROUP2))
            {
                // do not show specialized groups
                //
                continue;
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
            if(opt.second->get_short_name() != NO_SHORT_NAME)
            {
                // TODO: add support for Unicode characters
                //
                argument << " or -" << static_cast<char>(opt.second->get_short_name());
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
            ss << format_usage_string(argument.str()
                                    , process_help_string(help.c_str())
                                    , 30
                                    , 80);
        }
    }

    if(!save_default.empty())
    {
        ss << format_usage_string(save_default
                                , process_help_string(save_help.c_str())
                                , 30
                                , 80);
    }

    if(f_options_environment.f_help_footer != nullptr
    && f_options_environment.f_help_footer[0] != '\0')
    {
        ss << std::endl;
        ss << breakup_line(process_help_string(f_options_environment.f_help_footer), 0, 80);
    }

    return ss.str();
}


/** \brief Format a help string to make it fit on a given width.
 *
 * This function properly wraps a set of help strings so they fit in
 * your console. The width has to be given by you at the moment.
 *
 * The function takes two strings, the argument with it's options
 * and the actual help string for that argument. If the argument
 * is short enough, it will appear on the first line with the
 * first line of help. If not, then one whole line is reserved
 * just for the argument and the help starts on the next line.
 *
 * \param[in] argument  The option name with -- and arguments.
 * \param[in] help  The help string for this argument.
 * \param[in] option_width  Number of characters reserved for the option.
 * \param[in] line_width  The maximum number of characters to display in width.
 *
 * \return A help string formatted for display.
 */
std::string getopt::format_usage_string(
                      std::string const & argument
                    , std::string const & help
                    , size_t const option_width
                    , size_t const line_width)
{
    std::stringstream ss;

    ss << "   ";

    if( argument.size() < option_width - 3 )
    {
        // enough space on a single line
        //
        ss << argument
           << std::setw( option_width - 3 - argument.size() )
           << " ";
    }
    else if(argument.size() >= line_width - 4)
    {
        // argument too long for even one line on the screen!?
        // call the function to break it up with indentation of 3
        //
        ss << breakup_line(argument, 3, line_width);
    }
    else
    {
        // argument too long for the help to follow immediately
        //
        ss << argument
           << std::endl
           << std::setw( option_width )
           << " ";
    }

    ss << breakup_line(help, option_width, line_width);

    return ss.str();
}


/** \brief Breakup a string on multiple lines.
 *
 * This function breaks up the specified \p line of text in one or more
 * strings to fit your output.
 *
 * The \p line_width represents the maximum number of characters that get
 * printed in a row.
 *
 * The \p option_width parameter is the number of characters in the left
 * margin. When dealing with a very long argument, this width is 3 characters.
 * When dealing with the help itself, it is expected to be around 30.
 *
 * \note
 * This function always makes sure that the resulting string ends with
 * a newline character unless the input \p line string is empty.
 *
 * \param[in] line  The line to breakup.
 * \param[in] option_width  The number of characters in the left margin.
 * \param[in] line_width  The total number of characters in the output.
 *
 * \return The broken up line as required.
 */
std::string getopt::breakup_line(std::string line
                               , size_t const option_width
                               , size_t const line_width)
{
    std::stringstream ss;

    size_t const width(line_width - option_width);

    while(line.size() > width)
    {
        std::string l;
        std::string::size_type const nl(line.find('\n'));
        if(nl != std::string::npos
        && nl < width)      // we could avoid this problem with std::string_view (C++17)
        {
            l = line.substr(0, nl);
            line = line.substr(nl + 1);
        }
        else if(std::isspace(line[width]))
        {
            // special case when the space is right at the edge
            //
            l = line.substr(0, width);
            line = line.substr(width + 1);
        }
        else
        {
            // search for the last space before the edge of the screen
            //
            std::string::size_type const pos(line.find_last_of(' ', width));
            if(pos == std::string::npos)
            {
                // no space found, cut right at the edge...
                // (this should be really rare)
                //
                l = line.substr(0, width);
                line = line.substr(width);
            }
            else
            {
                // we found a space, write everything up to that space
                //
                l = line.substr(0, pos);
                line = line.substr(pos + 1);
            }
        }

        ss << l
           << std::endl;

        // more to print? if so we need the indentation
        //
        if(!line.empty()
        && option_width > 0)
        {
            ss << std::setw( option_width ) << " ";
        }
    }

    // some leftover?
    //
    if(!line.empty())
    {
        ss << line << std::endl;
    }

    return ss.str();
}


/** \brief Get the full name of the program.
 *
 * This function return the name of the program exactly as it was passed to
 * the program via argv[0].
 *
 * The reset() function will reset this parameter. If you are creating
 * internal lists of parameters that you want to parse with the same
 * getopt object and your main getopt object, then you may want to
 * consider using this function to define argv[0] of your new list:
 *
 * \code
 * std::vector<std::string> args;
 * args.push_back(my_opts.get_program_fullname());
 * args.push_back("--test");
 * [...]
 * // the following probably require some const_cast<>(), but that's the idea
 * my_opts.reset(args.size(), &args[0], ...);
 * \endcode
 *
 * \return The contents of the argv[0] parameter as defined on construction.
 */
std::string getopt::get_program_fullname() const
{
    return f_program_fullname;
}


/** \brief Get the basename of the program.
 *
 * This function retrieves the basename, the name of the program with its
 * path trimmed, from this getopt object.
 *
 * This is defined from the argv[0] parameter passed to the constructor or
 * the last reset() call.
 *
 * \return The basename of the program.
 */
std::string getopt::get_program_name() const
{
    return f_program_name;
}


/** \brief Retrieve the project name if one is defined.
 *
 * This function returns the name of the project as defined in the
 * options_environment structure passed to the constructor.
 *
 * For example, the snapwebsites project makes use of "snapwebsites"
 * name as its common project name. Many of the configuration files are
 * found under that sub-folder. This ensures that the configuration files
 * are searched for under the indicated folders and again under:
 *
 * \code
 *      <existing path>/<project name>.d/<basename>
 * \endcode
 *
 * So if you have a configuration file named "snapserver.conf" with
 * a path such as "/etc/snapwebsites", you end up with:
 *
 * \code
 *      "/etc/snapwebsites/snapserver.conf"
 *      "/etc/snapwebsites/snapwebsites.d/snapserver.conf"
 * \endcode
 *
 * Notice that the loader adds a ".d" at the end of the project name.
 * Also, if the user were to specify a different filename with the
 * --config command line option, it could end up like this:
 *
 * \code
 *      ... --config /home/alexis/.config/iplock/iplock.conf ...
 *
 *      # First we read this file:
 *      "/home/alexis/.config/iplock/iplock.conf"
 *
 *      # Second we read this file (assuming the same project name
 *      # of "snapwebsites"):
 *      "/home/alexis/.config/iplock/snapwebsites.d/iplock.conf"
 * \endcode
 *
 * The order is important as well. We first load the direct path, then
 * the path with the sub-folder. Finally, we move forward to the next
 * configuration file. We ignore errors when a file can't be loaded or
 * is missing.
 *
 * \return The name of the project, maybe empty if undefined.
 */
std::string getopt::get_project_name() const
{
    return f_options_environment.f_project_name;
}


/** \brief Read parameters of the current option.
 *
 * This function saves the option in the list of options found in this list
 * of arguments. If the option is expected to have parameters, then those
 * are taken from the argv array before the function saves the option in
 * the object list. The index, \p i, is increased accordingly.
 *
 * \warning
 * This function cannot be called properly with the '-' option in case it
 * is viewed as a default parameter. This is because the algorithm expects
 * the index (\p i) to be pointing to the command line option and not the
 * argument to that command.
 *
 * \param[in] opt  The concerned option
 * \param[in] i  The current position, starting with the option position
 * \param[in] argc  The number of arguments in the argv array.
 * \param[in] argv  The list of argument strings.
 */
void getopt::add_options(option_info::pointer_t opt, int & i, int argc, char ** argv)
{
    if(opt->has_flag(GETOPT_FLAG_FLAG))
    {
        add_option(opt, opt->get_default());
    }
    else
    {
        if(i + 1 < argc && !is_arg(argv[i + 1]))
        {
            if(opt->has_flag(GETOPT_FLAG_MULTIPLE))
            {
                do
                {
                    ++i;
                    add_option(opt, argv[i]);
                } while(i + 1 < argc && !is_arg(argv[i + 1]));
            }
            else
            {
                ++i;
                add_option(opt, argv[i]);
            }
        }
        else
        {
            if(opt->has_flag(GETOPT_FLAG_REQUIRED))
            {
                log << log_level_t::error
                    << "option --"
                    << opt->get_name()
                    << " expects an argument."
                    << end;
            }
            else
            {
                add_option(opt, opt->get_default());
            }
        }
    }
}


/** \brief Add one option to the internal list of options.
 *
 * This function adds the actual option name and value pair to the
 * option list.
 *
 * The name of the option is taken from the first one of these that is
 * defined:
 *
 * \li For options marked as an alias, use the f_help alias
 * \li For options with a long name, use the f_name string
 * \li For options with a short name, use the f_opt character as a name
 * \li In all other cases, use "--" as a fallback
 *
 * If the function is called multiple times with the same option and that
 * option is not marked as a "multiple" argument option, then the function
 * overwrites the current value with the latest passed to this function.
 * In other words, only the last argument in your configuration files,
 * environment variable, or command line options is kept.
 *
 * Options having a "multiple" flag accept multiple calls and each instance
 * is saved in the order found in your configuration files, environment
 * variable, and command line options.
 *
 * \note
 * The value pointer can be set to nullptr in which case it is considered to
 * be equivalent to "" (the empty string.)
 *
 * \note
 * Options that are marked as "no argument" ignore the value parameter
 * altogether.
 *
 * \param[in] opt  The concerned option.
 * \param[in] value  The value to add to that option info.
 */
void getopt::add_option(option_info::pointer_t opt, std::string const & value)
{
    if(opt->has_flag(GETOPT_FLAG_ALIAS))
    {
        opt = opt->get_alias();
        if(opt == nullptr)
        {
            throw getopt_exception_undefined("getopt::add_option(): alias is missing.");
        }
    }

    if(opt->has_flag(GETOPT_FLAG_MULTIPLE))
    {
        opt->add_value(value);
    }
    else
    {
        opt->set_value(0, value);
    }
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
