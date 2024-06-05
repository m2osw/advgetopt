// Copyright (c) 2006-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Implementation of utility functions.
 *
 * This file includes various utility functions that are not specifically
 * attached to a class.
 */

// self
//
#include    "advgetopt/utils.h"

#include    "advgetopt/exception.h"


// snapdev
//
#include    <snapdev/glob_to_list.h>
#include    <snapdev/isatty.h>
#include    <snapdev/not_used.h>
#include    <snapdev/trim_string.h>


// cppthread
//
#include    <cppthread/guard.h>
#include    <cppthread/mutex.h>


// C++
//
#include    <cstring>
#include    <iomanip>
#include    <set>
#include    <sstream>


// C
//
#include    <sys/ioctl.h>
#include    <sys/stat.h>
#include    <unistd.h>




// last include
//
#include    <snapdev/poison.h>



namespace advgetopt
{



namespace
{



/** \brief The configuration file mutex.
 *
 * This options are generally viewed as read-only global variables. They
 * get setup once early on and then used and reused as many times as
 * required.
 *
 * This mutex makes sure that access between multiple thread happens in
 * a safe manner.
 */
cppthread::mutex *      g_mutex;



constexpr char const    g_single_quote = '\'';
constexpr char const *  g_empty_string = "\"\"";
constexpr char const *  g_escaped_single_quotes = "'\\''";
constexpr char const *  g_simple_characters = "+-./0123456789=ABCEFGHIJKLMNOPQRSTUVWXYZabcefghijklmnopqrstuvwxyz_";



}
// no name namespace



/** \brief Get a global mutex.
 *
 * This function returns a global mutex we can use to lock the advgetopt
 * whenever multithread functionality is required (i.e. a global is used.)
 *
 * It is safe to call this function early (i.e. before main was ever
 * called.)
 *
 * Usage:
 *
 * \code
 *    cppthread::guard lock(get_global_mutex());
 * \endcode
 *
 * \return A reference to our global mutex.
 */
cppthread::mutex & get_global_mutex()
{
    {
        cppthread::guard lock(*cppthread::g_system_mutex);

        if(g_mutex == nullptr)
        {
            g_mutex = new cppthread::mutex();
        }
    }

    return *g_mutex;
}



/** \brief Remove single (') or double (") quotes from a string.
 *
 * If a string starts and ends with the same quotation mark, then it
 * gets removed.
 *
 * If no quotes appear, then the function returns a copy of the input as is.
 *
 * The \p pairs parameter must have an even size (or the last character
 * gets ignored). By default, it is set to the double and single quotes:
 *
 * \code
 *     "\"\"''"
 * \endcode
 *
 * To remove square, angle, curly brackets:
 *
 * \code
 *     "[]<>{}"
 * \endcode
 *
 * \todo
 * Add support for UTF-8 quotes. Right now only quotes of 1 byte will
 * work.
 *
 * \param[in] s  The string to unquote.
 * \param[in] pairs  A list of accepted quotes.
 *
 * \return The unquoted string.
 */
std::string unquote(std::string const & s, std::string const & pairs)
{
    if(s.length() >= 2)
    {
        std::string::size_type const max(pairs.length() - 1);
        for(std::string::size_type pos(0); pos < max; pos += 2)
        {
            if(s.front() == pairs[pos + 0]
            && s.back()  == pairs[pos + 1])
            {
                return s.substr(1, s.length() - 2);
            }
        }
    }

    return s;
}


/** \brief The converse of unquote.
 *
 * This function adds quotes around a string.
 *
 * If you do not define the \p close quotation (i.e. it remains set to the
 * NUL character '\0'), then the \p open quotation gets reused as the closing
 * quotation.
 *
 * \param[in] s  The string to be quoted.
 * \param[in] open  The opening quote to quote this string.
 * \param[in] close  The closing quote to quote this string.
 *
 * \return The input string quoted with \p quote.
 */
std::string quote(std::string const & s, char open, char close)
{
    std::string result;

    if(close == '\0')
    {
        close = open;
    }

    result += open;
    for(auto const c : s)
    {
        if(c == open
        || c == close)
        {
            result += '\\';
        }
        result += c;
    }
    result += close;

    return result;
}


/** \brief Convert the `_` found in a string to `-` instead.
 *
 * Options are saved with `-` instead of `_` so all the standard compare
 * functions can be used to find options. This function converts a string
 * so all of the `_` charaters get transformed to `-` characters.
 *
 * Why do we support both?
 *
 * It is customary to use the `-` in long command line option names.
 * For example `--long-form` uses a `-`. (One exception is ffmpeg which
 * uses `_` in their long command line option names).
 *
 * However, the advgetopt library also reads Unix like configuration files
 * and parameters in those files are generally expected to use underscores
 * (`_`) in their names. For example `email_address = contact@example.com`.
 *
 * To make it simpler, the advgetopt library accepts both characters and
 * decides to view them as being equal. So you can use both forms in both
 * situations. The following are equivalent:
 *
 * \code
 *     my-command --long-form
 *     my-command --long_form
 * \endcode
 *
 * This function is used to convert a string to the advgetopt format which
 * is to keep only `-` in the names. So if it finds a `_`, it gets
 * transformed.
 *
 * \param[in] s  The string to transform.
 *
 * \return A copy with all `_` transformed to `-`.
 */
std::string option_with_dashes(std::string const & s)
{
    std::string result;
    result.reserve(s.length());
    for(auto const & c : s)
    {
        if(c == '_')
        {
            result += '-';
        }
        else
        {
            result += c;
        }
    }
    return result;
}


/** \brief Converts an option back to using underscores.
 *
 * When generating some error messages, we like to show underscores if the
 * variable comes from a configuration file. In this case we use this function
 * to convert the dashes back to underscores and print that in the message.
 *
 * \param[in] s  The string to be converted.
 *
 * \return A copy of the string with `-` converted to `_`.
 */
std::string option_with_underscores(std::string const & s)
{
    std::string result;
    result.reserve(s.length());
    for(auto const & c : s)
    {
        if(c == '-')
        {
            result += '_';
        }
        else
        {
            result += c;
        }
    }
    return result;
}


/** \brief Split a string in sub-strings separated by \p separators.
 *
 * This function searches for any of the \p separators in \p str and
 * split at those locations.
 *
 * For example, to split a comma separated list of strings, use the
 * following:
 *
 * \code
 *     string_list_t result;
 *     option_info::split_string(string_to_split, result, {","});
 * \endcode
 *
 * If `string_to_split` is set to "a, b, c", then the `result` vector
 * will have three strings as a result: `a`, `b`, and `c`. Note that
 * the function automatically trims all strings and it never keeps
 * empty strings. So two separators one after another is accepted and
 * no empty string results.
 *
 * The trimming happens after the split occurs. This allows for the
 * list of separators to include spaces as separators.
 *
 * The function does not clear the result vector. This allows you to
 * call this function multiple times with various strings and the
 * results will be cumulated.
 *
 * \note
 * This function is a static so it can be used from anywhere to split
 * strings as required. You do not need to have an option_info instance.
 *
 * \todo
 * See to fix the fact that `a"b"c` becomes `{"a", "b", "c"}` when
 * there are not separators between `a`, `"b"`, and `c`. To the minimum
 * we may want to generate an error when such is found (i.e. when a
 * quote is found and `start < pos` is true.
 *
 * \param[in] str  The string to split.
 * \param[in] result  The vector where the split strings are saved.
 * \param[in] separators  The vector of strings used as separators.
 */
void split_string(std::string const & str
                , string_list_t & result
                , string_list_t const & separators)
{
    std::string::size_type pos(0);
    std::string::size_type start(0);
    while(pos < str.length())
    {
        if(str[pos] == '\'' || str[pos] == '"')
        {
            if(start < pos)
            {
                std::string const v(snapdev::trim_string(str.substr(start, pos - start)));
                if(!v.empty())
                {
                    result.push_back(v);
                }
                start = pos;
            }

            // quoted parameters are handled without the separators
            //
            char const quote(str[pos]);
            for(++pos; pos < str.length() && str[pos] != quote; ++pos);

            std::string const v(str.substr(start + 1, pos - (start + 1)));
            if(!v.empty())
            {
                result.push_back(v);
            }
            if(pos < str.length())
            {
                // skip the closing quote
                //
                ++pos;
            }
            start = pos;
        }
        else
        {
            bool found(false);
            for(auto const & sep : separators)
            {
                if(str.length() - pos >= sep.length())
                {
                    if(str.compare(pos, sep.length(), sep) == 0)
                    {
                        // match! cut here
                        //
                        if(start < pos)
                        {
                            std::string const v(snapdev::trim_string(str.substr(start, pos - start)));
                            if(!v.empty())
                            {
                                result.push_back(v);
                            }
                        }
                        pos += sep.length();
                        start = pos;
                        found = true;
                        break;
                    }
                }
            }

            if(!found)
            {
                ++pos;
            }
        }
    }

    if(start < pos)
    {
        std::string const v(snapdev::trim_string(str.substr(start, pos - start)));
        if(!v.empty())
        {
            result.push_back(v);
        }
    }
}


/** \brief Insert the group (or project) name in the filename.
 *
 * This function inserts the name of the group in the specified full path
 * filename. It gets added right before the basename. So for example you
 * have a path such as:
 *
 *     /etc/snapwebsites/advgetopt.conf
 *
 * and a group name such as:
 *
 *     adventure
 *
 * The resulting path is:
 *
 *     /etc/snapwebsites/adventure.d/##-advgetopt.conf
 *
 * where the '##' is a number from 00 to 99. If none of those files exists,
 * the default (50) is used if \p add_default_on_empty is true.
 *
 * Notice that the function adds a ".d" as well.
 *
 * If the group name is empty or null, then the project name is used. If
 * both are empty, then nothing happens (the function returns an empty list).
 *
 * \exception getopt_root_filename
 * The \p filename parameter cannot be a file in the root directory.
 *
 * \param[in] filename  The filename where the project name gets injected.
 * \param[in] group_name  The name of the group to inject in the filename.
 * \param[in] project_name  The name of the project to inject in the filename.
 * \param[in] add_default_on_empty  Whether the add the default if no files
 * exist.
 *
 * \return The list of filenames or an empty list if no group or project name
 *         or filename were specified.
 */
string_list_t insert_group_name(
          std::string const & filename
        , char const * group_name
        , char const * project_name
        , bool add_default_on_empty)
{
    if(filename.empty())
    {
        return string_list_t();
    }

    std::string name;
    if(group_name == nullptr
    || *group_name == '\0')
    {
        if(project_name == nullptr
        || *project_name == '\0')
        {
            return string_list_t();
        }
        name = project_name;
    }
    else
    {
        name = group_name;
    }

    std::string pattern;
    std::string::size_type const pos(filename.find_last_of('/'));
    if(pos == 0)
    {
        throw getopt_root_filename(
                  "filename \""
                + filename
                + "\" last slash (/) is at the start, which is not allowed.");
    }
    if(pos != std::string::npos
    && pos > 0)
    {
        pattern = filename.substr(0, pos + 1)
                + name
                + ".d/[0-9][0-9]-"
                + filename.substr(pos + 1);
    }
    else
    {
        pattern = name
                + ".d/[0-9][0-9]-"
                + filename;
    }

    // we use an std::set so the resulting list is sorted
    //
    snapdev::glob_to_list<std::set<std::string>> glob;

    // the glob() function is not thread safe
    {
        cppthread::guard lock(get_global_mutex());
        snapdev::NOT_USED(glob.read_path<snapdev::glob_to_list_flag_t::GLOB_FLAG_IGNORE_ERRORS>(pattern));
    }

    // we add the default name if none other exists
    //
    if(add_default_on_empty
    && glob.empty())
    {
        glob.insert(default_group_name(
                  filename
                , group_name
                , project_name));
    }

    return string_list_t(glob.begin(), glob.end());
}


/** \brief Generate the default filename (the ".../50-...")
 *
 * This function generates the default filename as the insert_group_name()
 * expects to find in the configuration sub-directory.
 *
 * The name is formed as follow:
 *
 *     <path> / <directory> ".d" / <priority> "-" <basename>
 *
 * Where `<path>` is the path found in \p filename. If no path is defined in
 * \p filename, then the `<path> /` part is not prepended:
 *
 *     <directory> ".d" / <priority> "-" <basename>
 *
 * Where `<directory>` is the \p group_name if defined, otherwise it uses
 * the \p project_name. This is why if neither is defined, then the function
 * immediately returns an empty string.
 *
 * Where `<priority>` is a number from 0 to 99 inclusive. This is used to
 * sort the files before processing them. File with lower priorities are
 * loaded first. Parameters found in files with higher priorities overwrite
 * the values of parameters found in files with lower priorities.
 *
 * Where `<basename>` is the end of \p filename, the part after the last
 * slash (`/`). If \p filename is not empty and it does not include a slash
 * then the entire \p filename is taken as the `<basename>`. Note that
 * \p filename is expected to include an extension such as `.conf`. The
 * extension is not modified in any way.
 *
 * Since the result is not viable when \p filename is empty, the function
 * immediately returns an empty string in that situation.
 *
 * \exception getopt_root_filename
 * The \p filename parameter cannot be a file in the root directory.
 *
 * \param[in] filename  The filename where the project name gets injected.
 * \param[in] group_name  The name of the group to inject in the filename.
 * \param[in] project_name  The name of the project to inject in the filename.
 * \param[in] priority  The priority of the new file (0 to 99).
 *
 * \return The default filenames or an empty list if no group or project
 *         or file name were specified.
 */
std::string default_group_name(
          std::string const & filename
        , char const * group_name
        , char const * project_name
        , int priority)
{
    if(priority < 0 || priority >= 100)
    {
        throw getopt_invalid_parameter(
              "priority must be a number between 0 and 99 inclusive; "
            + std::to_string(priority)
            + " is invalid.");
    }

    if(filename.empty())
    {
        return std::string();
    }

    char const * name(nullptr);
    if(group_name == nullptr
    || *group_name == '\0')
    {
        if(project_name == nullptr
        || *project_name == '\0')
        {
            return std::string();
        }
        name = project_name;
    }
    else
    {
        name = group_name;
    }

    std::string::size_type const pos(filename.find_last_of('/'));
    if(pos == 0)
    {
        throw getopt_root_filename("filename \"" + filename + "\" starts with a slash (/), which is not allowed.");
    }

    std::string result;
    result.reserve(filename.length() + strlen(name) + 6);
    if(pos != std::string::npos)
    {
        result = filename.substr(0, pos + 1);
    }
    result += name;
    result += ".d/";
    if(priority < 10)
    {
        result += '0';
    }
    result += std::to_string(priority);
    result += '-';
    if(pos == std::string::npos)
    {
        result += filename;
    }
    else
    {
        result += filename.substr(pos + 1);
    }

    return result;
}


/** \brief Replace a starting `~/...` with the contents of the \$HOME variable.
 *
 * This function checks the beginning of \p filename. If it starts with `'~/'`
 * then it replaces the `'~'` character with the contents of the \$HOME
 * environment variable.
 *
 * If \p filename is just `"~"`, then the function returns the contents of
 * the \$HOME environment variable by itself.
 *
 * If somehow the \$HOME environment variable is empty, the function does
 * nothing.
 *
 * \todo
 * Add support for "~<user name>/..." so that way a service could use its
 * own home folder even when run from a different user (a.k.a. root). This
 * requires that we load the user database and get the home folder from that
 * data.
 *
 * \param[in] filename  The filename to check for a tilde (~).
 *
 * \return The input as is unless the \$HOME path can be prepended to replace
 *         the tilde (~) character.
 */
std::string handle_user_directory(std::string const & filename)
{
    if(!filename.empty()
    && filename[0] == '~'
    && (filename.length() == 1 || filename[1] == '/'))
    {
        char const * const home(getenv("HOME"));
        if(home != nullptr
        && *home != '\0')
        {
            return home + filename.substr(1);
        }
    }

    return filename;
}


/** \brief Check whether a value represents "true".
 *
 * This function checks a string to see whether it is one of:
 *
 * * "true"
 * * "on"
 * * "yes"
 * * "1"
 *
 * If so, then the function returns true.
 *
 * \param[in] s  The string to be checked.
 *
 * \return true if the string represents "true".
 */
bool is_true(std::string s)
{
    return s == "true" || s == "on" || s == "yes" | s == "1";
}


/** \brief Check whether a value represents "false".
 *
 * This function checks a string to see whether it is one of:
 *
 * * "false"
 * * "off"
 * * "no"
 * * "0"
 *
 * If so, then the function returns true.
 *
 * \param[in] s  The string to be checked.
 *
 * \return true if the string represents "false".
 */
bool is_false(std::string s)
{
    return s == "false" || s == "off" || s == "no" || s == "0";
}


/** \brief Retrieve the width of one line in your console.
 *
 * This function retrieves the width of the console in number of characters.
 *
 * If the process is not connected to a TTY, then the function returns 80.
 *
 * If the width is less than 40, the function returns 40.
 *
 * \return The width of the console screen.
 */
size_t get_screen_width()
{
    std::int64_t cols(80);

    if(isatty(STDOUT_FILENO))
    {
// LCOV_EXCL_START
        // when running coverage, the output is redirected for logging purposes
        // which means that isatty() returns false -- so at this time I just
        // exclude those since they are unreachable from my standard Unit Tests
        //
        winsize w;
        if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
        {
            cols = std::max(static_cast<unsigned short>(40), w.ws_col);
        }
// LCOV_EXCL_STOP
    }

    return cols;
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
std::string breakup_line(
      std::string line
    , size_t const option_width
    , size_t const line_width)
{
    std::stringstream ss;

    size_t const width(line_width - option_width);

    // TODO: once we have C++17, avoid substr() using std::string_view instead
    //
    for(;;)
    {
        std::string l;
        std::string::size_type const nl(line.find('\n'));
        if(nl != std::string::npos
        && nl < width)
        {
            l = line.substr(0, nl);
            line = line.substr(nl + 1);
        }
        else if(line.size() <= width)
        {
            break;
        }
        else if(std::isspace(line[width]))
        {
            // special case when the space is right at the edge
            //
            l = line.substr(0, width);
            size_t pos(width);
            do
            {
                ++pos;
            }
            while(std::isspace(line[pos]));
            line = line.substr(pos);
        }
        else
        {
            // search for the last space before the edge of the screen
            //
            std::string::size_type pos(line.find_last_of(' ', width));
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

                // remove additional spaces from the start of the next line
                do  // LCOV_EXCL_LINE
                {
                    ++pos;
                }
                while(std::isspace(line[pos]));
                line = line.substr(pos);
            }
        }

        ss << l
           << std::endl;

        // more to print? if so we need the indentation
        //
        if(!line.empty()
        && option_width > 0)
        {
            ss << std::setw(option_width) << " ";
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
std::string format_usage_string(
                      std::string const & argument
                    , std::string const & help
                    , size_t const option_width
                    , size_t const line_width)
{
    std::stringstream ss;

    ss << "   ";

    if(argument.size() < option_width - 3)
    {
        // enough space on a single line
        //
        ss << argument
           << std::setw(option_width - 3 - argument.size())
           << " ";
    }
    else if(argument.size() >= line_width - 4)
    {
        // argument too long for even one line on the screen!?
        // call the function to break it up with indentation of 3
        //
        ss << breakup_line(argument, 3, line_width);

        if(!help.empty()
        && option_width > 0)
        {
            ss << std::setw(option_width) << " ";
        }
    }
    else
    {
        // argument too long for the help to follow immediately
        //
        ss << argument
           << std::endl
           << std::setw(option_width)
           << " ";
    }

    ss << breakup_line(help, option_width, line_width);

    return ss.str();
}


/** \brief Escape special characters from a shell argument.
 *
 * This function goes through the supplied argument. If it includes one
 * or more character other than `[-+0-9A-Za-z_]`, then it gets \em escaped.
 * This means we add single quotes at the start and end, and escape any
 * single quote within the argument.
 *
 * So the function may return the input string as is.
 *
 * \param[in] arg  The argument to escape.
 *
 * \return The escaped argument.
 */
std::string escape_shell_argument(std::string const & arg)
{
    if(arg.empty())
    {
        return std::string(g_empty_string);
    }

    std::string::size_type const pos(arg.find_first_not_of(g_simple_characters));
    if(pos == std::string::npos)
    {
        return arg;
    }

    std::string result;

    result += g_single_quote;
    std::string::size_type p1(0);
    while(p1 < arg.length())
    {
        std::string::size_type const p2(arg.find('\'', p1));
        if(p2 == std::string::npos)
        {
            result += arg.substr(p1);
            break;
        }
        result += arg.substr(p1, p2 - p1);
        result += g_escaped_single_quotes;
        p1 = p2 + 1;                            // skip the '
    }
    result += g_single_quote;

    return result;
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
std::string sanitizer_details()
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


/** \brief Retrieve the height of your console.
 *
 * This function retrieves the height of the console in number of characters.
 * This is also called the number of rows.
 *
 * If the process is not connected to a TTY, then the function returns 25.
 *
 * If the height is less than 2, the function returns 2.
 *
 * \note
 * The get_screen_width() and get_screen_height() should be combined.
 *
 * \return The width of the console screen.
 */
// LCOV_EXCL_START
size_t get_screen_height()
{
    std::int64_t rows(25);

    if(isatty(STDOUT_FILENO))
    {
        // when running coverage, the output is redirected for logging purposes
        // which means that isatty() returns false -- so at this time I just
        // exclude those since they are unreachable from my standard Unit Tests
        //
        winsize w;
        if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
        {
            rows = std::max(static_cast<unsigned short>(2), w.ws_row);
        }
    }

    return rows;
}
// LCOV_EXCL_STOP


/** \brief Print out a string to the console or use less.
 *
 * If the \p data string to be output is too large for the screen (too
 * many lines; we assume the width was already "fixed") then use less
 * to show the data. If less is not available, use more. If neither
 * is available, fall back to printing everything at once.
 *
 * \param[in,out] out  The output stream where the data has to be written.
 * \param[in] data  The data to be written to stream.
 */
void less(std::basic_ostream<char> & out, std::string const & data)
{
    if(snapdev::isatty(out))
    {
// LCOV_EXCL_START
        auto const lines(std::count(data.begin(), data.end(), '\n'));
        size_t const height(get_screen_height());
        if(lines > static_cast<std::remove_const_t<decltype(lines)>>(height))
        {
            struct stat s;
            if(stat("/bin/less", &s) == 0)
            {
                FILE * f(popen("/bin/less", "w"));
                if(f != nullptr)
                {
                    fwrite(data.c_str(), sizeof(char), data.length(), f);
                    pclose(f);
                    return;
                }
            }
            else if(stat("/bin/more", &s) == 0)
            {
                FILE * f(popen("/bin/more", "w"));
                if(f != nullptr)
                {
                    fwrite(data.c_str(), sizeof(char), data.length(), f);
                    pclose(f);
                    return;
                }
            }
        }
// LCOV_EXCL_STOP
    }

    // fallback, just print everything to the console as is
    //
    out << data << std::endl;
}



}   // namespace advgetopt
// vim: ts=4 sw=4 et
