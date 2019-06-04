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

// libutf8 lib
//
#include    <libutf8/libutf8.h>

// boost lib
//
#include    <boost/algorithm/string/replace.hpp>

// C++ lib
//
//#include    <iostream>



/** \brief The advgetopt environment to parse command line options.
 *
 * This namespace encompasses all the declarations and implementations
 * of functions used to parse and access the command line options.
 */
namespace advgetopt
{


namespace
{


/** \brief Definitions of the system options.
 *
 * The system options are options we add automatically (if the user asked
 * for them) and handle automatically when they are found.
 *
 * The following are the currently supported system options:
 *
 * \li `--help`
 *
 * Print out the usage() and exits.
 *
 * \li `--version`
 *
 * Print out the version and exits.
 *
 * \li `--copyright`
 *
 * Print out the copyright notice and exits.
 *
 * \li `--license`
 *
 * Print out the license notice and exits.
 *
 * \li `--build-date`
 *
 * Print out the build time and date and exits.
 *
 * \li `--environment-variable-name`
 *
 * Print out the build time and date and exits.
 *
 * \li `--configuration-filenames`
 *
 * Print out the list of configuration file names that the system checks
 * for configuration data.
 *
 * \li `--path-to-option-definitions`
 *
 * Print out the path to files which define options for this tool.
 */
option const g_system_options[] =
{
    define_option(
          Name("help")
        , ShortName('h')
        , Flags(standalone_command_flags())
        , Help("print out this help screen and exit.")
    ),
    define_option(
          Name("version")
        , ShortName('V')
        , Flags(standalone_command_flags())
        , Help("print out the version of %p and exit.")
    ),
    define_option(
          Name("copyright")
        , ShortName('C')
        , Flags(standalone_command_flags())
        , Help("print out the copyright of %p and exit.")
    ),
    define_option(
          Name("license")
        , ShortName('L')
        , Flags(standalone_command_flags())
        , Help("print out the license of %p and exit.")
    ),
    define_option(
          Name("build-date")
        , Flags(standalone_command_flags())
        , Help("print out the time and date when %p was build and exit.")
    ),
    define_option(
          Name("environment-variable-name")
        , Flags(standalone_command_flags())
        , Help("print out the name of the environment variable supported by %p (if any.)")
    ),
    define_option(
          Name("configuration-filenames")
        , Flags(standalone_command_flags())
        , Help("print out the list of configuration files checked out by this tool.")
    ),
    define_option(
          Name("path-to-option-definitions")
        , Flags(standalone_command_flags())
        , Help("print out the path to the option definitons.")
    ),
    end_options()
};




/** \brief Value when no default option was defined.
 *
 * Some options may have defaults in which case their indexes are used.
 * By default, an option has no defaults and we instead use -1.
 */
int const NO_DEFAULT_OPT = -1;


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
 * \section into Introduction
 *
 * This constructor initializes a getopt object. It also reads and parses
 * the corresponding option configuration file if it exists (based on the
 * project name defined in the environment parameter.)
 *
 * \section program_name Program Name
 *
 * Once constructed, if you want to have access to the program name, make
 * sure to call this function with your `argv` variable:
 *
 * \code
 *     opt.parse_program_name(argv);
 * \endcode
 *
 * Remember that the program name is often used in error messages so having
 * it defined early is generally a good idea.
 *
 * \section dynamism Dynamic Options
 *
 * This constructor is most often used when you want to dynamically add
 * options to your executable with the parse_options_info() function.
 * For example, the list of options may vary slightly depending on what
 * your command is named when launched.
 *
 * For example:
 *
 * \code
 *     if(time(nullptr) & 1)
 *     {
 *         opt.parse_options_info(odd_options);
 *     }
 *     else
 *     {
 *         opt.parse_options_info(even_options);
 *     }
 * \endcode
 *
 * \section aliases Linking Aliases
 *
 * After you added all your dynamic options, you want to make sure that
 * aliases are linked to the final option. You should always call that
 * function because you can't be sure whether someone will add such an
 * alias in the .ini option file.
 *
 * \code
 *     opt.link_aliases();
 * \endcode
 *
 * You can call this function any number of times. So if you add yet
 * more dynamic options at a later time, just make sure to call it
 * again in case aliases were added.
 *
 * \section parse Parse the Arguments
 *
 * Finally, you want to call the following functions in that order to
 * parse the data from configuration files, the environment variable,
 * and the list of command line arguments:
 *
 * \code
 *     opt.parse_configuration_files();
 *     opt.parse_environment_variable();
 *     opt.parse_arguments(argc, argv);
 * \endcode
 *
 * The order is important because the last command line option found is
 * the one kept. So if the same argument is found in the configuration
 * file, the environment variable and the command line, the one on the
 * command line is kept. In most cases it makes no difference for standalone
 * flags, but arguments that expect a parameter will be changed to the last
 * specified value.
 *
 * If you want to determine the configuration filenames, you may use the
 * process_configuration_file() function directly instead of the
 * parse_configuration_files() function. This also gives you the ability
 * to test whether a configuration file was indeed read.
 *
 * Note that the parse_arguments() last parameter (only_environment_variable)
 * is expected to be left along when you call it with `argc` and `argv`.
 *
 * If you just have a string instead of an `argv` variable, call the
 * parse_string() function instead. It will transform your string in an
 * array of arguments and then call the parse_arguments() for you.
 *
 * \important
 * Note that the program name does not get defined until you call the
 * parse_program_name() function since that information comes from the
 * first arguments of your command line which we do not get on
 * construction in this case.
 *
 * \param[in] opt_env  The list of options that your program supports.
 *
 * \sa link_aliases()
 * \sa parse_arguments()
 * \sa parse_configuration_files()
 * \sa parse_environment_variable()
 * \sa parse_string()
 * \sa process_configuration_file()
 */
getopt::getopt(options_environment const & opt_env)
    : f_options(std::make_shared<option_info>("root"))
{
    if(&opt_env == nullptr)
    {
        throw getopt_exception_logic("opt_env pointer cannot be nullptr");
    }

    f_options_environment = opt_env;

    parse_options_info(f_options_environment.f_options, false);
    parse_options_from_file();
    if((f_options_environment.f_environment_flags & GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS) != 0)
    {
        parse_options_info(g_system_options, true);
    }
}


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
        throw getopt_exception_logic("argv pointer cannot be nullptr");
    }
    if(&opt_env == nullptr)
    {
        throw getopt_exception_logic("opt_env pointer cannot be nullptr");
    }

    f_options_environment = opt_env;

    parse_program_name(argv);
    parse_options_info(f_options_environment.f_options, false);
    parse_options_from_file();
    if((f_options_environment.f_environment_flags & GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS) != 0)
    {
        parse_options_info(g_system_options, true);
    }

    if(f_options->get_children().empty())
    {
        throw getopt_exception_logic("an empty list of options is not legal, you must defined at least one (i.e. --version, --help...)");
    }

    link_aliases();

    parse_configuration_files();
    parse_environment_variable();
    parse_arguments(argc, argv, false);
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
        else if(*s == '"'
             || *s == '\'')
        {
            // support quotations and remove them from the argument
            char const quote(*s++);
            while(*s != '\0'
               && *s != quote)
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
                    std::string option_name(argv[i] + 2);
                    std::string option_value;
                    std::string::size_type const pos(option_name.find('='));
                    if(pos != std::string::npos)
                    {
                        if(pos == 0)
                        {
                            log << log_level_t::error
                                << "name missing in \""
                                << argv[i]
                                << "\"."
                                << end;
                            break;
                        }

                        option_value = option_name.substr(pos + 1);
                        option_name.resize(pos);
                    }
                    option_info::pointer_t opt(get_option(option_name));
                    if(opt == nullptr)
                    {
                        log << log_level_t::error
                            << "option --"
                            << option_name
                            << " is not supported."
                            << end;
                        break;
                    }
                    if(only_environment_variable)
                    {
                        if(!opt->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                        {
                            log << log_level_t::error
                                << "option --"
                                << option_name
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
                                << "option --"
                                << option_name
                                << " is not supported on the command line."
                                << end;
                            break;
                        }
                    }
                    if(pos != std::string::npos)
                    {
                        // the user specified a value after an equal sign
                        //
                        add_option_from_string(opt, option_value, std::string());
                    }
                    else
                    {
                        add_options(opt, i, argc, argv);
                    }
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


/** \brief Return the alias if there is one.
 *
 * This function returns the input \p opt parameter unless it is an
 * alias in which case the destination alias option is returned instead.
 *
 * \param[in] opt  The option for which an alias is desired.
 *
 * \return \p opt unless it is an alias in which case
 *         opt->get_alias_destination() is returned.
 *
 * \sa option_info::get_alias()
 */
option_info::pointer_t getopt::get_alias_destination(option_info::pointer_t opt) const
{
    if(opt != nullptr
    && opt->has_flag(GETOPT_FLAG_ALIAS))
    {
        opt = opt->get_alias_destination();
        if(opt == nullptr)
        {
            throw getopt_exception_undefined("getopt::get_alias_destination(): alias is missing. Did you call link_aliases()?");
        }
    }

    return opt;
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
 * By default the function returns the final option. That is, if the
 * named option is an alias, the destination option is returned, not
 * the alias. This way the rest of the code is much simpler. You may
 * get the exact option, even if it is aliased, by setting the
 * \p exact_option parameter to true. It is really rare that you
 * would need to do so, though.
 *
 * \param[in] name  The name of the option to retrieve.
 * \param[in] exact_option  Return the exact option, not its alias.
 *
 * \return The pointer to the named option or nullptr if not found.
 */
option_info::pointer_t getopt::get_option(std::string const & name, bool exact_option) const
{
    // we need a special case when looking for the default option
    // because the name may not be "--" in the option table
    // (i.e. you could call your default option "filenames" for
    // example.)
    //
    option_info::pointer_t opt;

    if(name.length() == 2
    && name[0] == '-'
    && name[1] == '-')
    {
        opt = f_default_option;
    }
    else if(name.length() == 1)
    {
        opt = f_options->get_child(name[0]);
    }
    else
    {
        opt = f_options->get_child(name);
    }

    return exact_option ? opt : get_alias_destination(opt);
}


/** \brief Get an option using its short name.
 *
 * This function searches for an option given its short name.
 *
 * By default the function returns the final option. That is, if the
 * named option is an alias, the destination option is returned, not
 * the alias. This way the rest of the code is much simpler. You may
 * get the exact option, even if it is aliased, by setting the
 * \p exact_option parameter to true. It is really rare that you
 * would need to do so, though.
 *
 * \param[in] short_name  The short name of the option to look for.
 * \param[in] exact_option  Return the exact option, not its alias.
 *
 * \return The pointer to the option or nullptr if not found.
 */
option_info::pointer_t getopt::get_option(short_name_t short_name, bool exact_option) const
{
    option_info::pointer_t opt(f_options->get_child(short_name));
    return exact_option ? opt : get_alias_destination(opt);
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


/** \brief Add an option with a value string.
 *
 * This function accepts a string as the value. If the option accepts
 * multiple values, then the function makes use of the set_multiple_value()
 * function of the option_info class. This will break the option up in
 * multiple value if possible.
 *
 * \param[in] opt  The option receiving a value.
 * \param[in] value  The value to assign this option.
 * \param[in] filename  The name of a configuration file is the option was
 *                      read from such.
 */
void getopt::add_option_from_string(option_info::pointer_t opt, std::string const & value, std::string const & filename)
{
    // is the value defined?
    //
    if(!value.empty())
    {
        // does the option support multiple entries?
        //
        if(opt->has_flag(GETOPT_FLAG_MULTIPLE))
        {
            opt->set_multiple_value(value);
        }
        else
        {
            opt->set_value(0, value);
        }

        return;
    }

    // does the option require a value when used?
    //
    if(opt->has_flag(GETOPT_FLAG_REQUIRED))
    {
        log << log_level_t::error
            << "option "
            << (filename.empty()
                    ? "--" + opt->get_name()
                    : "\"" + boost::replace_all_copy(opt->get_name(), "-", "_") + "\"")
            << " must be given a value"
            << (filename.empty()
                ? std::string()
                : " in configuration file \""
                    + filename
                    + "\"")
            << "."
            << end;
        return;
    }

    // accept an empty value otherwise
    //
    opt->set_value(0, value);
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
