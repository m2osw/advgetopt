/*
 * File:
 *    tests/main.cpp
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
#include "main.h"

// advgetopt lib
//
#include "advgetopt/advgetopt.h"
#include "advgetopt/version.h"

// libexcept lib
//
#include "libexcept/exception.h"

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
        int         seed = 0;
        std::string tmp = std::string();
        bool        progress = false;
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

    Catch::Session session;

    auto cli = session.cli()
             | Catch::clara::Opt(configData.progress)
                ["-p"]["--progress"]
                ("print name of test section being run")
             | Catch::clara::Opt(configData.seed, "seed")
                ["-S"]["--seed"]
                ("value to seed the randomizer, if not specified, randomize")
             | Catch::clara::Opt(configData.tmp, "tmp")
                ["-T"]["--tmp"]
                ("path to a temporary directory")
             | Catch::clara::Opt(configData.version)
                ["-V"]["--version"]
                ("print out the advgetopt library version these unit tests pertain to");

    session.cli(cli);

    auto result(session.applyCommandLine(argc, argv));
    if(result != 0)
    {
        std::cerr << "Error in command line." << std::endl;
        exit(1);
    }

    if( configData.version )
    {
        std::cout << LIBADVGETOPT_VERSION_STRING << std::endl;
        exit(0);
    }

    // by default we get a different seed each time; that really helps
    // in detecting errors! (I know, I wrote loads of tests before)
    //
    unsigned int seed(static_cast<unsigned int>(time(NULL)));
    if( configData.seed != 0 )
    {
        seed = static_cast<unsigned int>(configData.seed);
    }
    srand(seed);
    std::cout << "advgetopt v" LIBADVGETOPT_VERSION_STRING " [" << getpid() << "]:unittest: seed is " << seed << std::endl;

    // we can only have one of those for ALL the tests that directly
    // access the library...
    // (because the result is cached and thus cannot change)

    if( !configData.tmp.empty() )
    {
        unittest::g_tmp_dir = configData.tmp;
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

    unittest::g_verbose = configData.progress;

    // delete the existing tmp directory
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

    advgetopt::set_log_callback(unittest::log_for_test);

    //wpkg_filename::uri_filename config("~/.config/advgetopt/advgetopt.conf");
    //if(config.exists())
    //{
    //    fprintf(stderr, "\nerror:unittest_advgetopt: ~/.config/advgetopt/advgetopt.conf already exists, the advgetopt tests would not work as expected with such. Please delete or rename that file.\n");
    //    throw std::runtime_error("~/.config/advgetopt/advgetopt.conf already exists");
    //}
    const char *options(getenv("ADVGETOPT_TEST_OPTIONS"));
    if(options != nullptr && *options != '\0')
    {
        std::cerr << std::endl << "error:unittest_advgetopt: ADVGETOPT_TEST_OPTIONS already exists, the advgetopt tests would not work as expected with such. Please unset that environment variable and try again." << std::endl;
        throw std::runtime_error("ADVGETOPT_TEST_OPTIONS already exists");
    }

    return session.run();
}


int main(int argc, char * argv[])
{
    int r(1);

    try
    {
        libexcept::set_collect_stack(false);
        r = unittest_main(argc, argv);
    }
    catch(std::logic_error const & e)
    {
        std::cerr << "fatal error: caught a logic error in advgetopt unit tests: " << e.what() << "\n";
    }

    return r;
}

// vim: ts=4 sw=4 et
