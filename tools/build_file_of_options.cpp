// Copyright (c) 2020-2023  Made to Order Software Corp.  All Rights Reserved
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
 * \brief build-file-of-options tool.
 *
 * We use this tool to convert the comments found in our configuration files
 * in a list of options that can be parsed by the advgetopt objects.
 *
 * Specifically, it understands the option name, default value, and when
 * available some other features such as ranges and types.
 */


// advgetopt
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/exception.h>
#include    <advgetopt/version.h>


// libutf8
//
#include    <libutf8/libutf8.h>


// libexcept
//
#include    <libexcept/file_inheritance.h>


// snapdev
//
#include    <snapdev/not_reached.h>
#include    <snapdev/trim_string.h>


// boost
//
#include    <boost/preprocessor/stringize.hpp>


// C++
//
#include    <iostream>
#include    <fstream>
#include    <sstream>


// last include
//
#include    <snapdev/poison.h>



namespace
{

/** \brief Command line options.
 *
 * This table includes all the command line options supported by the
 * `build-file-of-options` tool.
 */
advgetopt::option const g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("output")
        , advgetopt::ShortName('o')
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("Specify the path and filename of the output file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("--")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_MULTIPLE
                    , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
        , advgetopt::Help("<configuration filename> ...")
    ),
    advgetopt::end_options()
};


/** \brief The tool looks for this configuration file.
 *
 * The build-file-of-options allow you to have a configuration file
 * with various options in it.
 */
constexpr char const * const g_configuration_files[]
{
    "/etc/advgetopt/build-file-of-options.conf",
    nullptr
};




// TODO: once we have stdc++20, remove all defaults
#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "build-file-of-options",
    .f_group_name = nullptr,
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = "BUILD_FILE_OF_OPTIONS_OPTIONS",
    .f_environment_variable_intro = nullptr,
    .f_section_variables_name = nullptr,
    .f_configuration_files = g_configuration_files,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>] <configuration file>\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = LIBADVGETOPT_VERSION_STRING,
    .f_license = nullptr,
    .f_copyright = "Copyright (c) 2019-" BOOST_PP_STRINGIZE(UTC_BUILD_YEAR) "  Made to Order Software Corporation",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME
};





class build_file
{
public:
                                    build_file(int argc, char * argv[]);

    int                             run();

private:
    int                             read_conf(std::string const & filename);
    void                            append_flag(std::string & flags, std::string const & name);

    advgetopt::getopt               f_opt;
    std::ifstream                   f_in = std::ifstream();
};



build_file::build_file(int argc, char * argv[])
    : f_opt(g_options_environment, argc, argv)
{
    if(!f_opt.is_defined("output"))
    {
        throw advgetopt::getopt_exit("the --output command line option is required.", 2);
    }
    if(f_opt.size("--") == 0)
    {
        throw advgetopt::getopt_exit("at least one input filename is required.", 2);
    }
}


int build_file::run()
{
    int r(0);

// TODO: actually implement the tool!

    // read the input in memory
    //
    std::size_t const max(f_opt.size("--"));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        r = read_conf(f_opt.get_string("--", idx));
        if(r != 0)
        {
            return r;
        }
    }

    return r;
}


int build_file::read_conf(std::string const & filename)
{
    f_in.open(filename);
    if(!f_in.is_open())
    {
        std::cerr
            << "error: could not open file \""
            << filename
            << "\".\n";
        return 1;
    }

    // read "whatever" up to an empty line (line without comments or a
    // value) -- we ignore spaces (trim)
    //
    std::string l;
    int line(1);
    for(++line; f_in; ++line)
    {
        for(; f_in; ++line)
        {
            getline(f_in, l);
            l = snapdev::trim_string(l);
            if(l.empty())
            {
                break;
            }
        }

        bool found(false);
        for(; f_in; ++line)
        {
            getline(f_in, l);
            l = snapdev::trim_string(l);
            if(l.length() > 0
            && l[0] == '#')
            {
                found = true;
                break;
            }
        }

        if(found)
        {
            // found a comment, parse it
            //
            std::string::size_type const pos(l.find('='));
            std::string name;
            std::string params;
            if(pos != std::string::npos)
            {
                // there is an equal, assume a parameter is properly
                // defined; read its name
                //
                name = snapdev::trim_string(l.substr(1, pos - 1));
                params = snapdev::trim_string(l.substr(pos + 1));
            }
            std::string::size_type const space(name.find(' '));
            std::string::size_type const colon(name.find(':'));
            if(!name.empty()
            && space == std::string::npos
            && colon == std::string::npos)
            {
                std::string alias;
                std::string::size_type const has_alias(name.find('|'));
                if(has_alias != std::string::npos)
                {
                    alias = name.substr(has_alias + 1);
                    name = name.substr(0, has_alias);
                }

                std::string short_name;
                if(params.back() == ')')
                {
                    std::string::size_type const start_short_name(params.rfind('('));
                    short_name = params.substr(start_short_name + 1, params.length() - start_short_name - 2);
                    params = snapdev::trim_string(params.substr(0, start_short_name));
                    if(libutf8::u8length(short_name) != 1)
                    {
                        std::cerr << "error:"
                            << filename
                            << ":"
                            << line
                            << ": a short name specification must be exactly one character; \""
                            << short_name
                            << "\" is not valid.\n";
                        return 1;
                    }
                }

                // the help is the first sentence following (if there is one)
                // the name=<possible values> line; this can be many lines
                // of comments. We read them all up to a "Default:"
                //
                std::string help;
                std::string default_value;
                for(++line; f_in; ++line)
                {
                    getline(f_in, l);
                    l = snapdev::trim_string(l);
                    if(l.length() > 0
                    && l[0] == '#')
                    {
                        l = snapdev::trim_string(l.substr(1));
                        if(l.length() >= sizeof("Default:")
                        && l[0] == 'D'
                        && l[1] == 'e'
                        && l[2] == 'f'
                        && l[3] == 'a'
                        && l[4] == 'u'
                        && l[5] == 'l'
                        && l[6] == 't'
                        && l[7] == ':')
                        {
                            default_value = snapdev::trim_string(l.substr(8));
                            break;
                        }
                        help += ' ';
                        help += l;
                    }
                }

                std::string::size_type const dot(help.find('.'));
                if(dot != std::string::npos)
                {
                    help = snapdev::trim_string(help.substr(0, dot + 1), true, true, true);
                }

                std::string flags;
                std::cout
                    << "    advgetopt::define_option(\n"
                    << "          advgetopt::Name(\"" << name << "\")\n";

                if(!short_name.empty())
                {
                    std::cout
                        << "        , advgetopt::ShortName(U'" << short_name << "')\n";
                }

                if(!params.empty())
                {
                    if(params.front() != '[' || params.back() != ']')
                    {
                        append_flag(flags, "REQUIRED");
                    }
                    if((params.front() == '{' && params.back() == '}')
                    || params.find("...") != std::string::npos)
                    {
                        append_flag(flags, "MULTIPLE");
                    }
                    std::cout
                        << "        , advgetopt::Args(\"" << params << "\")\n";
                }

                if(!help.empty())
                {
                    std::cout
                        << "        , advgetopt::Help(\"" << help << "\")\n";
                }

                if(!flags.empty())
                {
                    std::cout
                        << "        , advgetopt::Flags("
                        << flags << ")\n";
                }

                std::cout << "    ),\n";

                // if there is an alias defined, generate it now
                //
                if(!alias.empty())
                {
                    flags.clear();
                    std::cout
                        << "    advgetopt::define_option(\n"
                        << "          advgetopt::Name(\"" << alias << "\")\n";

                    // TBD: do we need the other flags?
                    //
                    append_flag(flags, "ALIAS");

                    // advgetopt replaces the alias with name
                    //
                    std::cout
                        << "        , advgetopt::Help(\"" << name << "\")\n";

                    if(!flags.empty())
                    {
                        std::cout
                            << "        , advgetopt::Flags("
                            << flags << ")\n";
                    }

                    std::cout << "    ),\n";
                }
            }
        }
    }

    return 0;
}


void build_file::append_flag(std::string & flags, std::string const & name)
{
    if(flags.empty())
    {
        flags += "\n                      ";
    }
    else
    {
        flags += "\n                    , ";
    }
    flags += "advgetopt::GETOPT_FLAG_";
    flags += name;
}




} // no name namespace




int main(int argc, char * argv[])
{
    libexcept::verify_inherited_files();

    try
    {
        build_file session(argc, argv);
        return session.run();
    }
    catch(advgetopt::getopt_exit const & e)
    {
        if(e.code() != 1)
        {
            std::cerr << "error: " << e.what() << std::endl;
        }
        exit(e.code());
    }
    catch(std::exception const & e)
    {
        std::cerr << "error: exception caught: " << e.what() << std::endl;
        return 1;
    }
    snapdev::NOT_REACHED();
}


// vim: ts=4 sw=4 et

