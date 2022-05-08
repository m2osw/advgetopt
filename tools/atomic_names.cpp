// Copyright (c) 2020-2022  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/cppthread
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
 * \brief Convert an atomic name file to C++.
 *
 * This tool helps in converting a file of atomic names to a C++
 * implementation and header.
 *
 * The format is:
 *
 * \code
 *     introducer=<name>
 *     project=<name>
 *     sub_project=<name>       # optional (used by plugins)
 *
 *     [names]
 *     <name1>="..."
 *     <name2>=deprecated:"..."
 *     ...
 * \endcode
 *
 * Values that start with "deprecated:" are marked as deprecated in the
 * header file. Trying to use them will generate an error as you are
 * expected to stop using them (i.e. the original owner of that string
 * is not using it anymore). It is useful to keep deprecated names
 * around until all your systems get updated with and none of the other
 * services, plugins, tools use them.
 */


// cppthread
//
#include    <cppthread/thread.h>
#include    <cppthread/version.h>


// libexcept
//
#include    <libexcept/exception.h>


// advgetopt
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/conf_file.h>


// snapdev
//
#include    <snapdev/file_contents.h>
#include    <snapdev/pathinfo.h>


// boost
//
#include    <boost/preprocessor/stringize.hpp>


// C++
//
#include    <iostream>
#include    <sstream>


// last include
//
#include    <snapdev/poison.h>


namespace
{


const advgetopt::option g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("output-path")
        , advgetopt::ShortName('o')
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_REQUIRED
            , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::EnvironmentVariableName("OUTPUT_PATH")
        , advgetopt::Help("path to where the output files get saved.")
    ),
    advgetopt::define_option(
          advgetopt::Name("verbose")
        , advgetopt::ShortName('v')
        , advgetopt::Flags(advgetopt::standalone_command_flags<
              advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::EnvironmentVariableName("VERBOSE")
        , advgetopt::Help("make the tool verbose.")
    ),
    advgetopt::define_option(
          advgetopt::Name("--")
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_REQUIRED
            , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("filename with atomic name definitions; the same name is used to generate the output, only the extension gets changed.")
    ),
    advgetopt::end_options()
};

advgetopt::group_description const g_group_descriptions[] =
{
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
        , advgetopt::GroupName("command")
        , advgetopt::GroupDescription("Commands:")
    ),
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
        , advgetopt::GroupName("option")
        , advgetopt::GroupDescription("Options:")
    ),
    advgetopt::end_groups()
};

constexpr char const * const g_configuration_files[] =
{
    "/etc/cppthread/atomic-names.conf",
    nullptr
};




// TODO: once we have stdc++20, remove all defaults & pragma
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "atomic-names",
    .f_group_name = "atomic-names",
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = "ATOMIC_NAMES",
    .f_environment_variable_intro = "ATOMIC_NAMES_",
    .f_section_variables_name = nullptr,
    .f_configuration_files = g_configuration_files,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>]\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "Try `man atomic-names` for more info.\n%c",
    .f_version = CPPTHREAD_VERSION_STRING,
    .f_license = "GPL v2 or newer",
    .f_copyright = "Copyright (c) 2022-" BOOST_PP_STRINGIZE(UTC_BUILD_YEAR) "  Made to Order Software Corporation",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions
};
#pragma GCC diagnostic pop






class atomic_names
{
public:
                            atomic_names(int argc, char * argv[]);

    int                     run();

private:
    int                     get_filenames();
    int                     load_input();
    int                     validate_name(
                                  std::string const & what
                                , std::string & name
                                , bool start_end_underscore = false);
    int                     extract_value(
                                  std::string const & name
                                , std::string const & value
                                , int64_t & id
                                , std::string & result);
    int                     generate_files();

    advgetopt::getopt       f_opt;
    bool                    f_verbose = false;
    std::string             f_filename = std::string();
    std::string             f_basename = std::string();
    std::string             f_output_path = std::string();
    advgetopt::conf_file::pointer_t
                            f_names = advgetopt::conf_file::pointer_t();
    std::string             f_introducer = "atomic_name";
    std::string             f_project = std::string();
    std::string             f_sub_project = std::string();
};


atomic_names::atomic_names(int argc, char * argv[])
    : f_opt(g_options_environment, argc, argv)
{
    f_verbose = f_opt.is_defined("verbose");
}


int atomic_names::run()
{
    int r(0);

    r = get_filenames();
    if(r != 0)
    {
        return r;
    }

    r = load_input();
    if(r != 0)
    {
        return r;
    }

    r = generate_files();
    if(r != 0)
    {
        return r;
    }

    return 0;
}


int atomic_names::get_filenames()
{
    if(f_verbose)
    {
        std::cout << "info: get filename.\n";
    }

    if(!f_opt.is_defined("--"))
    {
        std::cerr << "error: a <filename> is required.\n";
        return 1;
    }
    f_filename = f_opt.get_string("--");
    if(f_filename.empty())
    {
        std::cerr << "error: <filename> requires a non-empty name.\n";
        return 1;
    }

    f_basename = snapdev::pathinfo::basename(f_filename, ".*");
    if(f_basename.empty())
    {
        std::cerr
            << "error: somehow the basename of \""
            << f_filename
            << "\" is an empty string.\n";
        return 1;
    }

    f_output_path = f_opt.get_string("output-path");
    if(f_output_path.empty())
    {
        std::cerr << "error: the --output-path command line option requires a non-empty name.\n";
        return 1;
    }

    return 0;
}


int atomic_names::load_input()
{
    if(f_verbose)
    {
        std::cout
            << "info: load input \""
            << f_filename
            << "\".\n";
    }

    advgetopt::conf_file_setup setup(f_filename);
    f_names = advgetopt::conf_file::get_conf_file(setup);
    if(f_names == nullptr)
    {
        std::cerr
            << "error: could not read input file \""
            << f_filename
            << "\".\n";
        return 1;
    }

    return 0;
}


int atomic_names::validate_name(
      std::string const & what
    , std::string & name
    , bool start_end_underscore)
{
    if(name.empty())
    {
        std::cerr << "error: " << what << " cannot be empty.\n";
        return 1;
    }

    for(char const * n(name.c_str()); *n != '\0'; ++n)
    {
        if(*n == '-')
        {
            name[n - name.c_str()] = '_';
        }
        else if((*n < 'A' || *n > 'Z')
             && (*n < 'a' || *n > 'z')
             && (*n < '0' || *n > '9')
             && *n != '_')
        {
            std::cerr
                << "error: "
                << what
                << " includes unexpected characters in \""
                << name
                << "\".\n";
            return 1;
        }
    }

    if(!start_end_underscore
    && (name.front() == '_' || name.back() == '_'))
    {
        std::cerr
            << "error: "
            << what
            << " cannot start and/or end with an underscore in \""
            << name
            << "\".\n";
        return 1;
    }

    return 0;
}


int atomic_names::extract_value(
      std::string const & name
    , std::string const & value
    , int64_t & id
    , std::string & result)
{
    // check for an identifier
    //
    // note: it is not an error if not present; it will be given a default
    //       number when that happens
    //
    char const * v(value.c_str());
    int64_t identifier(0);
    bool found_digits(false);
    for(; *v != '\0'; ++v)
    {
        if(*v < '0' || *v > '9')
        {
            break;
        }
        identifier *= 10;
        identifier += *v - '0';
        found_digits = true;
    }
    if(!found_digits
    || *v != ':')
    {
        v = value.c_str();
    }
    else
    {
        id = identifier;
        ++v;
    }

    // the value may be quoted
    //
    std::string const str(advgetopt::unquote(v));
    if(str.empty())
    {
        std::cerr
            << "error: empty values are not currently allowed (parameter \""
            << name
            << "\").\n";
        return 1;
    }

    for(auto const & c : str)
    {
        switch(c)
        {
        case '\0':
            std::cerr
                << "error: found a NUL character in \""
                << name
                << "\".\n";
            return 1;

        case '"':
            result += '\\';
            result += '"';
            break;

        case '\r':
            result += '\\';
            result += 'r';
            break;

        case '\n':
            result += '\\';
            result += 'n';
            break;

        case '\t':
            result += '\\';
            result += 't';
            break;

        default:
            if(c < ' ')
            {
                result += '\\';
                result += ((c >> 6) & 0x03) + '0';
                result += ((c >> 3) & 0x07) + '0';
                result += ((c >> 6) & 0x07) + '0';
            }
            else
            {
                result += c;
            }
            break;

        }
    }

    return 0;
}


int atomic_names::generate_files()
{
    int r(0);

    if(f_verbose)
    {
        std::cout << "info: generate files.\n";
    }

    if(f_names->has_parameter("introducer"))
    {
        f_introducer = f_names->get_parameter("introducer");
        r = validate_name("introducer", f_introducer);
        if(r != 0)
        {
            return r;
        }
    }

    if(!f_names->has_parameter("project"))
    {
        std::cerr << "error: the \"project=...\" parameter is mandatory.\n";
        return 1;
    }
    f_project = f_names->get_parameter("project");
    r = validate_name("project", f_project);
    if(r != 0)
    {
        return r;
    }

    if(f_names->has_parameter("sub_project"))
    {
        f_sub_project = f_names->get_parameter("sub_project");
        r = validate_name("sub_project", f_sub_project);
        if(r != 0)
        {
            return r;
        }
    }
    else
    {
        f_sub_project = std::string();
    }

    std::stringstream cpp;
    std::stringstream h;

    h
        << "// DO NOT EDIT, see `man atomic-names` for details\n"
           "#pragma once\n"
           "namespace " << f_project << " {\n"
        << (f_sub_project.empty()
                ? std::string()
                : "namespace " + f_sub_project + " {\n");

    cpp
        << "// DO NOT EDIT, see `man atomic-names` for details\n"
           "#include \"./" << f_basename << ".h\"\n"
           "namespace " << f_project << " {\n"
        << (f_sub_project.empty()
                ? std::string()
                : "namespace " + f_sub_project + " {\n");

    advgetopt::conf_file::parameters_t parameters(f_names->get_parameters());
    for(auto const & p : parameters)
    {
        if(p.first.length() < 7)
        {
            continue;
        }
        std::string::size_type const pos(p.first.find("::"));
        if(pos == std::string::npos)
        {
            continue;
        }
        std::string const scope(p.first.substr(0, pos));
        bool is_public(true);
        if(scope != "public")
        {
            if(scope != "private")
            {
                continue;
            }
            is_public = false;
        }
        std::string name(p.first.substr(pos + 2));
        if(name.empty())
        {
            // as far as I know, this cannot happen
            //
            std::cerr
                << "error: empty names are not allowed.\n";
            return 1;
        }
        validate_name("name", name);

        int64_t id(-1);
        std::string value;
        r = extract_value(name, p.second, id, value);
        if(r != 0)
        {
            return r;
        }

        h
            << "extern char const * g_"
            << f_introducer
            << '_'
            << (f_sub_project.empty() ? f_project : f_sub_project)
            << '_'
            << name
            << ";\n";

        cpp << "char const * g_"
            << f_introducer
            << '_'
            << (f_sub_project.empty() ? f_project : f_sub_project)
            << '_'
            << name
            << " = \""
            << value
            << "\";\n";
    }

    if(!f_sub_project.empty())
    {
        h << "}\n";         // namespace sub-project
        cpp << "}\n";       // namespace sub-project
    }
    h << "}\n";             // namespace project
    cpp << "}\n";           // namespace project

    if(f_verbose)
    {
        std::cout
            << "info: save to \""
            << f_output_path
            << '/'
            << f_basename
            << "{.cpp,.h,_private.h}\".\n";
    }

    snapdev::file_contents header_public(f_output_path + "/" + f_basename + ".h");
    header_public.contents(h.str());
    if(!header_public.write_all())
    {
        std::cerr << "error: could not save public header file to \""
            << header_public.filename()
            << ".h\".\n";
        return 1;
    }

    snapdev::file_contents cpp_file(f_output_path + "/" + f_basename + ".cpp");
    cpp_file.contents(cpp.str());
    if(!cpp_file.write_all())
    {
        std::cerr << "error: could not save C++ implementation file to \""
            << cpp_file.filename()
            << ".cpp\".\n";
        return 1;
    }

    return 0;
}




}
// noname namespace



int main(int argc, char * argv[])
{
    try
    {
        atomic_names n(argc, argv);
        return n.run();
    }
    catch(libexcept::exception_t const & e)
    {
        std::cerr
            << "error: a libexcept exception occurred: \""
            << e.what()
            << "\".\n";
    }
    catch(std::exception const & e)
    {
        std::cerr
            << "error: a standard exception occurred: \""
            << e.what()
            << "\".\n";
    }
    catch(...)
    {
        std::cerr << "error: an unknown exception occurred.\n";
    }

    return 1;
}



// vim: ts=4 sw=4 et
