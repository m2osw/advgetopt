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
#include    "advgetopt/exception.h"


// cppthread lib
//
#include    <cppthread/log.h>


// libutf8 lib
//
#include    <libutf8/iterator.h>


// boost lib
//
#include    <boost/algorithm/string/replace.hpp>


// last include
//
#include    <snapdev/poison.h>





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
 * Print out the usage() with most of the command line arguments.
 *
 * \li '--long-help'
 *
 * Print all the command line arguments with usage().
 *
 * The long help is is only added if the list of options include at least
 * one group flag (GETOPT_FLAG_SHOW_GROUP1 or GETOPT_FLAG_SHOW_GROUP2).
 * See the getopt::parse_options_from_group_names() in advgetopt_usage.cpp.
 *
 * \li '--\<name>-help'
 *
 * Print the help from the group named \<name>.
 *
 * These command line options are added only when groups are defined.
 *
 * \li `--version`
 *
 * Print out the version.
 *
 * \li `--copyright`
 *
 * Print out the copyright notice.
 *
 * \li `--license`
 *
 * Print out the license notice.
 *
 * \li `--build-date`
 *
 * Print out the build time and date.
 *
 * \li `--environment-variable-name`
 *
 * Print out the build time and date.
 *
 * \li `--configuration-filenames`
 *
 * Print out the list of configuration file names that the system checks
 * for configuration data.
 *
 * \li `--path-to-option-definitions`
 *
 * Print out the path to files which define options for this tool.
 *
 * \li `--source-option-sources`
 *
 * Print out all the options and their sources. This shows you where a
 * value come from: command line, environment variable, configuration file,
 * etc.
 *
 * \todo
 * Add a `--config` option to allow the user to name one specific
 * configuration file to use with an executable.
 */
option const g_system_options[] =
{
    define_option(
          Name("build-date")
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print out the time and date when %p was built and exit.")
    ),
    define_option(
          Name("compiler-version")
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print the version of the compiler used to compile the advgetopt library.")
    ),
    define_option(
          Name("configuration-filenames")
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print out the list of configuration files checked out by this tool.")
    ),
    define_option(
          Name("copyright")
        , ShortName('C')
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print out the copyright of %p and exit.")
    ),
    define_option(
          Name("environment-variable-name")
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print out the name of the environment variable supported by %p (if any.)")
    ),
    define_option(
          Name("has-sanitizer")
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print whether the advgetopt was compiled with the sanitizer extension.")
    ),
    define_option(
          Name("help")
        , ShortName('h')
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS
                                       , GETOPT_FLAG_SHOW_USAGE_ON_ERROR>())
        , Help("print out this help screen and exit.")
    ),
    define_option(
          Name("license")
        , ShortName('L')
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print out the license of %p and exit.")
    ),
    define_option(
          Name("path-to-option-definitions")
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print out the path to the option definitions.")
    ),
    define_option(
          Name("show-option-sources")
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("parse all the options and then print out the source of each value and each override.")
    ),
    define_option(
          Name("version")
        , ShortName('V')
        , Flags(standalone_command_flags<GETOPT_FLAG_GROUP_COMMANDS>())
        , Help("print out the version of %p and exit.")
    ),
    end_options()
};


/** \brief Optional list of options.
 *
 * This optional list of options is added only when the
 * f_configuration_filename parameter has a valid filename.
 *
 * The following are the currently added options:
 *
 * \li `--config-dir`
 *
 * This option allows for adding more configuration directories.
 * These work the same way as directories defined in the
 * f_configuration_directories.
 */
option const g_if_configuration_filename_system_options[] =
{
    define_option(
          Name("config-dir")
        , Flags(any_flags<GETOPT_FLAG_COMMAND_LINE
                        , GETOPT_FLAG_ENVIRONMENT_VARIABLE
                        , GETOPT_FLAG_REQUIRED
                        , GETOPT_FLAG_MULTIPLE
                        , GETOPT_FLAG_GROUP_OPTIONS>())
        , Help("add one or more configuration directory paths to search for configuration files.")
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



/** \brief Check for a "--show-option-sources" flag.
 *
 * When this flag is defined, we turn on the trace mode in the option_info
 * class (which is a global flag). That way we can honor the
 * "--show-option-sources" when we are done parsing the configuration files,
 * environment variable, and command line.
 *
 * \param[in] argc  The number of items in the \p argv array.
 * \param[in] argv  The arguments.
 */
void check_for_show_sources(int argc, char * argv[])
{
    static bool found = false;

    if(!found && argv != nullptr)
    {
        for(int idx(1); idx < argc; ++idx)
        {
            if(strcmp(argv[idx], "--show-option-sources") == 0)
            {
                found = true;
                option_info::set_trace_sources(true);
            }
        }
    }
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
 * array is defined as a set of option structures.
 *
 * The last option must be an end_options(). It has its f_flags set to
 * GETOPT_FLAG_END and all the other parameters are set to zero (i.e. no
 * name, no short name, etc.)
 *
 * Note that with the newer version of the library, you are not expected
 * to manually define an array of options. Instead, you want to use the
 * C++ functions such as Name(), ShortName(), Flags(), etc. These functions
 * are capable of verifying that at least some of the values are valid at
 * compile time.
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
 *     opt.parse_arguments(argc, argv, option_source_t::SOURCE_COMMAND_LINE);
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
 * \attention
 * Note that the program name does not get defined until you call the
 * parse_program_name() function since that information comes from the
 * first arguments of your command line which we do not get on
 * construction in this case.
 *
 * \attention
 * Since the arguments are not known to the getopt system yet, the
 * GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS is not used in
 * this case.
 *
 * \param[in] opt_env  The list of options that your program supports.
 *
 * \sa link_aliases()
 * \sa parse_arguments()
 * \sa parse_configuration_files()
 * \sa parse_environment_variable()
 * \sa parse_string()
 * \sa process_configuration_file()
 *
 * \sa initialize_parser()
 * \sa finish_parsing()
 */
getopt::getopt(options_environment const & opt_env)
    : f_variables(std::make_shared<variables>())
{
    initialize_parser(opt_env);
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
 * \exception getopt_exit
 * This function calls finish_parsing() which may throw this exception.
 * See that function for details.
 *
 * \param[in] opt_env  The list of options that your program supports.
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  An array of strings representing arguments.
 *
 * \sa initialize_parser()
 * \sa finish_parsing()
 */
getopt::getopt(options_environment const & opt_env
             , int argc
             , char * argv[])
{
    // TODO: I do not think that this check_for_show_sources() is required
    //       because we also do it in finish_parsing()
    //
    check_for_show_sources(argc, argv);

    initialize_parser(opt_env);
    finish_parsing(argc, argv);
}


/** \brief Initialize the parser.
 *
 * This function is called from the two constructors. It initializes the
 * basic options from the user definitions, the file when there is one,
 * the group names, and if allowed the system command line options.
 *
 * This is enough to then parse arguments or configuration files, although
 * in most cases this is used to allow for additional environment options
 * to be inserted before calling the finish_parsing() function.
 *
 * \param[in] opt_env  The list of options that your program supports.
 */
void getopt::initialize_parser(options_environment const & opt_env)
{
    f_options_environment = opt_env;

    parse_options_info(f_options_environment.f_options, false);
    parse_options_from_file();
    parse_options_from_group_names();
    if(has_flag(GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS | GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS))
    {
        parse_options_info(g_system_options, true);
        if(f_options_environment.f_configuration_filename != nullptr
        && *f_options_environment.f_configuration_filename != '\0')
        {
            // add the "--config-dir <path> ..." option
            //
            parse_options_info(g_if_configuration_filename_system_options, true);
        }
    }

    define_environment_variable_data();
}


/** \brief Actually parse everything.
 *
 * This function allows you to run the second half of the initialization
 * process. We've broken this process up in two, so you can initialize
 * a getopt object, add some other options, then finish up the
 * initialization process by calling this function.
 *
 * The command line arguments, configuration files.
 *
 * \exception getopt_exit
 * If the GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS is set and
 * a system command was specified on the command, such as --help or
 * --version, then that command is run and the function throws this
 * exception.
 *
 * \param[in] argc  The number of arguments in argv.
 * \param[in] argv  An array of strings representing arguments.
 */
void getopt::finish_parsing(int argc, char * argv[])
{
    if(argv == nullptr)
    {
        throw getopt_logic_error("argv pointer cannot be nullptr");
    }

    check_for_show_sources(argc, argv);

    parse_program_name(argv);
    if(f_options_by_name.empty())
    {
        throw getopt_logic_error("an empty list of options is not legal, you must defined at least one (i.e. --version, --help...)");
    }

    link_aliases();

    parse_configuration_files(argc, argv);
    f_parsed = false;
    parse_environment_variable();
    f_parsed = false;
    parse_arguments(argc, argv, option_source_t::SOURCE_COMMAND_LINE, false);

    if(has_flag(GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS))
    {
        flag_t const result(process_system_options(std::cout));
        if((result & SYSTEM_OPTION_COMMANDS_MASK) != 0)
        {
            throw getopt_exit("system command processed.", 0);
        }
    }

    if(cppthread::log.get_errors() != 0)
    {
        throw getopt_exit("errors were found on your command line, environment variable, or configuration file.", 1);
    }
}


/** \brief Verify that the parser is done.
 *
 * This function ensures that the parser is done. If the parser is not yet
 * done, then the function raises an exception. This allows me to detect
 * that I am trying to access a parameter before the whole parsing process
 * is done (i.e. I had a call to is_defined("config-dir") happening in the
 * configuration handling way before the environment variables and command
 * line arguments were parsed, that would never work!)
 *
 * \exception getopt_initialization
 * This exception is raised if the parser is not done yet.
 */
void getopt::is_parsed() const
{
    if(!f_parsed
    && (f_options_environment.f_environment_flags & GETOPT_ENVIRONMENT_FLAG_AUTO_DONE) == 0)
    {
        throw getopt_initialization(
                "function called too soon, parser is not done yet"
                " (i.e. is_defined(), get_string(), get_integer(),"
                " get_double() cannot be called until the parser is done)");
    }
}


/** \brief Return a reference to the options environment.
 *
 * This function returns a reference to the options environment that
 * was passed to the constructor. This is useful to functions that
 * do not otherwise have access to that object.
 *
 * \return This getopt options environment.
 */
options_environment const & getopt::get_options_environment() const
{
    return f_options_environment;
}


/** \brief Check whether an environment flag is set or not.
 *
 * This function checks the environment flags for the specified \p flag.
 * When the flag is set, the function returns true.
 *
 * You may test multiple flags at the same time, if any one of them is set,
 * then the function returns true.
 *
 * \param[in] flag  The flag to check out.
 *
 * \return true if the flag is set.
 */
bool getopt::has_flag(flag_t flag) const
{
    return (f_options_environment.f_environment_flags & flag) != 0;
}


/** \brief Retrieve the environment variable string.
 *
 * This function retrieves the environment variable string and saves it
 * in the f_environment_variable field. This is used to parse that string
 * and add option values, and also by the configuration file loader to see
 * whether a --config-dir was used in there.
 */
void getopt::define_environment_variable_data()
{
    f_environment_variable.clear();

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

    f_environment_variable = s;
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
 *
 * \note
 * If you change the environment variable between the creation of the
 * getopt object and a call to this function, you want to call the
 * define_environment_variable_data() again to make sure it can be
 * parsed.
 */
void getopt::parse_environment_variable()
{
    // first test the global environment variable
    //
    if(!f_environment_variable.empty())
    {
        parse_string(
                  f_environment_variable
                , option_source_t::SOURCE_ENVIRONMENT_VARIABLE
                , true);
    }

    // second check each option environment variable
    //
    for(auto const & opt : f_options_by_name)
    {
        std::string const name(opt.second->get_environment_variable_name());
        if(!name.empty())
        {
            // make sure the value is defined, if empty, ignore altogether
            //
            std::string const value(opt.second->get_environment_variable_value(f_options_environment.f_environment_variable_intro));
            if(!value.empty())
            {
                add_option_from_string(
                          opt.second
                        , value
                        , std::string()
                        , option_source_t::SOURCE_ENVIRONMENT_VARIABLE);
            }
        }
    }

    f_parsed = true;
}


/** \brief Parse a string similar to a command line argument.
 *
 * This function parses a line of command line arguments from a string.
 * Especially, it is used to parse the environment variable which is
 * a string of arguments.
 *
 * This can be used to parse the command line string as received under
 * MS-Windows (i.e. an unparsed one long string of arguments, where
 * you also need to do the glob() calls yourself.)
 *
 * This function actually transforms the input string in an array of
 * strings and then calls the parse_arguments() function.
 *
 * \note
 * The input allows for an empty string in which case pretty much nothing
 * happens.
 *
 * \param[in] str  The string that is going to be parsed.
 * \param[in] source  Where the value comes from.
 * \param[in] only_environment_variable  Whether only options marked with
 *            the GETOPT_FLAG_ENVIRONMENT_VARIABLE flag are accepted.
 */
void getopt::parse_string(
          std::string const & str
        , option_source_t source
        , bool only_environment_variable)
{
    string_list_t args(split_environment(str));
    if(args.empty())
    {
        // nothing extra to do
        //
        return;
    }

    // TODO: expand the arguments that include unquoted '*', '?', '[...]'
    //       (note that we remove the quotes at the moment so we'd have
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
    parse_arguments(
          static_cast<int>(sub_argv.size() - 1)
        , sub_argv.data()
        , source
        , only_environment_variable);
}


/** \brief Transform a string in an array of arguments.
 *
 * This function is used to transform a string to an array of arguments
 * that can then be used with the parse_arguments() function.
 *
 * For example, it is used to parse the environment variable string.
 *
 * \note
 * The input string may include quotes. These will be removed. There is
 * currently no support for the backslash character.
 *
 * \param[in] environment  The string to be split in arguments.
 *
 * \return An array of strings.
 */
string_list_t getopt::split_environment(std::string const & environment)
{
    // this is exactly like the command line only in an environment variable
    // so parse the parameters just like the shell
    //
    string_list_t args;
    std::string a;
    char const * s(environment.c_str());
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
            //
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

    return args;
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
 * \param[in] source  Where the value comes from.
 * \param[in] only_environment_variable  Accept command line arguments (false)
 *            or environment variable arguments (true).
 */
void getopt::parse_arguments(
          int argc
        , char * argv[]
        , option_source_t source
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
                        cppthread::log << cppthread::log_level_t::error
                                       << "no default options defined; thus \"--\" is not accepted by this program."
                                       << cppthread::end;
                        break;
                    }

                    if(only_environment_variable)
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "option \"--\" is not supported in the environment variable."
                                           << cppthread::end;
                            break;
                        }
                    }
                    else
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_COMMAND_LINE))
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "option \"--\" is not supported on the command line."
                                           << cppthread::end;
                            break;
                        }
                    }

                    // in this case we do NOT test whether an argument uses
                    // a dash (-) we take them all as default options
                    //
                    while(i + 1 < argc)
                    {
                        ++i;
                        f_default_option->add_value(argv[i], source);
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
                            cppthread::log << cppthread::log_level_t::error
                                           << "name missing in \""
                                           << argv[i]
                                           << "\"."
                                           << cppthread::end;
                            break;
                        }

                        option_value = option_name.substr(pos + 1);
                        option_name.resize(pos);
                    }
                    option_info::pointer_t opt(get_option(option_name));
                    if(opt == nullptr)
                    {
                        cppthread::log << cppthread::log_level_t::error
                                       << "option \"--"
                                       << option_name
                                       << "\" is not supported."
                                       << cppthread::end;
                        break;
                    }
                    if(only_environment_variable)
                    {
                        if(!opt->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "option \"--"
                                           << option_name
                                           << "\" is not supported in the environment variable."
                                           << cppthread::end;
                            break;
                        }
                    }
                    else
                    {
                        if(!opt->has_flag(GETOPT_FLAG_COMMAND_LINE))
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "option \"--"
                                           << option_name
                                           << "\" is not supported on the command line."
                                           << cppthread::end;
                            break;
                        }
                    }
                    if(pos != std::string::npos)
                    {
                        // the user specified a value after an equal sign
                        //
                        add_option_from_string(opt, option_value, std::string(), source);
                    }
                    else
                    {
                        add_options(opt, i, argc, argv, source);
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
                        cppthread::log << cppthread::log_level_t::error
                                       << "no default options defined; thus \"-\" is not accepted by this program."
                                       << cppthread::end;
                        break;
                    }
                    if(only_environment_variable)
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "option \"-\" is not supported in the environment variable."
                                           << cppthread::end;
                            break;
                        }
                    }
                    else
                    {
                        if(!f_default_option->has_flag(GETOPT_FLAG_COMMAND_LINE))
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "option \"-\" is not supported on the command line."
                                           << cppthread::end;
                            break;
                        }
                    }

                    // this is similar to a default option by itself
                    //
                    f_default_option->add_value(argv[i], source);
                }
                else
                {
                    // short option(s)
                    //
                    // i gets incremented by add_options() so we have to
                    // keep a copy in `k`
                    //
                    std::string const short_args_string(argv[i] + 1);
                    for(libutf8::utf8_iterator short_args(short_args_string)
                      ; short_args != short_args_string.end()
                      ; ++short_args)
                    {
                        option_info::pointer_t opt(get_option(*short_args));
                        if(opt == nullptr)
                        {
                            cppthread::log << cppthread::log_level_t::error
                                           << "option \"-"
                                           << short_name_to_string(*short_args)
                                           << "\" is not supported."
                                           << cppthread::end;
                            break;
                        }
                        if(only_environment_variable)
                        {
                            if(!opt->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                            {
                                cppthread::log << cppthread::log_level_t::error
                                               << "option \"-"
                                               << short_name_to_string(*short_args)
                                               << "\" is not supported in the environment variable."
                                               << cppthread::end;
                                break;
                            }
                        }
                        else
                        {
                            if(!opt->has_flag(GETOPT_FLAG_COMMAND_LINE))
                            {
                                cppthread::log << cppthread::log_level_t::error
                                               << "option \"-"
                                               << short_name_to_string(*short_args)
                                               << "\" is not supported on the command line."
                                               << cppthread::end;
                                break;
                            }
                        }
                        add_options(opt, i, argc, argv, source);
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
                cppthread::log << cppthread::log_level_t::error
                               << "no default options defined; we do not know what to do of \""
                               << argv[i]
                               << "\"; standalone parameters are not accepted by this program."
                               << cppthread::end;
                break;
            }
            if(only_environment_variable)
            {
                if(!f_default_option->has_flag(GETOPT_FLAG_ENVIRONMENT_VARIABLE))
                {
                    cppthread::log << cppthread::log_level_t::error
                                   << "default options are not supported in the environment variable."
                                   << cppthread::end;
                    break;
                }
            }
            else
            {
                if(!f_default_option->has_flag(GETOPT_FLAG_COMMAND_LINE))
                {
                    cppthread::log << cppthread::log_level_t::error
                                   << "default options are not supported on the command line."
                                   << cppthread::end;
                    break;
                }
            }
            f_default_option->add_value(argv[i], source);
        }
    }

    f_parsed = true;
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
            throw getopt_undefined("getopt::get_alias_destination(): alias is missing. Did you call link_aliases()?");
        }
    }

    return opt;
}


/** \brief Retrieve the complete list of options.
 *
 * Applications that let their users enter dynamically options need to
 * have access to the resulting list of options which may not otherwise
 * be known.
 *
 * \return The map of options indexed by name.
 *
 * \sa parse_options_from_file()
 */
option_info::map_by_name_t const & getopt::get_options() const
{
    return f_options_by_name;
}


/** \brief Retrieve an option by name.
 *
 * This function retrieves an option by name. The function handles the
 * special case of the default option. This means "--" can always be
 * used to access the default option, whatever the name given to that
 * option in the declaration of your options.
 *
 * Of course, if no default is defined, then "--" returns a nullptr.
 *
 * By default the function returns the final option. That is, if the
 * named option is an alias, the destination option is returned, not
 * the alias. This way the rest of the code is much simpler. You may
 * get the exact option, even if it is aliased, by setting the
 * \p exact_option parameter to true. It is really rare that you
 * would need to do so.
 *
 * \note
 * The \p name parameter could be an empty string to represent the default
 * value (I think that's what that was for, now you use "--"). I've added
 * the throw in this function to catch that invalid use, which either means
 * you tried to check something with an invalid name or are expected to use
 * the "--".
 *
 * \param[in] name  The name of the option to retrieve.
 * \param[in] exact_option  Return the exact option, not its alias.
 *
 * \return The pointer to the named option or nullptr if not found.
 */
option_info::pointer_t getopt::get_option(std::string const & name, bool exact_option) const
{
    option_info::pointer_t opt;

    if(name.empty())
    {
        // see \note section in doxy above about this
        //
        throw getopt_invalid_parameter("get_option() `name` argument cannot be empty.");
    }

    std::string const n(boost::replace_all_copy(name, "_", "-"));

    // we need this special case when looking for the default option
    // because the name may not be "--" in the option table
    // (i.e. you could call your default option "filenames" for example.)
    //
    if(n.length() == 2
    && n[0] == '-'
    && n[1] == '-')
    {
        opt = f_default_option;
    }
    else
    {
        short_name_t short_name(string_to_short_name(n));
        if(short_name != NO_SHORT_NAME)
        {
            auto it(f_options_by_short_name.find(short_name));
            if(it != f_options_by_short_name.end())
            {
                opt = it->second;
            }
        }
        else
        {
            auto it(f_options_by_name.find(n));
            if(it != f_options_by_name.end())
            {
                opt = it->second;
            }
        }
    }

    return exact_option
            ? opt
            : get_alias_destination(opt);
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
    auto it(f_options_by_short_name.find(short_name));
    if(it == f_options_by_short_name.end())
    {
        return option_info::pointer_t();
    }

    return exact_option
                ? it->second
                : get_alias_destination(it->second);
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
 * \param[in] source  Where the value comes from.
 */
void getopt::add_options(
          option_info::pointer_t opt
        , int & i
        , int argc
        , char ** argv
        , option_source_t source)
{
    if(opt->has_flag(GETOPT_FLAG_FLAG))
    {
        opt->add_value(opt->get_default(), source);
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
                    opt->add_value(argv[i], source);
                }
                while(i + 1 < argc && !is_arg(argv[i + 1]));
            }
            else
            {
                ++i;
                opt->add_value(argv[i], source);
            }
        }
        else
        {
            if(opt->has_flag(GETOPT_FLAG_REQUIRED))
            {
                cppthread::log << cppthread::log_level_t::error
                               << "option --"
                               << opt->get_name()
                               << " expects an argument."
                               << cppthread::end;
            }
            else
            {
                // We need to set something because the value is being
                // set although no argument was specified (but that's
                // legal by this argument's definition)
                //
                opt->add_value(std::string(), source);
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
 * \param[in] filename  The name of a configuration file if the option was
 *                      read from such.
 * \param[in] source  Where the value comes from.
 */
void getopt::add_option_from_string(
          option_info::pointer_t opt
        , std::string const & value
        , std::string const & filename
        , option_source_t source)
{
    // is the value defined?
    //
    if(!value.empty())
    {
        if(opt->has_flag(GETOPT_FLAG_FLAG))
        {
            cppthread::log << cppthread::log_level_t::error
                           << "option "
                           << (filename.empty()
                                   ? "--" + opt->get_name()
                                   : "\"" + boost::replace_all_copy(opt->get_name(), "-", "_") + "\"")
                           << " cannot be given a value"
                           << (filename.empty()
                               ? std::string()
                               : " in configuration file \""
                                   + filename
                                   + "\"")
                           << "."
                           << cppthread::end;
            return;
        }

        // does the option support multiple entries?
        //
        if(opt->has_flag(GETOPT_FLAG_MULTIPLE))
        {
            opt->set_multiple_values(value, source);
        }
        else
        {
            opt->set_value(0, value, source);
        }

        return;
    }

    // does the option require a value when used?
    //
    if(opt->has_flag(GETOPT_FLAG_REQUIRED))
    {
        cppthread::log << cppthread::log_level_t::error
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
                       << cppthread::end;
        return;
    }

    // accept an empty value otherwise
    //
    opt->set_value(0, value, source);
}


/** \brief Retrieve a pointer to the variables defined in the getopt object.
 *
 * This function retrieves the variables defined in the getopt object.
 * This is useful if you want to support those variables within
 * configuration files that you read later on.
 *
 * \note
 * This pointer is never a nullptr.
 *
 * \return A pointer to the getopt variables.
 */
variables::pointer_t getopt::get_variables() const
{
    return f_variables;
}



} // namespace advgetopt
// vim: ts=4 sw=4 et
