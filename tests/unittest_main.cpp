/*
 * File:
 *    tests/unittest_main.cpp
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
 */

// Tell catch we want it to add the runner code in this file.
#define CATCH_CONFIG_RUNNER

// self
//
#include "unittest_main.h"

// advgetopt lib
//
#include "advgetopt/advgetopt.h"
#include "advgetopt/version.h"

// C++ lib
//
#include <sstream>


namespace unittest
{
    std::string     g_tmp_dir;
    bool            g_verbose;
}


namespace
{
    struct UnitTestCLData
    {
        bool        help = false;
        int         seed = 0;
        std::string tmp = std::string();
        bool        verbose = false;
        bool        version = false;
    };

    void remove_from_args( std::vector<std::string> & vect, std::string const & long_opt, std::string const & short_opt )
    {
        auto iter = std::find_if( vect.begin(), vect.end(), [long_opt, short_opt]( std::string const & arg )
        {
            return arg == long_opt || arg == short_opt;
        });
        if( iter != vect.end() )
        {
            auto next_iter = iter;
            vect.erase( ++next_iter );
            vect.erase( iter );
        }
    }
}
// namespace


int unittest_main(int argc, char * argv[])
{
    UnitTestCLData configData;
    Catch::Clara::CommandLine<UnitTestCLData> cli;

    cli["-?"]["-h"]["--help"]
        .describe("display usage information")
        .bind(&UnitTestCLData::help);

    cli["-S"]["--seed"]
        .describe("value to seed the randomizer, if not specified, randomize")
        .bind(&UnitTestCLData::seed, "the seed value");

    cli["-T"]["--tmp"]
        .describe("path to a temporary directory")
        .bind(&UnitTestCLData::tmp, "path");

    cli["-v"]["--verbose"]
        .describe("make the test more verbose")
        .bind(&UnitTestCLData::verbose);

    cli["-V"]["--version"]
        .describe("print out the advgetopt library version these unit tests pertain to")
        .bind(&UnitTestCLData::version);

    cli.parseInto( argc, argv, configData );

    if( configData.help )
    {
        cli.usage( std::cout, argv[0] );
        Catch::Session().run(argc, argv);
        exit(1);
    }

    if( configData.version )
    {
        std::cout << LIBADVGETOPT_VERSION_STRING << std::endl;
        exit(0);
    }

    std::vector<std::string> arg_list;
    for( int i = 0; i < argc; ++i )
    {
        arg_list.push_back( argv[i] );
    }

    // by default we get a different seed each time; that really helps
    // in detecting errors! (I know, I wrote loads of tests before)
    unsigned int seed(static_cast<unsigned int>(time(NULL)));
    if( configData.seed != 0 )
    {
        seed = static_cast<unsigned int>(configData.seed);
        remove_from_args( arg_list, "--seed", "-S" );
    }
    srand(seed);
    std::cout << "advgetopt[" << getpid() << "]:unittest: seed is " << seed << std::endl;

    // we can only have one of those for ALL the tests that directly
    // access the library...
    // (because the result is cached and thus cannot change)

    if( !configData.tmp.empty() )
    {
        unittest::g_tmp_dir = configData.tmp;
        remove_from_args( arg_list, "--tmp", "-T" );

        if(unittest::g_tmp_dir == "/tmp")
        {
            std::cerr << "fatal error: you must specify a sub-directory for your temporary directory such as /tmp/advgetopt";
            exit(1);
        }
    }
    else
    {
        unittest::g_tmp_dir = "/tmp/advgetopt";
    }

    unittest::g_verbose = configData.verbose;
    if( unittest::g_verbose )
    {
        remove_from_args( arg_list, "--verbose", "-v" );
    }

    // delete the existing directory
    {
        std::stringstream ss;
        ss << "rm -rf \"" << unittest::g_tmp_dir << "\"";
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: could not delete temporary directory \"" << unittest::g_tmp_dir << "\".";
            exit(1);
        }
    }
    // then re-create the directory
    {
        std::stringstream ss;
        ss << "mkdir -p \"" << unittest::g_tmp_dir << "\"";
        if(system(ss.str().c_str()) != 0)
        {
            std::cerr << "fatal error: could not create temporary directory \"" << unittest::g_tmp_dir << "\".";
            exit(1);
        }
    }

    std::vector<char *> new_argv;
    std::for_each( arg_list.begin(), arg_list.end(), [&new_argv]( const std::string& arg )
    {
        new_argv.push_back( const_cast<char *>(arg.c_str()) );
    });

    return Catch::Session().run( static_cast<int>(new_argv.size()), &new_argv[0] );
}


int main(int argc, char * argv[])
{
    int r(1);

    try
    {
        r = unittest_main(argc, argv);
    }
    catch(std::logic_error const & e)
    {
        std::cerr << "fatal error: caught a logic error in advgetopt unit tests: " << e.what() << "\n";
    }

    return r;
}

// vim: ts=4 sw=4 et
