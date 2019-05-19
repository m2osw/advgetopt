/*
 * Files:
 *    tests/unittest_advgetopt.cpp
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

// self
//
#include "unittest_main.h"

// advgetopt lib
//
#include "advgetopt/advgetopt.h"
#include "advgetopt/exception.h"
#include "advgetopt/log.h"

// C++ lib
//
#include <cstring>
#include <cmath>
#include <sstream>
#include <fstream>

// C lib
//
#include <time.h>


namespace
{


std::vector<std::string>        g_expected_logs = std::vector<std::string>();

void log_for_test(advgetopt::log_level_t level, std::string const & message)
{
    if(unittest::g_verbose)
    {
        std::cerr << "logger sent:\n"
                  << advgetopt::to_string(level)
                  << ": "
                  << message
                  << std::endl;
    }

    // at this time it's impossible to debug the location of the empty
    // problem without a proper stack trace...
    //
    if(g_expected_logs.empty())
    {
        libexcept::stack_trace_t trace(libexcept::collect_stack_trace_with_line_numbers());
        std::cerr << "*** STACK TRACE ***" << std::endl;
        for(auto const & l : trace)
        {
            std::cerr << l << std::endl;
        }
        std::cerr << "***" << std::endl;
    }

    CATCH_REQUIRE_FALSE(g_expected_logs.empty());

    std::stringstream ss;
    ss << advgetopt::to_string(level) << ": " << message;

    // again, the REQUIRE() is not going to be useful in terms of line number
    //
    if(g_expected_logs[0] != ss.str())
    {
        libexcept::stack_trace_t trace(libexcept::collect_stack_trace_with_line_numbers());
        std::cerr << "*** STACK TRACE ***" << std::endl;
        for(auto const & l : trace)
        {
            std::cerr << l << std::endl;
        }
        std::cerr << "***" << std::endl;
    }

    std::string expected_msg(g_expected_logs[0]);
    g_expected_logs.erase(g_expected_logs.begin());

    CATCH_REQUIRE(expected_msg == ss.str());
}


} // no name namespace


class AdvGetOptUnitTests
{
public:
    AdvGetOptUnitTests();

    void invalid_parameters();
    void valid_config_files();
    void valid_config_files_extra();
};


AdvGetOptUnitTests::AdvGetOptUnitTests()
{
    advgetopt::set_log_callback(log_for_test);

    //wpkg_filename::uri_filename config("~/.config/wpkg/wpkg.conf");
    //if(config.exists())
    //{
    //    fprintf(stderr, "\nerror:unittest_advgetopt: ~/.config/wpkg/wpkg.conf already exists, the advgetopt tests would not work as expected with such. Please delete or rename that file.\n");
    //    throw std::runtime_error("~/.config/wpkg/wpkg.conf already exists");
    //}
    const char *options(getenv("ADVGETOPT_TEST_OPTIONS"));
    if(options != nullptr && *options != '\0')
    {
        std::cerr << std::endl << "error:unittest_advgetopt: ADVGETOPT_TEST_OPTIONS already exists, the advgetopt tests would not work as expected with such. Please unset that environment variable and try again." << std::endl;
        throw std::runtime_error("ADVGETOPT_TEST_OPTIONS already exists");
    }
}


void AdvGetOptUnitTests::invalid_parameters()
{
    std::cout << std::endl << "Advanced GetOpt Output (expected until the test fails):" << std::endl;
    // default arguments
    char const * cargv[] =
    {
        "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
        "--ignore-parameters",
        nullptr
    };
    const int argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
    char **argv = const_cast<char **>(cargv);

    // no options available
    const advgetopt::option options_empty_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (empty list)",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_empty;
    options_empty.f_project_name = "unittest";
    options_empty.f_options = options_empty_list;
    options_empty.f_help_header = "Usage: try this one and we get a throw (empty list)";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_empty, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // option without a name and "wrong" type
    const advgetopt::option options_no_name_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (no name)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_REQUIRED,
            nullptr,
            "we can have a default though",
            nullptr,
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_no_name;
    options_no_name.f_project_name = "unittest";
    options_no_name.f_options = options_no_name_list;
    options_no_name.f_help_header = "Usage: try this one and we get a throw (no name)";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_no_name, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // long options must be 2+ characters
    const advgetopt::option options_2chars_minimum_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (2 chars minimum)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_MULTIPLE,
            "", // cannot be empty string (use nullptr instead)
            nullptr,
            "long option must be 2 characters long at least",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_2chars_minimum;
    options_2chars_minimum.f_project_name = "unittest";
    options_2chars_minimum.f_options = options_2chars_minimum_list;
    options_2chars_minimum.f_help_header = "Usage: try this one and we get a throw (2 chars minimum)";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_2chars_minimum, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // long options must be 2+ characters
    const advgetopt::option options_2chars_minimum2_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (2 chars minimum 2nd)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_MULTIPLE,
            "f", // cannot be 1 character
            nullptr,
            "long option must be 2 characters long at least",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_2chars_minimum2;
    options_2chars_minimum2.f_project_name = "unittest";
    options_2chars_minimum2.f_options = options_2chars_minimum2_list;
    options_2chars_minimum2.f_help_header = "Usage: try this one and we get a throw (2 chars minimum 2nd)";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_2chars_minimum2, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // same long option defined twice
    const advgetopt::option options_defined_twice_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (long defined twice)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_REQUIRED,
            "filename",
            nullptr,
            "options must be unique",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_REQUIRED,
            "filename", // copy/paste problem maybe?
            nullptr,
            "options must be unique",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_defined_twice;
    options_defined_twice.f_project_name = "unittest";
    options_defined_twice.f_options = options_defined_twice_list;
    options_defined_twice.f_help_header = "Usage: try this one and we get a throw (long defined twice)";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_defined_twice, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // same short option defined twice
    const advgetopt::option options_short_defined_twice_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (short defined twice)",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            'f',
            advgetopt::GETOPT_FLAG_REQUIRED,
            "from",
            nullptr,
            "options must be unique",
            nullptr
        },
        {
            'f',
            advgetopt::GETOPT_FLAG_REQUIRED,
            "to",
            nullptr,
            "options must be unique",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_short_defined_twice;
    options_short_defined_twice.f_project_name = "unittest";
    options_short_defined_twice.f_options = options_short_defined_twice_list;
    options_short_defined_twice.f_help_header = "Usage: try this one and we get a throw (short defined twice)";
    options_short_defined_twice.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_short_defined_twice, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // 2 default_multiple_argument's in the same list is invalid
    const advgetopt::option options_two_default_multiple_arguments_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (two defaults, multiple args)",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
            "filenames",
            nullptr,
            "other parameters are viewed as filenames",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
            "more",
            nullptr,
            "yet other parameters are viewed as \"more\" data--here it breaks, one default max.",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_two_default_multiple_arguments;
    options_two_default_multiple_arguments.f_project_name = "unittest";
    options_two_default_multiple_arguments.f_options = options_two_default_multiple_arguments_list;
    options_two_default_multiple_arguments.f_help_header = "Usage: try this one and we get a throw (two defaults by flag, multiple args)";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_two_default_multiple_arguments, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // 2 default_argument's in the same list is invalid
    const advgetopt::option options_two_default_arguments_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (two default args)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            0,
            "--",
            nullptr,
            "one other parameter is viewed as a filename",
            nullptr
        },
        {
            '\0',
            0,
            "--",
            nullptr,
            "yet other parameter viewed as \"more\" data--here it breaks, one default max.",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_two_default_arguments;
    options_two_default_arguments.f_project_name = "unittest";
    options_two_default_arguments.f_options = options_two_default_arguments_list;
    options_two_default_arguments.f_help_header = "Usage: try this one and we get a throw (two default args by name)";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_two_default_arguments, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // mix of default arguments in the same list is invalid
    const advgetopt::option options_mix_of_default_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (mix of defaults)",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_MULTIPLE,
            "--",
            nullptr,
            "other parameters are viewed as filenames",
            nullptr
        },
        {
            '\0',
            0,
            "--",
            nullptr,
            "yet other parameter viewed as \"more\" data--here it breaks, one default max.",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_mix_of_default;
    options_mix_of_default.f_project_name = "unittest";
    options_mix_of_default.f_options = options_mix_of_default_list;
    options_mix_of_default.f_help_header = "Usage: try this one and we get a throw (mix flags of defaults by name)";
    options_mix_of_default.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options_mix_of_default, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // try the - and -- without a default in the arguments
    const advgetopt::option options_no_defaults_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (no defaults)",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "just a flag to test.",
            0
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_no_defaults;
    options_no_defaults.f_project_name = "unittest";
    options_no_defaults.f_options = options_no_defaults_list;
    options_no_defaults.f_help_header = "Usage: try this one and we get a throw (no defaults)";

    {
        // a filename by itself is a problem when there is no default
        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "this",
            "filename",
            nullptr
        };
        const int sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv(const_cast<char **>(sub_cargv));

        g_expected_logs.push_back("error: no default options defined; we do not know what to do of \"this\"; standalone parameters are not accepted by this program.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }
    {
        // a '-' by itself is a problem when there is no default because it
        // is expected to represent a filename (stdin)
        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "-",
            nullptr
        };
        const int sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char **sub_argv(const_cast<char **>(sub_cargv));

        g_expected_logs.push_back("error: no default options defined; thus - is not accepted by this program.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }
    {
        // the -- by itself would be fine, but since it represents a
        // transition from arguments to only filenames (or whatever the
        // program expects as default options) it generates an error if
        // no default options are accepted
        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "--", // already just by itself it causes problems
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        g_expected_logs.push_back("error: no default options defined; thus -- is not accepted by this program.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }
    {
        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "--",
            "66",
            "--filenames",
            "extra",
            "--file",
            "names",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        g_expected_logs.push_back("error: no default options defined; thus -- is not accepted by this program.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }
    {
        // check that -v, that does not exist, generates a usage error
        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "-v",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        g_expected_logs.push_back("error: option -v is not supported.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }

    // check -- when default does not allowed environment variables
    const advgetopt::option options_no_defaults_in_envvar_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (no defaults in envvar)",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "just a flag to test.",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
            "--",
            nullptr,
            "default multiple filenames",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_no_defaults_in_envvar;
    options_no_defaults_in_envvar.f_project_name = "unittest";
    options_no_defaults_in_envvar.f_options = options_no_defaults_in_envvar_list;
    options_no_defaults_in_envvar.f_help_header = "Usage: try this one and we get a throw (no defaults in envvar)";
    options_no_defaults_in_envvar.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    {
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS=--verbose - no default here");
        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "-",
            "here",
            "it",
            "works",
            nullptr
        };
        int const sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        g_expected_logs.push_back("error: option - is not supported in the environment variable.");
        advgetopt::getopt opt(options_no_defaults_in_envvar, sub_argc, sub_argv);
    }
    {
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS=--verbose no default here");
        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "-",
            "here",
            "it",
            "works",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        g_expected_logs.push_back("error: default options are not supported in the environment variable.");
        advgetopt::getopt opt(options_no_defaults_in_envvar, sub_argc, sub_argv);
    }
    {
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS=--verbose -- foo bar blah");
        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "here",
            "it",
            "works",
            "--",
            "66",
            "--filenames",
            "extra",
            "--file",
            "names",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        g_expected_logs.push_back("error: option -- is not supported in the environment variable.");
        advgetopt::getopt opt(options_no_defaults_in_envvar, sub_argc, sub_argv);
    }

    // unnknown long options
    const advgetopt::option valid_options_unknown_command_line_option_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (unknown command line option)",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
            "--command",
            nullptr,
            "there is a --command, but the user tries --verbose!",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment valid_options_unknown_command_line_option;
    valid_options_unknown_command_line_option.f_project_name = "unittest";
    valid_options_unknown_command_line_option.f_options = valid_options_unknown_command_line_option_list;
    valid_options_unknown_command_line_option.f_help_header = "Usage: try this one and we get a throw (unknown command line option)";

    {
        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        g_expected_logs.push_back("error: option --verbose is not supported.");
        advgetopt::getopt opt(valid_options_unknown_command_line_option, sub_argc, sub_argv);
    }

    // illegal short or long option in variable
    const advgetopt::option options_illegal_in_variable_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (illegal in variable)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            'v',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "just a flag to test.",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment options_illegal_in_variable;
    options_illegal_in_variable.f_project_name = "unittest";
    options_illegal_in_variable.f_options = options_illegal_in_variable_list;
    options_illegal_in_variable.f_help_header = "Usage: try this one and we get a throw (illegal in variable)";
    options_illegal_in_variable.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    {
        // long
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS=--verbose");

        g_expected_logs.push_back("error: option --verbose is not supported in the environment variable.");
        g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
        advgetopt::getopt opt(options_illegal_in_variable, argc, argv);
    }
    {
        // short
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS=-v");

        g_expected_logs.push_back("error: option -v is not supported in the environment variable.");
        g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
        advgetopt::getopt opt(options_illegal_in_variable, argc, argv);
    }

    // configuration file options must have a long name
    const advgetopt::option configuration_long_name_missing_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (long name missing)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            'c',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE,
            nullptr,
            nullptr,
            "a valid option",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment configuration_long_name_missing;
    configuration_long_name_missing.f_project_name = "unittest";
    configuration_long_name_missing.f_options = configuration_long_name_missing_list;
    configuration_long_name_missing.f_help_header = "Usage: try this one and we get a throw (long name missing)";
    configuration_long_name_missing.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    {
        CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(configuration_long_name_missing, argc, argv); }, advgetopt::getopt_exception_logic);
    }

    // create invalid configuration files
    const advgetopt::option valid_options_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: try this one and we get a throw (valid options!)",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE,
            "valid",
            nullptr,
            "a valid option",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE,
            "command",
            nullptr,
            "a valid command, but not a valid configuration option",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
            "filename",
            nullptr,
            "other parameters are viewed as filenames",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    std::string tmpdir(unittest::g_tmp_dir);
    tmpdir += "/.config";
    std::stringstream ss;
    ss << "mkdir -p " << tmpdir;
    if(system(ss.str().c_str()) != 0)
    {
        std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
        exit(1);
    }

    std::string const config_filename(tmpdir + "/advgetopt.config");
    char const * invalid_confs[] =
    {
        config_filename.c_str(),
        nullptr
    };

    advgetopt::options_environment valid_options;
    valid_options.f_project_name = "unittest";
    valid_options.f_options = valid_options_list;
    valid_options.f_help_header = "Usage: try this one and we get a throw (valid options!)";
    valid_options.f_configuration_files = invalid_confs;

    {
        // '=' operator missing
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "valid param\n"
                "# no spaces acceptable in param names\n"
            ;
        }
        {
            g_expected_logs.push_back("error: option name from \"valid param\" on line 2 in configuration file \"/tmp/advgetopt/.config/advgetopt.config\" cannot include a space, missing assignment operator?");
            g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(valid_options, argc, argv);
        }
    }
    {
        // same effect with a few extra spaces
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                " valid param \n"
                "# no spaces acceptable in param names\n"
            ;
        }
        {
            g_expected_logs.push_back("error: option name from \" valid param \" on line 2 in configuration file \"/tmp/advgetopt/.config/advgetopt.config\" cannot include a space, missing assignment operator?");
            g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(valid_options, argc, argv);
        }
    }
    {
        // param name missing
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                " = valid param\n"
                "# no spaces acceptable in param names\n"
            ;
        }
        {
            g_expected_logs.push_back("error: no option name in \" = valid param\" on line 2 from configuration file \"/tmp/advgetopt/.config/advgetopt.config\", missing name before = sign?");
            g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(valid_options, argc, argv);
        }
    }
    {
        // param name starts with a dash or more
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "--valid=param\n"
                "# no spaces acceptable in param names\n"
            ;
        }
        {
            g_expected_logs.push_back("error: option names in configuration files cannot start with a dash in \"--valid=param\" on line 2 from configuration file \"/tmp/advgetopt/.config/advgetopt.config\".");
            g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(valid_options, argc, argv);
        }
    }
    {
        // unknown param name
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "invalid=param\n"
                "# no spaces acceptable in param names\n"
            ;
        }
        {
            g_expected_logs.push_back("error: unknown option \"invalid\" found in configuration file \"/tmp/advgetopt/.config/advgetopt.config\".");
            g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(valid_options, argc, argv);
        }
    }
    {
        // known command, not valid in configuration file
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "command=value\n"
                "# no spaces acceptable in param names\n"
            ;
        }
        {
            g_expected_logs.push_back("error: option \"command\" is not supported in configuration files (found in \"/tmp/advgetopt/.config/advgetopt.config\").");
            g_expected_logs.push_back("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(valid_options, argc, argv);
        }
    }

#if 0
// in version 2.x we do not have this case anymore

    // one of the options has an invalid mode; explicit option
    {
        const advgetopt::option options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: one of the options has an invalid mode",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
                "mode",
                nullptr,
                "an argument with an invalid mode to see that we get an exception",
                static_cast<advgetopt::getopt::argument_mode_t>(static_cast<int>(advgetopt::getopt::argument_mode_t::end_of_options) + 1)
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: one of the options has an invalid mode";

        {
            const char *cargv2[] =
            {
                "tests/unittests/unittest_advgetopt",
                "--mode",
                "test",
                nullptr
            };
            const int argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char **argv2 = const_cast<char **>(cargv2);

            // here we hit the one in add_options() (plural)
            // the one in add_option() is not reachable because it is called only
            // when a default option is defined and that means the mode is
            // correct
            CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(argc2, argv2, options, confs, nullptr); }, advgetopt::getopt_exception_invalid );
        }
        {
            const char *cargv2[] =
            {
                "tests/unittests/unittest_advgetopt",
                nullptr
            };
            const int argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char **argv2 = const_cast<char **>(cargv2);

            // this one checks that the --mode flag does indeed generate a
            // throw when not used on the command line but then gets shown
            // in the usage() function
            advgetopt::getopt opt(argc2, argv2, options, confs, nullptr);
            for(int i(static_cast<int>(advgetopt::getopt::status_t::no_error)); i <= static_cast<int>(advgetopt::getopt::status_t::fatal); ++i)
            {
                CATCH_REQUIRE_THROWS_AS( opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exception_invalid);
            }
        }
    }
#endif

    // a valid initialization, but not so valid calls afterward
    {
        advgetopt::option const options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (valid options, but not the calls after)",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                "validate",
                nullptr,
                "this is used to validate different things.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "long",
                nullptr,
                "used to validate that invalid numbers generate an error.",
                nullptr
            },
            {
                'o',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "out-of-bounds",
                nullptr,
                "valid values from 1 to 9.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-specified-and-no-default",
                nullptr,
                "test long without having used the option and no default.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-specified-with-invalid-default",
                "123abc",
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-specified-string-without-default",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "string",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "filename",
                nullptr,
                "other parameters are viewed as filenames",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };
        char const * cargv2[] =
        {
            "tests/unittests/unittest_advgetopt",
            "--validate",
            "--long",
            "123abc",
            "--out-of-bounds",
            "123",
            "--string",
            "string value",
            nullptr
        };
        int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
        char ** argv2 = const_cast<char **>(cargv2);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: one of the options has an invalid mode";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        advgetopt::getopt opt(options, argc2, argv2);

        // cannot get the default without a valid name!
        CATCH_REQUIRE_THROWS_AS( opt.get_default(""), advgetopt::getopt_exception_undefined);

        // cannot get a long named "blah"
        CATCH_REQUIRE_THROWS_AS( opt.get_long("blah"), advgetopt::getopt_exception_undefined);
        // existing "long", but only 1 entry
        CATCH_REQUIRE_THROWS_AS( opt.get_long("long", 100), advgetopt::getopt_exception_undefined);
        long l(-1);
        CATCH_REQUIRE_THROWS_AS( l = opt.get_long("not-specified-and-no-default", 0), advgetopt::getopt_exception_undefined);
        CATCH_REQUIRE(l == -1);
        CATCH_REQUIRE_THROWS_AS( l = opt.get_long("not-specified-with-invalid-default", 0), advgetopt::getopt_exception_invalid);
        CATCH_REQUIRE(l == -1);
        g_expected_logs.push_back("error: invalid number (123abc) in parameter --long.");
        l = opt.get_long("long");
        CATCH_REQUIRE(l == -1);
        g_expected_logs.push_back("error: 123 is out of bounds (1..9 inclusive) in parameter --out-of-bounds.");
        l = opt.get_long("out-of-bounds", 0, 1, 9);
        CATCH_REQUIRE(l == -1);
        std::string s;
        CATCH_REQUIRE_THROWS_AS( s = opt.get_string("not-specified-string-without-default", 0), advgetopt::getopt_exception_undefined);
        CATCH_REQUIRE(s.empty());
        CATCH_REQUIRE_THROWS_AS( s = opt.get_string("string", 100), advgetopt::getopt_exception_undefined);
        CATCH_REQUIRE(s.empty());

        // reuse all those invalid options with the reset() function
        // and expect the same result
        // (the constructor is expected to call reset() the exact same way)
        //  -- this changed in version 2.x; we can't reset the option definitions
        //
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_empty_list, confs, nullptr), advgetopt::getopt_exception_invalid);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_no_name_list, confs, nullptr), advgetopt::getopt_exception_invalid);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_2chars_minimum, confs, nullptr), advgetopt::getopt_exception_invalid);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_2chars_minimum2, confs, nullptr), advgetopt::getopt_exception_invalid);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_defined_twice, confs, "ADVGETOPT_TEST_OPTIONS"), advgetopt::getopt_exception_invalid);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_short_defined_twice, confs, nullptr), advgetopt::getopt_exception_invalid);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_two_default_multiple_arguments, confs, nullptr), advgetopt::getopt_exception_default);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_two_default_arguments, confs, "ADVGETOPT_TEST_OPTIONS"), advgetopt::getopt_exception_default);
        //CATCH_REQUIRE_THROWS_AS( opt.reset(argc, argv, options_mix_of_default, confs, nullptr), advgetopt::getopt_exception_default);
    }

    // valid initialization + usage calls
    {
        const advgetopt::option options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (valid options + usage calls)",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                "validate",
                nullptr,
                "this is used to validate different things.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "long",
                nullptr,
                "used to validate that invalid numbers generate an error.",
                nullptr
            },
            {
                'o',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "out-of-bounds",
                nullptr,
                "valid values from 1 to 9.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-specified-and-no-default",
                nullptr,
                "test long without having used the option and no default.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE,
                "not-specified-with-invalid-default",
                "123abc",
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-specified-string-without-default",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "string",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                'u',
                advgetopt::GETOPT_FLAG_COMMAND_LINE,
                "unique",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                'q',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
                "quiet",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "filename",
                nullptr,
                "other parameters are viewed as filenames.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };
        char const * cargv2[] =
        {
            "tests/unittests/unittest_advgetopt",
            "--validate",
            "--long",
            "123abc",
            "--out-of-bounds",
            "123",
            "--string",
            "string value",
            nullptr
        };
        int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
        char ** argv2 = const_cast<char **>(cargv2);

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls)";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        // this initialization works as expected
        advgetopt::getopt opt(options, argc2, argv2);

        // all of the following have the exiting exception
        CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) ==
"Usage: try this one and we get a throw (valid options + usage calls)\n"
"   --long <arg>               used to validate that invalid numbers generate an\n"
"                              error.\n"
"   --not-specified-and-no-default <arg>\n"
"                              test long without having used the option and no\n"
"                              default.\n"
"   --not-specified-string-without-default <arg>\n"
"                              test long with an invalid default value.\n"
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test long with an invalid default value.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      test long with an invalid default value.\n"
"   --string <arg>             test long with an invalid default value.\n"
"   --unique or -u [<arg>]     test long with an invalid default value.\n"
"   --validate                 this is used to validate different things.\n"
"   [default arguments]        other parameters are viewed as filenames.\n"
                        );
    }

    // valid initialization + usage calls with a few different options
    {
        const advgetopt::option options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (valid options + usage calls bis)",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                "validate",
                nullptr,
                "this is used to validate different things.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "long",
                nullptr,
                "used to validate that invalid numbers generate an error.",
                nullptr
            },
            {
                'o',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "out-of-bounds",
                nullptr,
                "valid values from 1 to 9.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-specified-and-no-default",
                nullptr,
                "test long without having used the option and no default.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE,
                "not-specified-with-invalid-default",
                "123abc",
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-specified-string-without-default",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "string",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                'u',
                advgetopt::GETOPT_FLAG_COMMAND_LINE,
                "unique",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                'q',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_MULTIPLE,
                "quiet",
                nullptr,
                "test long with an invalid default value.",
                nullptr
            },
            {
                'l',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
                "not-in-v2-though",
                nullptr,
                "long with just a letter.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE,
                "--",
                nullptr,
                "other parameters are viewed as filenames; and we need at least one option with a very long help to check that it wraps perfectly (we'd really need to get the output of the command and check that against what is expected because at this time the test is rather blind in that respect! FIXED IN v2!)",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (valid options + usage calls bis)";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            // make sure that --long (REQUIRED) fails if the
            // long value is not specified
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt",
                "--validate",
                "--long",
                "--out-of-bounds",
                "123",
                "--string",
                "string value",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            g_expected_logs.push_back("error: option --long expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
        {
            // again with the lone -l (no long name)
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt",
                "--validate",
                "-l",
                "--out-of-bounds",
                "123",
                "--string",
                "string value",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            g_expected_logs.push_back("error: option --not-in-v2-though expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
        {
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt",
                "--validate",
                "--long",
                "123abc",
                "--out-of-bounds",
                "123",
                "--string",
                "string value",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            // this initialization works as expected
            advgetopt::getopt opt(options, argc2, argv2);

            // all of the following have the exiting exception
//            for(int i(static_cast<int>(advgetopt::getopt::status_t::no_error)); i <= static_cast<int>(advgetopt::getopt::status_t::fatal); ++i)
//            {
//                CATCH_REQUIRE_THROWS_AS( opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exception_exiting);
//            }
            CATCH_REQUIRE(opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) ==
"Usage: try this one and we get a throw (valid options + usage calls bis)\n"
"   --long <arg>               used to validate that invalid numbers generate an\n"
"                              error.\n"
"   --not-in-v2-though or -l <arg>\n"
"                              long with just a letter.\n"
"   --not-specified-and-no-default <arg>\n"
"                              test long without having used the option and no\n"
"                              default.\n"
"   --not-specified-string-without-default <arg>\n"
"                              test long with an invalid default value.\n"
"   --not-specified-with-invalid-default <arg> {<arg>} (default is \"123abc\")\n"
"                              test long with an invalid default value.\n"
"   --out-of-bounds or -o <arg>\n"
"                              valid values from 1 to 9.\n"
"   --quiet or -q {<arg>}      test long with an invalid default value.\n"
"   --string <arg>             test long with an invalid default value.\n"
"   --unique or -u [<arg>]     test long with an invalid default value.\n"
"   --validate                 this is used to validate different things.\n"
"   [default argument]         other parameters are viewed as filenames; and we\n"
"                              need at least one option with a very long help to\n"
"                              check that it wraps perfectly (we'd really need to\n"
"                              get the output of the command and check that\n"
"                              against what is expected because at this time the\n"
"                              test is rather blind in that respect! FIXED IN\n"
"                              v2!)\n"
                        );
        }
    }

    // strange entry without a name
    {
        advgetopt::option const options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (strange empty entry!)",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR | advgetopt::GETOPT_FLAG_FLAG,
                nullptr,
                nullptr,
                "this entry has help, but no valid name...",
                nullptr
            },
            {
                'v',
                advgetopt::GETOPT_FLAG_FLAG,
                "verbose",
                nullptr,
                "show more stuff when found on the command line.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (strange empty entry!)";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            // this initialization works as expected
            CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options, argc2, argv2); }, advgetopt::getopt_exception_logic );

//            // all of the following have the exiting exception
//            for(int i(static_cast<int>(advgetopt::getopt::status_t::no_error)); i <= static_cast<int>(advgetopt::getopt::status_t::fatal); ++i)
//            {
//                CATCH_REQUIRE_THROWS_AS( opt.usage(static_cast<advgetopt::getopt::status_t>(i), "test no error, warnings, errors..."), advgetopt::getopt_exception_invalid);
//            }
//std::cout << "test usage output here? " << opt.usage(advgetopt::GETOPT_FLAG_SHOW_ALL) << "\n";
        }
    }

    // required multiple without arguments
    {
        advgetopt::option const options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (required multiple without args)",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                'f',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "filenames",
                nullptr,
                "test a required multiple without any arguments and fail.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (required multiple without args)";

        {
            // first with -f
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                "-f",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            g_expected_logs.push_back("error: option --filenames expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
        {
            // second with --filenames
            const char *cargv2[] =
            {
                "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                "--filenames",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            g_expected_logs.push_back("error: option --filenames expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
    }

    // required multiple without arguments, short name only
    {
        advgetopt::option const options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (required multiple without args + short name)",
            //    advgetopt::getopt::argument_mode_t::help_argument
            //},
            {
                'f',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED | advgetopt::GETOPT_FLAG_MULTIPLE,
                "filenames",
                nullptr,
                "test a required multiple without any arguments and fail.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (required multiple without args + short name)";

        {
            // -f only in this case
            char const * cargv2[] =
            {
                "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/test-with-an-empty-entry",
                "-f",
                nullptr
            };
            int const argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char ** argv2 = const_cast<char **>(cargv2);

            g_expected_logs.push_back("error: option --filenames expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
    }
}


void AdvGetOptUnitTests::valid_config_files()
{
    // default arguments
    const char *cargv[] =
    {
        "tests/unittests/AdvGetOptUnitTests::valid_config_files",
        "--valid-parameter",
        nullptr
    };
    const int argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
    char **argv = const_cast<char **>(cargv);

    //std::vector<std::string> empty_confs;

    std::string tmpdir(unittest::g_tmp_dir);
    tmpdir += "/.config";
    std::stringstream ss;
    ss << "mkdir -p " << tmpdir;
    if(system(ss.str().c_str()) != 0)
    {
        std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
        exit(1);
    }
    std::string const config_filename(tmpdir + "/advgetopt.config");

    //std::vector<std::string> confs;
    //confs.push_back(config_filename);
    char const * confs[] =
    {
        config_filename.c_str(),
        nullptr
    };

    char const * space_separators[] =
    {
        " ",
        "\t",
        nullptr
    };

    // some command line options to test against
    const advgetopt::option valid_options_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::getopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: test valid options",
        //    advgetopt::getopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE,
            "valid-parameter",
            nullptr,
            "a valid option",
            nullptr
        },
        {
            'v',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "a verbose like option, select it or not",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "not-specified",
            nullptr,
            "a verbose like option, but never specified anywhere",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "number",
            "111",
            "expect a valid number",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "string",
            "the default string",
            "expect a valid string",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_MULTIPLE,
            "filenames",
            "a.out",
            "expect multiple strings",
            space_separators
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment valid_options_no_confs;
    valid_options_no_confs.f_project_name = "unittest";
    valid_options_no_confs.f_options = valid_options_list;
    valid_options_no_confs.f_help_header = "Usage: test valid options";
    valid_options_no_confs.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    advgetopt::options_environment valid_options;
    valid_options.f_project_name = "unittest";
    valid_options.f_options = valid_options_list;
    valid_options.f_help_header = "Usage: test valid options";
    valid_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
    valid_options.f_configuration_files = confs;

    // test that a configuration files gets loaded as expected
    {
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number = 5\n"
                "string=     strange\n"
                "verbose\n"
                "filenames\t= foo bar blah\n"
            ;
        }

        advgetopt::getopt opt(valid_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 5);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "strange");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 3);

        // as we're at it, make sure that indices out of bounds generate an exception
        for(int i(-100); i <= 100; ++i)
        {
            if(i != 0 && i != 1 && i != 2)
            {
                CATCH_REQUIRE_THROWS_AS( opt.get_string("filenames", i), advgetopt::getopt_exception_undefined);
            }
        }

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // make sure that command line options have priority or are cumulative
    {
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number = 5\n"
                "string=     strange\n"
                "verbose\n"
                "filenames\t= foo bar blah\n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files",
            "--valid-parameter",
            "--number",
            "66",
            "--filenames",
            "extra",
            "file",
            "names",
            nullptr
        };
        int const sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 66);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "strange");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "file");
        CATCH_REQUIRE(opt.get_string("filenames", 5) == "names");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 6);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // repeat with ADVGETOPT_TEST_OPTIONS instead of a configuration file
    {
        // here we have verbose twice which should hit the no_argument case
        // in the add_option() function
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS= --verbose --number\t15\t--filenames foo bar blah --string weird -v");
        advgetopt::getopt opt(valid_options_no_confs, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 15);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "weird");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 3);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // test that the environment variable has priority over a configuration file
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=--number 501 --filenames more files"));

        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number=99\n"
                "string      =     strange\n"
                "verbose\n"
                "filenames =\tfoo\tbar \t blah\n"
            ;
        }
        advgetopt::getopt opt(valid_options, argc, argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 501);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "strange");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "more");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "files");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 5);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // test order: conf files, environment var, command line
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=--number 501 --filenames more files"));
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number=99\n"
                "string      =     strange\n"
                "verbose\n"
                "filenames =\tfoo\tbar \t blah\n"
            ;
        }

        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files",
            "--valid-parameter",
            "--string",
            "hard work",
            "--filenames",
            "extra",
            "file",
            "names",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 501);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "hard work");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "more");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "files");
        CATCH_REQUIRE(opt.get_string("filenames", 5) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames", 6) == "file");
        CATCH_REQUIRE(opt.get_string("filenames", 7) == "names");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 8);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }

    // test again, just in case: conf files, environment var, command line
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=--number 709 --filenames more files --string \"hard work in env\""));
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number=99\n"
                "string      =     strange\n"
                "verbose\n"
                "filenames =\tfoo\tbar \t blah\n"
            ;
        }

        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files",
            "--valid-parameter",
            "--filenames",
            "extra",
            "file",
            "names",
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 709);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "hard work in env");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames", 0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames", 1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames", 2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames", 3) == "more");
        CATCH_REQUIRE(opt.get_string("filenames", 4) == "files");
        CATCH_REQUIRE(opt.get_string("filenames", 5) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames", 6) == "file");
        CATCH_REQUIRE(opt.get_string("filenames", 7) == "names");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 8);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files");
    }
}


void AdvGetOptUnitTests::valid_config_files_extra()
{
    //std::vector<std::string> empty_confs;

    std::string tmpdir(unittest::g_tmp_dir);
    tmpdir += "/.config";
    std::stringstream ss;
    ss << "mkdir -p " << tmpdir;
    if(system(ss.str().c_str()) != 0)
    {
        std::cerr << "fatal error: creating sub-temporary directory \"" << tmpdir << "\" failed.\n";
        exit(1);
    }
    std::string const config_filename(tmpdir + "/advgetopt.config");

    //std::vector<std::string> confs;
    //confs.push_back(config_filename);
    char const * confs[] =
    {
        config_filename.c_str(),
        nullptr
    };

    char const * separator_spaces[] =
    {
        " ",
        "\t",
        nullptr
    };

    // new set of options to test the special "--" option
    advgetopt::option const valid_options_with_multiple_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: test valid options",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE,
            "valid-parameter",
            nullptr,
            "a valid option",
            nullptr
        },
        {
            'v',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "verbose",
            nullptr,
            "a verbose like option, select it or not",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "not-specified",
            nullptr,
            "a verbose like option, but never specified anywhere",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "number",
            "111",
            "expect a valid number",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_REQUIRED,
            "string",
            "the default string",
            "expect a valid string",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
            "filenames",
            "a.out",
            "expect multiple strings, may be used after the -- and - is added to it too",
            separator_spaces
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment valid_options_with_multiple;
    valid_options_with_multiple.f_project_name = "unittest";
    valid_options_with_multiple.f_options = valid_options_with_multiple_list;
    valid_options_with_multiple.f_help_header = "Usage: test valid options with multiple";
    valid_options_with_multiple.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";
    valid_options_with_multiple.f_configuration_files = confs;

    // yet again, just in case: conf files, environment var, command line
    {
        unittest::obj_setenv env(const_cast<char *>("ADVGETOPT_TEST_OPTIONS=- --verbose -- more files --string \"hard work in env\""));
        {
            std::ofstream config_file;
            config_file.open(config_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            CATCH_REQUIRE(config_file.good());
            config_file <<
                "# Auto-generated\n"
                "number      =\t\t\t\t1111\t\t\t\t\n"
                "string      =     strange    \n"
                " filenames =\tfoo\tbar \t blah \n"
            ;
        }

        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
            "--valid-parameter",
            "--",
            "extra",
            "-file",
            "names",
            "-", // copied as is since we're after --
            nullptr
        };
        int const sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char ** sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_options_with_multiple, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE_FALSE(opt.is_defined("invalid-parameter"));

        // the valid parameter
        CATCH_REQUIRE(opt.is_defined("valid-parameter"));
        CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
        CATCH_REQUIRE(opt.size("valid-parameter") == 1);

        // a valid number
        CATCH_REQUIRE(opt.is_defined("number"));
        CATCH_REQUIRE(opt.get_long("number") == 1111);
        CATCH_REQUIRE(opt.get_default("number") == "111");
        CATCH_REQUIRE(opt.size("number") == 1);

        // a valid string
        CATCH_REQUIRE(opt.is_defined("string"));
        CATCH_REQUIRE(opt.get_string("string") == "strange");
        CATCH_REQUIRE(opt.get_default("string") == "the default string");
        CATCH_REQUIRE(opt.size("string") == 1);

        // verbosity
        CATCH_REQUIRE(opt.is_defined("verbose"));
        CATCH_REQUIRE(opt.get_string("verbose") == "");
        CATCH_REQUIRE(opt.get_default("verbose").empty());
        CATCH_REQUIRE(opt.size("verbose") == 1);

        // filenames
        CATCH_REQUIRE(opt.is_defined("filenames"));
        CATCH_REQUIRE(opt.get_string("filenames") == "foo"); // same as index = 0
        CATCH_REQUIRE(opt.get_string("filenames",  0) == "foo");
        CATCH_REQUIRE(opt.get_string("filenames",  1) == "bar");
        CATCH_REQUIRE(opt.get_string("filenames",  2) == "blah");
        CATCH_REQUIRE(opt.get_string("filenames",  3) == "-");
        CATCH_REQUIRE(opt.get_string("filenames",  4) == "more");
        CATCH_REQUIRE(opt.get_string("filenames",  5) == "files");
        CATCH_REQUIRE(opt.get_string("filenames",  6) == "--string");
        CATCH_REQUIRE(opt.get_string("filenames",  7) == "hard work in env");
        CATCH_REQUIRE(opt.get_string("filenames",  8) == "extra");
        CATCH_REQUIRE(opt.get_string("filenames",  9) == "-file");
        CATCH_REQUIRE(opt.get_string("filenames", 10) == "names");
        CATCH_REQUIRE(opt.get_string("filenames", 11) == "-");
        CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
        CATCH_REQUIRE(opt.size("filenames") == 12);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
    }

    // check that multiple flags can be used one after another
    advgetopt::option const valid_short_options_list[] =
    {
        //{
        //    '\0',
        //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
        //    nullptr,
        //    nullptr,
        //    "Usage: test valid short options",
        //    advgetopt::argument_mode_t::help_argument
        //},
        {
            'a',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
            "a-not-in-v2",
            nullptr,
            "letter option",
            nullptr
        },
        {
            'c',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "c-not-in-v2",
            nullptr,
            "letter option",
            nullptr
        },
        {
            'd',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "d-not-in-v2",
            nullptr,
            "letter option",
            nullptr
        },
        {
            'f',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
            "f-not-in-v2",
            nullptr,
            "another letter",
            nullptr
        },
        {
            'r',
            advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_REQUIRED,
            "r-not-in-v2",
            nullptr,
            "another letter",
            nullptr
        },
        {
            '\0',
            advgetopt::GETOPT_FLAG_END,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        }
    };

    advgetopt::options_environment valid_short_options;
    valid_short_options.f_project_name = "unittest";
    valid_short_options.f_options = valid_short_options_list;
    valid_short_options.f_help_header = "Usage: test valid short options";
    valid_short_options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

    // test that we can use -cafard as expected
    {
        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
            "-cafard",
            "alpha",
            "-",
            "recurse",
            nullptr
        };
        int const sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        advgetopt::getopt opt(valid_short_options, sub_argc, sub_argv);

        // check that the result is valid

        // an invalid parameter, MUST NOT EXIST
        CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

        // 2x 'a' in cafard, but we only keep the last entry
        CATCH_REQUIRE(opt.is_defined("a"));
        CATCH_REQUIRE(opt.get_string("a") == "-");
        CATCH_REQUIRE(opt.get_string("a", 0) == "-");
        CATCH_REQUIRE(opt.get_default("a").empty());
        CATCH_REQUIRE(opt.size("a") == 1);

        // c
        CATCH_REQUIRE(opt.is_defined("c"));
        CATCH_REQUIRE(opt.get_string("c") == "");
        CATCH_REQUIRE(opt.get_default("c").empty());
        CATCH_REQUIRE(opt.size("c") == 1);

        // d
        CATCH_REQUIRE(opt.is_defined("d"));
        CATCH_REQUIRE(opt.get_string("d") == "");
        CATCH_REQUIRE(opt.get_default("d").empty());
        CATCH_REQUIRE(opt.size("d") == 1);

        // f
        CATCH_REQUIRE(opt.is_defined("f"));
        CATCH_REQUIRE(opt.get_string("f") == "");
        CATCH_REQUIRE(opt.get_default("f").empty());
        CATCH_REQUIRE(opt.size("f") == 1);

        // r
        CATCH_REQUIRE(opt.is_defined("r"));
        CATCH_REQUIRE(opt.get_string("r") == "recurse");
        CATCH_REQUIRE(opt.get_string("r", 0) == "recurse");
        CATCH_REQUIRE(opt.get_default("r").empty());
        CATCH_REQUIRE(opt.size("r") == 1);

        // other parameters
        CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
        CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
    }

    // check that an optional option gets its default value if no arguments
    // were specified on the command line
    {
        // we need options with a --filenames that is optional
        const advgetopt::option valid_options_with_optional_filenames_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: test valid options with optional filenames",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE,
                "valid-parameter",
                nullptr,
                "a valid option",
                nullptr
            },
            {
                'v',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_FLAG,
                "verbose",
                nullptr,
                "a verbose like option, select it or not",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_CONFIGURATION_FILE | advgetopt::GETOPT_FLAG_ENVIRONMENT_VARIABLE | advgetopt::GETOPT_FLAG_MULTIPLE | advgetopt::GETOPT_FLAG_DEFAULT_OPTION,
                "filenames",
                "a.out",
                "expect multiple strings",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment valid_options_with_optional_filenames;
        valid_options_with_optional_filenames.f_project_name = "unittest";
        valid_options_with_optional_filenames.f_options = valid_options_with_optional_filenames_list;
        valid_options_with_optional_filenames.f_help_header = "Usage: test valid short options";
        valid_options_with_optional_filenames.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            // first try with that option by itself
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
                "--valid-parameter",
                "optional argument",
                "--filenames",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            advgetopt::getopt opt(valid_options_with_optional_filenames, sub_argc, sub_argv);

            // check that the result is valid

            // an invalid parameter, MUST NOT EXIST
            CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

            // valid parameter
            CATCH_REQUIRE(opt.is_defined("valid-parameter"));
            CATCH_REQUIRE(opt.get_string("valid-parameter") == "optional argument"); // same as index = 0
            CATCH_REQUIRE(opt.get_string("valid-parameter", 0) == "optional argument");
            CATCH_REQUIRE(opt.get_default("valid-parameter").empty());
            CATCH_REQUIRE(opt.size("valid-parameter") == 1);

            // filenames
            CATCH_REQUIRE(opt.is_defined("filenames"));
            CATCH_REQUIRE(opt.get_string("filenames") == "a.out"); // same as index = 0
            CATCH_REQUIRE(opt.get_string("filenames", 0) == "a.out");
            CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
            CATCH_REQUIRE(opt.size("filenames") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
            CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
        }
        {
            // try again with a -v after the --filenames without filenames
            const char *sub_cargv[] =
            {
                "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra",
                "--filenames",
                "-v",
                nullptr
            };
            const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
            char **sub_argv = const_cast<char **>(sub_cargv);

            advgetopt::getopt opt(valid_options_with_optional_filenames, sub_argc, sub_argv);

            // check that the result is valid

            // an invalid parameter, MUST NOT EXIST
            CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

            // filenames
            CATCH_REQUIRE(opt.is_defined("filenames"));
            CATCH_REQUIRE(opt.get_string("filenames") == "a.out"); // same as index = 0
            CATCH_REQUIRE(opt.get_string("filenames", 0) == "a.out");
            CATCH_REQUIRE(opt.get_default("filenames") == "a.out");
            CATCH_REQUIRE(opt.size("filenames") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "AdvGetOptUnitTests::valid_config_files_extra");
            CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/AdvGetOptUnitTests::valid_config_files_extra");
        }
    }

    // strange entry without a name
    {
        const advgetopt::option options_list[] =
        {
            //{
            //    '\0',
            //    advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR,
            //    nullptr,
            //    nullptr,
            //    "Usage: try this one and we get a throw (strange entry without a name)",
            //    advgetopt::argument_mode_t::help_argument
            //},
            {
                '\0',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_SHOW_USAGE_ON_ERROR | advgetopt::GETOPT_FLAG_MULTIPLE,
                nullptr, // no name
                "README",
                nullptr, // no help
                nullptr
            },
            {
                'v',
                advgetopt::GETOPT_FLAG_COMMAND_LINE | advgetopt::GETOPT_FLAG_FLAG,
                "verbose",
                nullptr,
                "show more stuff when found on the command line.",
                nullptr
            },
            {
                '\0',
                advgetopt::GETOPT_FLAG_END,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            }
        };

        advgetopt::options_environment options;
        options.f_project_name = "unittest";
        options.f_options = options_list;
        options.f_help_header = "Usage: try this one and we get a throw (strange entry without a name)";
        options.f_environment_variable_name = "ADVGETOPT_TEST_OPTIONS";

        {
            const char *cargv2[] =
            {
                "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/no-name-arg-defaults-to-dash-dash",
                "-v",
                "wpkg.cpp",
                nullptr
            };
            const int argc2 = sizeof(cargv2) / sizeof(cargv2[0]) - 1;
            char **argv2 = const_cast<char **>(cargv2);

            // this initialization works as expected
            {
                CATCH_REQUIRE_THROWS_AS( { advgetopt::getopt opt(options, argc2, argv2); }, advgetopt::getopt_exception_logic);
            }

#if 0
// it throws, so no results...
            // check that the result is valid

            // an invalid parameter, MUST NOT EXIST
            CATCH_REQUIRE(!opt.is_defined("invalid-parameter"));

            // verbose
            CATCH_REQUIRE(opt.is_defined("verbose"));
            CATCH_REQUIRE(opt.get_string("verbose") == ""); // same as index = 0
            CATCH_REQUIRE(opt.get_string("verbose", 0) == "");
            CATCH_REQUIRE(opt.get_default("verbose").empty());
            CATCH_REQUIRE(opt.size("verbose") == 1);

            // the no name parameter!?
            CATCH_REQUIRE(opt.is_defined("--"));
            CATCH_REQUIRE(opt.get_string("--") == "wpkg.cpp"); // same as index = 0
            CATCH_REQUIRE(opt.get_string("--", 0) == "wpkg.cpp");
            CATCH_REQUIRE(opt.get_default("--") == "README");
            CATCH_REQUIRE(opt.size("--") == 1);

            // other parameters
            CATCH_REQUIRE(opt.get_program_name() == "no-name-arg-defaults-to-dash-dash");
            CATCH_REQUIRE(opt.get_program_fullname() == "tests/unittests/unittest_advgetopt/AdvGetOptUnitTests::invalid_parameters/no-name-arg-defaults-to-dash-dash");
#endif
        }
    }
}


CATCH_TEST_CASE( "AdvGetOptUnitTests::invalid_parameters", "AdvGetOptUnitTests" )
{
    AdvGetOptUnitTests advgetopt;
    advgetopt.invalid_parameters();
}


CATCH_TEST_CASE( "AdvGetOptUnitTests::valid_config_files", "AdvGetOptUnitTests" )
{
    AdvGetOptUnitTests advgetopt;
    advgetopt.valid_config_files();
}


CATCH_TEST_CASE( "AdvGetOptUnitTests::valid_config_files_extra", "AdvGetOptUnitTests" )
{
    AdvGetOptUnitTests advgetopt;
    advgetopt.valid_config_files_extra();
}


// vim: ts=4 sw=4 et
