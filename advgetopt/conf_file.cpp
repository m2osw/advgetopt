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


/** \file
 * \brief Implementation of the option_info class.
 *
 * This is the implementation of the class used to load and save
 * configuration files.
 */

// self
//
#include    "advgetopt/conf_file.h"


// advgetopt lib
//
#include    "advgetopt/exception.h"
#include    "advgetopt/utils.h"


// snapdev lib
//
#include    <snapdev/safe_variable.h>
#include    <snapdev/tokenize_string.h>


// cppthread lib
//
#include    <cppthread/guard.h>
#include    <cppthread/log.h>
#include    <cppthread/mutex.h>


// boost lib
//
#include    <boost/algorithm/string/join.hpp>
#include    <boost/algorithm/string/replace.hpp>


// C++ lib
//
#include    <algorithm>
#include    <fstream>


// C lib
//
#include    <sys/stat.h>


// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{


// from utils.cpp
//
// (it's here because we do not want to make cppthread public in
// out header files--we could have an advgetopt_private.h, though)
//
cppthread::mutex &  get_global_mutex();



/** \brief Private conf_file data.
 *
 * The conf_file has a few globals used to cache configuration files.
 * Since it has to work in a multi-thread environment, we also have
 * a mutex.
 */
namespace
{



/** \brief A map of configuration files.
 *
 * This typedef defines a type used to hold all the configuration files
 * that were loaded so far.
 *
 * The map is indexed by a string representing the full path to the
 * configuration file.
 *
 * The value is a shared pointer to configuration file. Since we may
 * share that data between multiple users, it made sense to force you
 * to use a configuration file smart pointer. Note, though, that we
 * never destroy the pointer until we quit (i.e. you cannot force a
 * re-load of the configuration file. Changes that happen in memory
 * are visible to all users, but changes to the actual configuration
 * file are complete invisible to use.)
 */
typedef std::map<std::string, conf_file::pointer_t>     conf_file_map_t;


/** \brief The configuration files.
 *
 * This global defines a list of configuration files indexed by
 * filename (full path, but not the URL, just a path.)
 *
 * Whenever a configuration file is being retrieved with the
 * conf_file::get_conf_file() function, it is first searched
 * in this map. If it exists in the map, that version gets
 * used (if the URL of the two setups match one to one.)
 * If there is no such file in the map, then a new one is
 * created by loading the corresponding file.
 */
conf_file_map_t     g_conf_files = conf_file_map_t();


} // no name namespace





/** \brief Initialize the file setup object.
 *
 * This constructor initializes the setup object which can later be used
 * to search for an existing conf_file or creating a new conf_file.
 *
 * The setup holds the various parameters used to know how to load a
 * configuration file in memory. The parameters include
 *
 * \li \p filename -- the name of the file to read as a configuration file.
 * \li \p line_continuation -- how lines in the files are being read; in
 * most cases a line in a text file ends when a newline character (`\\n`)
 * is found; this parameter allows for lines that span (continue) on
 * multiple text lines. Only one type of continuation or no continue
 * (a.k.a. "single line") can be used per file.
 * \li \p assignment_operator -- the character(s) accepted between the
 * name of a variable and its value; by default this is the equal sign
 * (`=`). Multiple operators can be accepted.
 * \li \p comment -- how comments are introduced when supported. Multiple
 * introducers can be accepted within one file. By default we accept the
 * Unix Shell (`#`) and INI file (`;`) comment introducers.
 * \li \p section_operator -- the set of characters accepted as section
 * separator. By default we accept the INI file syntax (the `[section]`
 * syntax.)
 *
 * \note
 * If the filename represent an existing file, then the name is going to
 * get canonicalized before it gets saved in the structure. Otherwise it
 * gets saved as is.
 *
 * \param[in] filename  A valid filename.
 * \param[in] line_continue  One of the line_continuation_t values.
 * \param[in] assignment_operator  A set of assignment operator flags.
 * \param[in] comment  A set of comment flags.
 * \param[in] section_operator  A set of section operator flags.
 */
conf_file_setup::conf_file_setup(
          std::string const & filename
        , line_continuation_t line_continuation
        , assignment_operator_t assignment_operator
        , comment_t comment
        , section_operator_t section_operator)
    : f_original_filename(filename)
    , f_line_continuation(line_continuation)
    , f_assignment_operator(assignment_operator == 0
                ? ASSIGNMENT_OPERATOR_EQUAL
                : assignment_operator)
    , f_comment(comment)
    , f_section_operator(section_operator)
{
    if(filename.empty())
    {
        throw getopt_invalid("trying to load a configuration file using an empty filename.");
    }

    std::unique_ptr<char, decltype(&::free)> fn(realpath(filename.c_str(), nullptr), &::free);
    if(fn != nullptr)
    {
        f_filename = fn.get();
    }
    else
    {
        f_filename = filename;
    }
}


/** \brief Check whether the setup is considered valid.
 *
 * This function is used to check whether the conf_file_setup is valid or
 * not. It is valid when everything is in order, which at this point means
 * the filename is not empty.
 *
 * All the other parameters are always viewed as being valid.
 *
 * \warning
 * The is_valid() always returns true at this time. We always save the
 * filename. I'm not totally sure why I wanted to not have a way to get
 * a valid configuration file by viewing a non-existing file as the same
 * as an empty file. Now that's what happens.
 *
 * \return true if the conf_file_setup is considered valid.
 */
bool conf_file_setup::is_valid() const
{
    return !f_filename.empty();
}


/** \brief Get the original filename.
 *
 * When creating a new conf_file_setup, you have to specify a filename.
 * This function returns that string exactly, without canonicalization.
 *
 * \return The filename as specified at the time of construction.
 *
 * \sa get_filename()
 */
std::string const & conf_file_setup::get_original_filename() const
{
    return f_original_filename;
}


/** \brief Get the filename.
 *
 * When creating a new conf_file_setup, you have to specify a filename.
 * This function returns that filename after it was canonicalized by
 * the constructor.
 *
 * The canonicalization process computes the full path to the real
 * file. If such does not exist then no filename is defined, so this
 * function may return an empty string.
 *
 * \return The filename or an empty string if the realpath() could not
 *         be calculated.
 *
 * \sa get_original_filename()
 */
std::string const & conf_file_setup::get_filename() const
{
    return f_filename;
}


/** \brief Get the line continuation setting.
 *
 * This function returns the line continuation for this setup.
 *
 * This parameter is not a set of flags. We only support one type of
 * line continuation per file. Many continuations could be contradictory
 * if used simultaneously.
 *
 * The continuation setting is one of the following:
 *
 * \li line_continuation_t::single_line -- no continuation support; any
 * definition must be on one single line.
 * \li line_continuation_t::rfc_822 -- like email/HTTP, whitespace at
 * the start of the next line means that the current line continues there;
 * those whitespaces get removed from the value so if you want a space
 * between two lines, make sure to finish the current line with a space.
 * \li line_continuation_t::msdos -- `&` at end of the line.
 * \li line_continuation_t::unix -- `\` at end of the line.
 * \li line_continuation_t::fortran -- `&` at the start of the next line;
 * there cannot be any spaces, the `&` has to be the very first character.
 * \li line_continuation_t::semicolon -- `;` ends the _line_; when reading
 * a line with this continuation mode, the reader stops only when it finds
 * the `;` or EOF (also if a comment is found.)
 *
 * \return a line continuation mode.
 */
line_continuation_t conf_file_setup::get_line_continuation() const
{
    return f_line_continuation;
}


/** \brief Get the accepted assignment operators.
 *
 * This function returns the set of flags describing the list of
 * accepted operators one can use to do assignments.
 *
 * Right now we support the follow:
 *
 * \li ASSIGNMENT_OPERATOR_EQUAL -- the equal (`=`) character, like in
 * most Unix configuration files and shell scripts.
 * \li ASSIGNMENT_OPERATOR_COLON -- the colon (`:`) character, like in
 * email and HTTP headers.
 * \li ASSIGNMENT_OPERATOR_SPACE -- the space (` `) character; this is
 * less used, but many Unix configuration files still use this scheme.
 *
 * \todo
 * Add support for additional operators such as:
 * \todo
 * \li `+=` -- append data
 * \li `?=` -- set to this value if not yet set
 *
 * \return The set of accepted assignment operators.
 *
 * \sa is_assignment_operator()
 */
assignment_operator_t conf_file_setup::get_assignment_operator() const
{
    return f_assignment_operator;
}


/** Get the comment flags.
 *
 * This function returns the comment flags. These describe which type
 * of comments are supported in this configuration file.
 *
 * Currently we support:
 *
 * \li COMMENT_INI -- INI file like comments, these are introduced with
 * a semi-colon (`;`) and end with a newline.
 * \li COMMENT_SHELL -- Unix shell like comments, these are introduced
 * with a hash (`#`) and end with a newline.
 * \li COMMENT_CPP -- C++ like comments, these are introduced with two
 * slashes (`//`) and end with a newline.
 *
 * Right now we only support line comments. Configuration entries cannot
 * include comments. A comment character can be preceeded by spaces and
 * tabs.
 *
 * Line continuation is taken in account with comments. So the following
 * when the line continuation is set to Unix is one long comment:
 *
 * \code
 *   # line continuation works with comments \
 *   just like with any other line... because the \
 *   continuation character and the newline characters \
 *   just get removed before the get_line() function \
 *   returns...
 * \endcode
 *
 * \return The comment flags.
 *
 * \sa is_comment()
 */
comment_t conf_file_setup::get_comment() const
{
    return f_comment;
}


/** \brief Get the accepted section operators.
 *
 * This function returns the flags representing which of the
 * section operators are accepted.
 *
 * We currently support the following types of sections:
 *
 * \li SECTION_OPERATOR_NONE -- no sections are accepted.
 * \li SECTION_OPERATOR_C -- the period (`.`) is viewed as a section/name
 * separator as when you access a variable member in a structure.
 * \li SECTION_OPERATOR_CPP -- the scope operator (`::`) is viewed as a
 * section/name separator; if used at the very beginning, it is viewed
 * as "global scope" and whatever other section is currently active is
 * ignored.
 * \li SECTION_OPERATOR_BLOCK -- the configuration files can include
 * opening (`{`) and closing (`}`) curvly brackets to group parameters
 * together; a name must preceed the opening bracket, it represents
 * the section name.
 * \li SECTION_OPERATOR_INI_FILE -- like in the MS-DOS .ini files, the
 * configuration file can include square brackets to mark sections; this
 * method limits the number of section names to one level.
 *
 * \bug
 * The INI file support does not verify that a section name does not
 * itself include more sub-sections. For example, the following would
 * be three section names:
 * \bug
 * \code
 * [a::b::c]
 * var=123
 * \endcode
 * \bug
 * So in effect, the variable named `var` ends up in section `a`,
 * sub-section `b`, and sub-sub-section `c` (or section `a::b::c`.)
 * Before saving the results in the parameters, all section operators
 * get transformed to the C++ scope (`::`) operator, which is why that
 * operator used in any name ends up looking like a section separator.
 */
section_operator_t conf_file_setup::get_section_operator() const
{
    return f_section_operator;
}


/** \brief Transform the setup in a URL.
 *
 * This function transforms the configuration file setup in a unique URL.
 * This URL allows us to verify that two setup are the same so when
 * attempting to reload the same configuration file, we can make sure
 * you are attempting to do so with the same URL.
 *
 * This is because trying to read the same file with, for example, line
 * continuation set to Unix the first time and then set to MS-DOS the
 * second time would not load the same thing is either line continuation
 * was used.
 *
 * \todo
 * We should look into have a set_config_url() or have a constructor
 * which accepts a URL.
 *
 * \return The URL representing this setup.
 */
std::string conf_file_setup::get_config_url() const
{
    if(f_url.empty())
    {
        std::stringstream ss;

        ss << "file://"
           << (f_filename.empty()
                    ? "/<empty>"
                    : f_filename);

        std::vector<std::string> params;
        if(f_line_continuation != line_continuation_t::line_continuation_unix)
        {
            std::string name;
            switch(f_line_continuation)
            {
            case line_continuation_t::line_continuation_single_line:
                name = "single-line";
                break;

            case line_continuation_t::line_continuation_rfc_822:
                name = "rfc-822";
                break;

            case line_continuation_t::line_continuation_msdos:
                name = "msdos";
                break;

            // we should not ever receive this one since we don't enter
            // this block when the value is "unix"
            //
            //case line_continuation_t::line_continuation_unix:
            //    name = "unix";
            //    break;

            case line_continuation_t::line_continuation_fortran:
                name = "fortran";
                break;

            case line_continuation_t::line_continuation_semicolon:
                name = "semi-colon";
                break;

            default:
                throw getopt_logic_error("unexpected line continuation.");

            }
            params.push_back("line-continuation=" + name);
        }

        if(f_assignment_operator != ASSIGNMENT_OPERATOR_EQUAL)
        {
            std::vector<std::string> assignments;
            if((f_assignment_operator & ASSIGNMENT_OPERATOR_EQUAL) != 0)
            {
                assignments.push_back("equal");
            }
            if((f_assignment_operator & ASSIGNMENT_OPERATOR_COLON) != 0)
            {
                assignments.push_back("colon");
            }
            if((f_assignment_operator & ASSIGNMENT_OPERATOR_SPACE) != 0)
            {
                assignments.push_back("space");
            }
            if(!assignments.empty())
            {
                params.push_back("assignment-operator=" + boost::algorithm::join(assignments, ","));
            }
        }

        if(f_comment != COMMENT_INI | COMMENT_SHELL)
        {
            std::vector<std::string> comment;
            if((f_comment & COMMENT_INI) != 0)
            {
                comment.push_back("ini");
            }
            if((f_comment & COMMENT_SHELL) != 0)
            {
                comment.push_back("shell");
            }
            if((f_comment & COMMENT_CPP) != 0)
            {
                comment.push_back("cpp");
            }
            if(comment.empty())
            {
                params.push_back("comment=none");
            }
            else
            {
                params.push_back("comment=" + boost::algorithm::join(comment, ","));
            }
        }

        if(f_section_operator != SECTION_OPERATOR_INI_FILE)
        {
            std::vector<std::string> section_operator;
            if((f_section_operator & SECTION_OPERATOR_C) != 0)
            {
                section_operator.push_back("c");
            }
            if((f_section_operator & SECTION_OPERATOR_CPP) != 0)
            {
                section_operator.push_back("cpp");
            }
            if((f_section_operator & SECTION_OPERATOR_BLOCK) != 0)
            {
                section_operator.push_back("block");
            }
            if((f_section_operator & SECTION_OPERATOR_INI_FILE) != 0)
            {
                section_operator.push_back("ini-file");
            }
            if(!section_operator.empty())
            {
                params.push_back("section-operator=" + boost::algorithm::join(section_operator, ","));
            }
        }

        std::string const query_string(boost::algorithm::join(params, "&"));
        if(!query_string.empty())
        {
            ss << '?'
               << query_string;
        }

        f_url = ss.str();
    }

    return f_url;
}




/** \brief Create and read a conf_file.
 *
 * This function creates a new conf_file object unless one with the same
 * filename already exists.
 *
 * If the configuration file was already loaded, then that pointer gets
 * returned instead of reloading the file. There is currently no API to
 * allow for the removal because another thread or function may have
 * the existing pointer cached and we want all instances of a configuration
 * file to be the same (i.e. if you update the value of a parameter then
 * that new value should be visible by all the users of that configuration
 * file.) Therefore, you can think of a configuration file as a global
 * variable.
 *
 * \note
 * Any number of call this function to load a given file always returns
 * exactly the same pointer.
 *
 * \todo
 * With the communicator, we will at some point implement a class
 * used to detect that a file changed, allowing us to get a signal
 * and reload the file as required. This get_conf_file() function
 * will greatly benefit from such since that way we can automatically
 * reload the configuration file. In other words, process A could
 * make a change, then process B reloads and sees the change that
 * process A made. Such an implementation will require a proper
 * locking mechanism of the configuration files while modifications
 * are being performed.
 *
 * \param[in] setup  The settings to be used in this configuration file reader.
 *
 * \return A pointer to the configuration file data.
 */
conf_file::pointer_t conf_file::get_conf_file(conf_file_setup const & setup)
{
    cppthread::guard lock(get_global_mutex());

    auto it(g_conf_files.find(setup.get_filename()));
    if(it != g_conf_files.end())
    {
        if(it->second->get_setup().get_config_url() != setup.get_config_url())
        {
            throw getopt_logic_error("trying to load configuration file \""
                                       + setup.get_config_url()
                                       + "\" but an existing configuration file with the same name was loaded with URL: \""
                                       + it->second->get_setup().get_config_url()
                                       + "\".");
        }
        return it->second;
    }

    // TODO: look into not blocking forever?
    //
    conf_file::pointer_t cf(new conf_file(setup));
    g_conf_files[setup.get_filename()] = cf;
    return cf;
}


/** \brief Save the configuration file.
 *
 * This function saves the current data from this configuration file to
 * the file. It overwrites the existing file.
 *
 * Note that when you load the configuration, you may get data from
 * many different configuration files. This very file will only
 * include the data that was loaded from this file, though, and whatever
 * modifications you made.
 *
 * If the conf is not marked as modified, the function returns immediately
 * with true.
 *
 * \param[in] create_backup  Whether to create a backup or not.
 *
 * \return true if the save worked as expected.
 */
bool conf_file::save_configuration(bool create_backup)
{
    if(f_modified)
    {
        // create backup?
        //
        if(create_backup)
        {
            // TODO: offer means to set the backup extension
            //
            std::string const backup_filename(f_setup.get_filename() + ".bak");

            if(unlink(backup_filename.c_str()) != 0
            && errno != ENOENT)
            {
                f_errno = errno;   // LCOV_EXCL_LINE
                return false;      // LCOV_EXCL_LINE
            }

            if(rename(f_setup.get_filename().c_str(), backup_filename.c_str()) != 0)
            {
                f_errno = errno;   // LCOV_EXCL_LINE
                return false;      // LCOV_EXCL_LINE
            }
        }

        // save parameters to file
        //
        std::ofstream conf;
        conf.open(f_setup.get_filename().c_str());
        if(!conf.is_open())
        {
            f_errno = errno;   // LCOV_EXCL_LINE
            return false;      // LCOV_EXCL_LINE
        }

        time_t const now(time(nullptr));
        tm t;
        gmtime_r(&now, &t);
        char str_date[16];
        strftime(str_date, sizeof(str_date), "%Y/%m/%d", &t);
        char str_time[16];
        strftime(str_time, sizeof(str_time), "%H:%M:%S", &t);

        // header warning with date & time
        //
        conf << "# This file was auto-generated by snap_config.cpp on " << str_date << " at " << str_time << "." << std::endl
             << "# Making modifications here is likely safe unless the tool handling this" << std::endl
             << "# configuration file is actively working on it while you do the edits." << std::endl;
        for(auto p : f_parameters)
        {
            conf << p.first << "=";

            // prevent saving \r and \n characters as is when part of the
            // value; also double \ otherwise reading those back would fail
            //
            std::string value(p.second);
            boost::replace_all(value, "\\", "\\\\");
            boost::replace_all(value, "\r", "\\r");
            boost::replace_all(value, "\n", "\\n");
            boost::replace_all(value, "\t", "\\t");
            conf << value << std::endl;

            if(!conf)
            {
                return false;   // LCOV_EXCL_LINE
            }
        }

        // it all worked, it's considered saved now
        //
        f_modified = false;
    }

    return true;
}


/** \brief Initialize and read a configuration file.
 *
 * This constructor initializes this conf_file object and then reads the
 * corresponding configuration file.
 *
 * Note that you have to use the create_conf_file() function for you
 * to be able to create a configuration file. It is done that way became
 * a file can be read only once. Once loaded, it gets cached until your
 * application quits.
 *
 * \param[in] setup  The configuration file setup.
 */
conf_file::conf_file(conf_file_setup const & setup)
    : f_setup(setup)
{
    read_configuration();
}


/** \brief Get the configuration file setup.
 *
 * This function returns a copy of the setup used to load this
 * configuration file.
 *
 * \note
 * This function has no mutex protection because the setup can't
 * change so there is no multi-thread protection necessary (the
 * fact that you hold a shared pointer to the conf_file object
 * is enough protection in this case.)
 *
 * \return A reference to this configuration file setup.
 */
conf_file_setup const & conf_file::get_setup() const
{
    return f_setup;
}


/** \brief Set a callback to detect when changes happen.
 *
 * This function is used to attach a callback to this file. This is
 * useful if you'd like to know when a change happen to a parameter
 * in this configuration file.
 *
 * The callback gets called when:
 *
 * \li The set_parameter() is called and the parameter gets created.
 * \li The set_parameter() is called and the parameter gets updated.
 * \li The erase_parameter() is called and the parameter gets erased.
 *
 * You can cancel your callback by calling this function again without
 * a target (i.e. `cf->set_callback(callback_t());`).
 *
 * To attach another object to your callback, you can either create
 * a callback which is attached to your object and a function
 * member or use std::bind() to attach the object to the function
 * call.
 *
 * \param[in] callback  The new callback std::function.
 */
void conf_file::set_callback(callback_t callback)
{
    f_callback = callback;
}


/** \brief Get the error number opening/reading the configuration file.
 *
 * The class registers the errno value whenever an I/O error happens
 * while handling the configuration file. In most cases the function
 * is expected to return 0.
 *
 * The ENOENT error should not happen since the setup is going to be
 * marked as invalid when a configuration file does not exist and
 * you should not end up creation a conf_file object when that
 * happens. However, it is expected when you want to make some
 * changes to a few parameters and save them back to file (i.e. 
 * the very first time there will be no file under the writable
 * configuration folder.)
 *
 * \return The last errno detected while accessing the configuration file.
 */
int conf_file::get_errno() const
{
    cppthread::guard lock(get_global_mutex());

    return f_errno;
}


/** \brief Get a list of sections.
 *
 * This function returns a copy of the list of sections defined in
 * this configuration file. In most cases, you should not need this
 * function since you are expected to know what parameters may be
 * defined. There are times though when it can be very practical.
 * For example, the options_config.cpp makes use of it since each
 * section is a parameter which we do not know the name of until
 * we have access to this array of sections.
 *
 * \note
 * We return a list because in a multithread environment another thread
 * may decide to make changes to the list of parameters which has the
 * side effect of eventually adding a section.
 *
 * \return A copy of the list of sections.
 */
conf_file::sections_t conf_file::get_sections() const
{
    cppthread::guard lock(get_global_mutex());

    return f_sections;
}


/** \brief Get a list of parameters.
 *
 * This function returns a copy of the list of parameters defined in
 * this configuration file.
 *
 * \note
 * We return a list because in a multithread environment another thread
 * may decide to make changes to the list of parameters (including
 * erasing a parameter.)
 *
 * \return A copy of the list of parameters.
 */
conf_file::parameters_t conf_file::get_parameters() const
{
    cppthread::guard lock(get_global_mutex());

    return f_parameters;
}


/** \brief Check whether a parameter is defined.
 *
 * This function checks for the existance of a parameter. It is a good
 * idea to first check for the existance of a parameter since the
 * get_parameter() function may otherwise return an empty string and
 * you cannot know whether that empty string means that the parameter
 * was not defined or it was set to the empty string.
 *
 * \param[in] name  The name of the parameter to check.
 *
 * \return true if the parameter is defined, false otherwise.
 *
 * \sa get_parameter()
 * \sa set_parameter()
 */
bool conf_file::has_parameter(std::string name) const
{
    std::replace(name.begin(), name.end(), '_', '-');

    cppthread::guard lock(get_global_mutex());

    auto it(f_parameters.find(name));
    return it != f_parameters.end();
}


/** \brief Get the named parameter.
 *
 * This function searches for the specified parameter. If that parameter
 * exists, then its value is returned. Note that the value of a parameter
 * may be the empty string.
 *
 * If the parameter does not exist, the function returns the empty string.
 * To distinguish between an undefined parameter and a parameter set to
 * the empty string, use the has_parameter() function.
 *
 * \param[in] name  The name of the parameter to retrieve.
 *
 * \return The current value of the parameter or an empty string.
 *
 * \sa has_parameter()
 * \sa set_parameter()
 */
std::string conf_file::get_parameter(std::string name) const
{
    std::replace(name.begin(), name.end(), '_', '-');

    cppthread::guard lock(get_global_mutex());

    auto it(f_parameters.find(name));
    if(it != f_parameters.end())
    {
        return it->second;
    }
    return std::string();
}


/** \brief Set a parameter.
 *
 * This function sets a parameter to the specified value.
 *
 * The name of the value includes the \p section names and the \p name
 * parameter concatenated with a C++ scopre operator (::) in between
 * (unless \p section is the empty string in which case no scope operator
 * gets added.)
 *
 * When the \p name parameter starts with a scope parameter, the \p section
 * parameter is ignored. This allows one to ignore the current section
 * (i.e. the last '[...]' or any '\<name> { ... }').
 *
 * The \p section parameter is a list of section names separated by
 * the C++ scope operator (::).
 *
 * The \p name parameter may include C (.) and/or C++ (::) section
 * separators when the configuration file supports those. Internally,
 * those get moved to the \p section parameter. That allows us to
 * verify that the number of sections is valid.
 *
 * This function may be called any number of time. The last value is
 * the one kept. While reading the configuration file, though, a warning
 * is generated when a parameter gets overwritten since this is often the
 * source of a problem.
 *
 * In the following configuration file:
 *
 * \code
 *     var=name
 *     var=twice
 * \endcode
 *
 * The variable named `var` will be set to `twice` on return and a warning
 * will have been generated warning about the fact that the variable was
 * modified while reading the configuration file.
 *
 * The full name of the parameter (i.e. section + name) cannot include any
 * of the following characters:
 *
 * \li control characters (any character between 0x00 and 0x1F)
 * \li a space (0x20)
 * \li a backslash (`\`)
 * \li quotation (`"` and `'`)
 * \li comment (';', '#', '/')
 * \li assignment ('=', ':', '?', '+')
 *
 * \note
 * The \p section and \p name parameters have any underscore (`_`)
 * replaced with dashes (`-`) before getting used. The very first
 * character can be a dash. This allows you to therefore create
 * parameters which cannot appear in a configuration file, an
 * environment variable or on the command line (where parameter are
 * not allowed to start with a dash.)
 *
 * \warning
 * It is important to note that when a \p name includes a C++ scope
 * operator, the final parameter name looks like it includes a section
 * name (i.e. the name "a::b", when the C++ section flag is not set,
 * is accepted as is; so the final parameter name is going to be "a::b"
 * and therefore it will include what looks like a section name.)
 * There should not be any concern about this small \em glitch though
 * since you do not have to accept any such parameter.
 *
 * \param[in] section  The list of section or an empty string.
 * \param[in] name  The name of the parameter.
 * \param[in] value  The value of the parameter.
 */
bool conf_file::set_parameter(std::string section, std::string name, std::string const & value)
{
    // use the tokenize_string() function because we do not want to support
    // quoted strings in this list of sections which our split_string()
    // does automatically
    //
    string_list_t section_list;

    std::replace(section.begin(), section.end(), '_', '-');
    std::replace(name.begin(), name.end(), '_', '-');

    char const * n(name.c_str());

    // global scope? if so ignore the section parameter
    //
    if((f_setup.get_section_operator() & SECTION_OPERATOR_CPP) != 0
    && n[0] == ':'
    && n[1] == ':')
    {
        do
        {
            ++n;
        }
        while(*n == ':');
    }
    else
    {
        snap::tokenize_string(section_list
                            , section
                            , "::"
                            , true
                            , std::string()
                            , &snap::string_predicate<string_list_t>);
    }

    char const * s(n);
    while(*n != '\0')
    {
        if((f_setup.get_section_operator() & SECTION_OPERATOR_C) != 0
        && *n == '.')
        {
            if(s == n)
            {
                cppthread::log << cppthread::log_level_t::error
                               << "option name \""
                               << name
                               << "\" cannot start with a period (.)."
                               << cppthread::end;
                return false;
            }
            section_list.push_back(std::string(s, n - s));
            do
            {
                ++n;
            }
            while(*n == '.');
            s = n;
        }
        else if((f_setup.get_section_operator() & SECTION_OPERATOR_CPP) != 0
             && n[0] == ':'
             && n[1] == ':')
        {
            if(s == n)
            {
                cppthread::log << cppthread::log_level_t::error
                               << "option name \""
                               << name
                               << "\" cannot start with a scope operator (::)."
                               << cppthread::end;
                return false;
            }
            section_list.push_back(std::string(s, n - s));
            do
            {
                ++n;
            }
            while(*n == ':');
            s = n;
        }
        else
        {
            ++n;
        }
    }
    if(s == n)
    {
        cppthread::log << cppthread::log_level_t::error
                       << "option name \""
                       << name
                       << "\" cannot end with a section operator or be empty."
                       << cppthread::end;
        return false;
    }
    std::string param_name(s, n - s);

    std::string const section_name(boost::algorithm::join(section_list, "::"));

    if(f_setup.get_section_operator() == SECTION_OPERATOR_NONE
    && !section_list.empty())
    {
        cppthread::log << cppthread::log_level_t::error
                       << "option name \""
                       << name
                       << "\" cannot be added to section \""
                       << section_name
                       << "\" because there is no section support for this configuration file."
                       << cppthread::end;
        return false;
    }
    if((f_setup.get_section_operator() & SECTION_OPERATOR_ONE_SECTION) != 0
    && section_list.size() > 1)
    {
        cppthread::log << cppthread::log_level_t::error
                       << "option name \""
                       << name
                       << "\" cannot be added to section \""
                       << section_name
                       << "\" because this configuration only accepts one section level."
                       << cppthread::end;
        return false;
    }

    section_list.push_back(param_name);
    std::string const full_name(boost::algorithm::join(section_list, "::"));

    // verify that each section name only includes characters we accept
    // for a parameter name
    //
    // WARNING: we do not test with full_name because it includes ':'
    //
    for(auto sn : section_list)
    {
        for(char const * f(sn.c_str()); *f != '\0'; ++f)
        {
            switch(*f)
            {
            case '\001':    // forbid controls
            case '\002':
            case '\003':
            case '\004':
            case '\005':
            case '\006':
            case '\007':
            case '\010':
            case '\011':
            case '\012':
            case '\013':
            case '\014':
            case '\015':
            case '\016':
            case '\017':
            case '\020':
            case '\021':
            case '\022':
            case '\023':
            case '\024':
            case '\025':
            case '\026':
            case '\027':
            case '\030':
            case '\031':
            case '\032':
            case '\033':
            case '\034':
            case '\035':
            case '\036':
            case '\037':
            case ' ':       // forbid spaces
            case '\'':      // forbid all quotes
            case '"':       // forbid all quotes
            case ';':       // forbid all comment operators
            case '#':       // forbid all comment operators
            case '/':       // forbid all comment operators
            case '=':       // forbid all assignment operators
            case ':':       // forbid all assignment operators
            case '?':       // forbid all assignment operators (for later)
            case '+':       // forbid all assignment operators (for later)
            case '\\':      // forbid backslashes
                cppthread::log << cppthread::log_level_t::error
                               << "parameter \""
                               << full_name
                               << "\" on line "
                               << f_line
                               << " in configuration file \""
                               << f_setup.get_filename()
                               << "\" includes a character not acceptable for a section or parameter name (controls, space, quotes, and \";#/=:?+\\\")."
                               << cppthread::end;
                return false;

            }
        }
    }

    cppthread::guard lock(get_global_mutex());

    // add the section to the list of sections
    //
    // TODO: should we have a list of all the parent sections? Someone can
    //       write "a::b::c::d = 123" and we currently only get section
    //       "a::b::c", no section "a" and no section "a::b".
    //
    if(!section_name.empty())
    {
        f_sections.insert(section_name);
    }

    callback_action_t action(callback_action_t::created);
    auto it(f_parameters.find(full_name));
    if(it == f_parameters.end())
    {
        f_parameters[full_name] = value;
    }
    else
    {
        if(f_reading)
        {
            // this is just a warning; it can be neat to know about such
            // problems and fix them early
            //
            cppthread::log << cppthread::log_level_t::warning
                           << "parameter \""
                           << full_name
                           << "\" on line "
                           << f_line
                           << " in configuration file \""
                           << f_setup.get_filename()
                           << "\" was found twice in the same configuration file."
                           << cppthread::end;
        }

        it->second = value;

        action = callback_action_t::updated;
    }

    if(!f_reading)
    {
        f_modified = true;

        if(f_callback)
        {
            f_callback(shared_from_this(), action, full_name, value);
        }
    }

    return true;
}


/** \brief Erase the named parameter from this configuration file.
 *
 * This function can be used to remove the specified parameter from
 * this configuration file.
 *
 * If that parameter is not defined in the file, then nothing happens.
 *
 * \param[in] name  The name of the parameter to remove.
 *
 * \return true if the parameter was removed, false if it did not exist.
 */
bool conf_file::erase_parameter(std::string name)
{
    std::replace(name.begin(), name.end(), '_', '-');

    auto it(f_parameters.find(name));
    if(it == f_parameters.end())
    {
        return false;
    }

    f_parameters.erase(it);

    if(!f_reading)
    {
        f_modified = true;

        if(f_callback)
        {
            f_callback(shared_from_this(), callback_action_t::erased, name, std::string());
        }
    }

    return true;
}


/** \brief Check whether this configuration file was modified.
 *
 * This function returns the value of the f_modified flag which is true
 * if any value was createed, updated, or erased from the configuration
 * file since after it was loaded.
 *
 * This tells you whether you should call the save() function, assuming
 * you want to keep such changes.
 *
 * \return true if changes were made to this file parameters.
 */
bool conf_file::was_modified() const
{
    return f_modified;
}


/** \brief Read one characte from the input stream.
 *
 * This function reads one character from the input stream and returns it
 * as an `int`.
 *
 * If there is an ungotten character (i.e. ungetc() was called) then that
 * character is returned.
 *
 * When the end of the file is reached, this function returns -1.
 *
 * \note
 * This function is oblivious of UTF-8. It should not matter since any
 * Unicode character would anyway be treated as is.
 *
 * \param[in,out] in  The input stream.
 *
 * \return The character read or -1 when EOF is reached.
 */
int conf_file::getc(std::ifstream & in)
{
    if(f_unget_char != '\0')
    {
        int const r(f_unget_char);
        f_unget_char = '\0';
        return r;
    }

    char c;
    in.get(c);

    if(!in)
    {
        return EOF;
    }

    return static_cast<std::uint8_t>(c);
}


/** \brief Restore one character.
 *
 * This function is used whenever we read one additional character to
 * know whether a certain character followed another. For example, we
 * check for a `'\\n'` whenever we find a `'\\r'`. However, if the
 * character right after the `'\\r'` is not a `'\\n'` we call this
 * ungetc() function so next time we can re-read that same character.
 *
 * \note
 * You can call ungetc() only once between calls to getc(). The
 * current buffer is just one single character. Right now our
 * parser doesn't need more than that.
 *
 * \param[in] c  The character to restore.
 */
void conf_file::ungetc(int c)
{
    if(f_unget_char != '\0')
    {
        throw getopt_logic_error("conf_file::ungetc() called when the f_unget_char variable member is not '\\0'."); // LCOV_EXCL_LINE
    }
    f_unget_char = c;
}


/** \brief Get one line.
 *
 * This function reads one line. The function takes the line continuation
 * setup in account. So for example a line that ends with a backslash
 * continues on the next line when the line continuation is setup to Unix.
 *
 * Note that by default comments are also continued. So a backslash in
 * Unix mode continues a comment on the next line.
 *
 * There is a special case with the semicolon continuation setup. When
 * the line starts as a comment, it will end on the first standalone
 * newline (i.e. a comment does not need to end with a semi-colon.)
 *
 * \param[in,out] in  The input stream.
 * \param[out] line  Where the line gets saved.
 *
 * \return true if a line was read, false on EOF.
 */
bool conf_file::get_line(std::ifstream & in, std::string & line)
{
    line.clear();

    for(;;)
    {
        int c(getc(in));
        if(c == EOF)
        {
            return false;
        }
        if(c == ';'
        && f_setup.get_line_continuation() == line_continuation_t::line_continuation_semicolon)
        {
            return true;
        }

        while(c == '\n' || c == '\r')
        {
            // count the "\r\n" sequence as one line
            //
            if(c == '\r')
            {
                c = getc(in);
                if(c != '\n')
                {
                    ungetc(c);
                }
                c = '\n';
            }

            ++f_line;
            switch(f_setup.get_line_continuation())
            {
            case line_continuation_t::line_continuation_single_line:
                // continuation support
                return true;

            case line_continuation_t::line_continuation_rfc_822:
                c = getc(in);
                if(!iswspace(c))
                {
                    ungetc(c);
                    return true;
                }
                do
                {
                    c = getc(in);
                }
                while(iswspace(c));
                break;

            case line_continuation_t::line_continuation_msdos:
                if(line.empty()
                || line.back() != '&')
                {
                    return true;
                }
                line.pop_back();
                c = getc(in);
                break;

            case line_continuation_t::line_continuation_unix:
                if(line.empty()
                || line.back() != '\\')
                {
                    return true;
                }
                line.pop_back();
                c = getc(in);
                break;

            case line_continuation_t::line_continuation_fortran:
                c = getc(in);
                if(c != '&')
                {
                    ungetc(c);
                    return true;
                }
                c = getc(in);
                break;

            case line_continuation_t::line_continuation_semicolon:
                // if we have a comment, we want to return immediately;
                // at this time, the comments are not multi-line so
                // the call can return true only if we were reading the
                // very first line
                //
                if(is_comment(line.c_str()))
                {
                    return true;
                }
                // the semicolon is checked earlier, just keep the newline
                // in this case (but not at the start)
                //
                if(!line.empty() || c != '\n')
                {
                    line += c;
                }
                c = getc(in);
                break;

            }
        }

        // we just read the last line
        if(c == EOF)
        {
            return true;
        }

        line += c;
    }
}


/** \brief Read a configuration file.
 *
 * This function reads a configuration file and saves all the parameters it
 * finds in a map which can later be checked against an option table for
 * validation.
 *
 * \todo
 * Add support for quotes in configuration files as parameters are otherwise
 * saved as a separated list of parameters losing the number of spaces between
 * each entry.
 */
void conf_file::read_configuration()
{
    snap::safe_variable<decltype(f_reading)> safe_reading(f_reading, true);

    std::ifstream conf(f_setup.get_filename());
    if(!conf)
    {
        f_errno = errno;
        return;
    }

    std::string current_section;
    std::vector<std::string> sections;
    std::string str;
    f_line = 0;
    while(get_line(conf, str))
    {
        char const * s(str.c_str());
        while(iswspace(*s))
        {
            ++s;
        }
        if(*s == '\0'
        || is_comment(s))
        {
            // skip empty lines and comments
            continue;
        }
        if((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) != 0
        && *s == '}')
        {
            current_section = sections.back();
            sections.pop_back();
            continue;
        }
        char const * str_name(s);
        char const * e(nullptr);
        while(!is_assignment_operator(*s)
           && ((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) == 0 || (*s != '{' && *s != '}'))
           && ((f_setup.get_section_operator() & SECTION_OPERATOR_INI_FILE) == 0 || *s != ']')
           && *s != '\0'
           && !iswspace(*s))
        {
            ++s;
        }
        if(iswspace(*s))
        {
            e = s;
            while(iswspace(*s))
            {
                ++s;
            }
            if(*s != '\0'
            && !is_assignment_operator(*s)
            && (f_setup.get_assignment_operator() & ASSIGNMENT_OPERATOR_SPACE) == 0
            && ((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) == 0 || (*s != '{' && *s != '}')))
            {
                cppthread::log << cppthread::log_level_t::error
                               << "option name from \""
                               << str
                               << "\" on line "
                               << f_line
                               << " in configuration file \""
                               << f_setup.get_filename()
                               << "\" cannot include a space, missing assignment operator?"
                               << cppthread::end;
                continue;
            }
        }
        if(e == nullptr)
        {
            e = s;
        }
        if(e - str_name == 0)
        {
            cppthread::log << cppthread::log_level_t::error
                           << "no option name in \""
                           << str
                           << "\" on line "
                           << f_line
                           << " from configuration file \""
                           << f_setup.get_filename()
                           << "\", missing name before the assignment operator?"
                           << cppthread::end;
            continue;
        }
        std::string name(str_name, e - str_name);
        std::replace(name.begin(), name.end(), '_', '-');
        if(name[0] == '-')
        {
            cppthread::log << cppthread::log_level_t::error
                           << "option names in configuration files cannot start with a dash or an underscore in \""
                           << str
                           << "\" on line "
                           << f_line
                           << " from configuration file \""
                           << f_setup.get_filename()
                           << "\"."
                           << cppthread::end;
            continue;
        }
        if((f_setup.get_section_operator() & SECTION_OPERATOR_INI_FILE) != 0
        && name.length() >= 1
        && name[0] == '['
        && *s == ']')
        {
            ++s;
            if(!sections.empty())
            {
                cppthread::log << cppthread::log_level_t::error
                               << "`[...]` sections can't be used within a `section { ... }` on line "
                               << f_line
                               << " from configuration file \""
                               << f_setup.get_filename()
                               << "\"."
                               << cppthread::end;
                continue;
            }
            while(iswspace(*s))
            {
                ++s;
            }
            if(*s != '\0'
            && !is_comment(s))
            {
                cppthread::log << cppthread::log_level_t::error
                               << "section names in configuration files cannot be followed by anything other than spaces in \""
                               << str
                               << "\" on line "
                               << f_line
                               << " from configuration file \""
                               << f_setup.get_filename()
                               << "\"."
                               << cppthread::end;
                continue;
            }
            if(name.length() == 1)
            {
                // "[]" removes the section
                //
                current_section.clear();
            }
            else
            {
                current_section = name.substr(1);
                current_section += "::";
            }
        }
        else if((f_setup.get_section_operator() & SECTION_OPERATOR_BLOCK) != 0
             && *s == '{')
        {
            sections.push_back(current_section);
            current_section += name;
            current_section += "::";
        }
        else
        {
            if(is_assignment_operator(*s))
            {
                ++s;
            }
            while(iswspace(*s))
            {
                ++s;
            }
            for(e = str.c_str() + str.length(); e > s; --e)
            {
                if(!iswspace(e[-1]))
                {
                    break;
                }
            }
            size_t const len(e - s);
            std::string value(s, len);
            boost::replace_all(value, "\\\\", "\\");
            boost::replace_all(value, "\\r", "\r");
            boost::replace_all(value, "\\n", "\n");
            boost::replace_all(value, "\\t", "\t");
            set_parameter(current_section, name, unquote(value));
        }
    }
    if(!conf.eof())
    {
        f_errno = errno;                                            // LCOV_EXCL_LINE
        cppthread::log << cppthread::log_level_t::error             // LCOV_EXCL_LINE
                       << "an error occurred while reading line "   // LCOV_EXCL_LINE
                       << f_line                                    // LCOV_EXCL_LINE
                       << " of configuration file \""               // LCOV_EXCL_LINE
                       << f_setup.get_filename()                    // LCOV_EXCL_LINE
                       << "\"."                                     // LCOV_EXCL_LINE
                       << cppthread::end;                           // LCOV_EXCL_LINE
    }
    if(!sections.empty())
    {
        cppthread::log << cppthread::log_level_t::error
                       << "unterminated `section { ... }`, the `}` is missing in configuration file \""
                       << f_setup.get_filename()
                       << "\"."
                       << cppthread::end;
    }
}


/** \brief Check whether `c` is an assignment operator.
 *
 * This function checks the \p c parameter to know whether it matches
 * one of the character allowed as an assignment character.
 *
 * \param[in] c  The character to be checked.
 *
 * \return true if c is considered to represent an assignment character.
 */
bool conf_file::is_assignment_operator(int c) const
{
    assignment_operator_t const assignment_operator(f_setup.get_assignment_operator());
    return ((assignment_operator & ASSIGNMENT_OPERATOR_EQUAL) != 0 && c == '=')
        || ((assignment_operator & ASSIGNMENT_OPERATOR_COLON) != 0 && c == ':')
        || ((assignment_operator & ASSIGNMENT_OPERATOR_SPACE) != 0 && std::iswspace(c));
}


/** \brief Check whether the string starts with a comment introducer.
 *
 * This function checks whether the \p s string starts with a comment.
 *
 * We support different types of comment introducers. This function
 * checks the flags as defined in the constructor and returns true
 * if the type of character introducer defines a comment.
 *
 * We currently support:
 *
 * \li .ini file comments, introduced by a semi-colon (;)
 *
 * \li Shell file comments, introduced by a hash character (#)
 *
 * \li C++ comment, introduced by two slashes (//)
 *
 * \param[in] s  The string to check for a comment.
 *
 * \return `true` if the string represents a comment.
 */
bool conf_file::is_comment(char const * s) const
{
    comment_t const comment(f_setup.get_comment());
    if((comment & COMMENT_INI) != 0
    && *s == ';')
    {
        return true;
    }

    if((comment & COMMENT_SHELL) != 0
    && *s == '#')
    {
        return true;
    }

    if((comment & COMMENT_CPP) != 0
    && s[0] == '/'
    && s[1] == '/')
    {
        return true;
    }

    return false;
}


/** \brief Returns true if \p c is considered to be a whitespace.
 *
 * Our iswspace() function is equivalent to the std::iswspace() function
 * except that `'\\r'` and `'\\n'` are never viewed as white spaces.
 *
 * \return true if c is considered to be a white space character.
 */
bool iswspace(int c)
{
    return c != '\n'
        && c != '\r'
        && std::iswspace(c);
}


}   // namespace advgetopt
// vim: ts=4 sw=4 et
