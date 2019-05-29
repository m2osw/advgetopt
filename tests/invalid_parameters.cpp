/*
 * Files:
 *    tests/invalid_parameters.cpp
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
#include "main.h"

// advgetopt lib
//
#include "advgetopt/exception.h"

// C++ lib
//
#include <fstream>





CATCH_TEST_CASE("invalid_parameters", "inalid,getopt")
{
    std::cout << std::endl << "Advanced GetOpt Output (expected until the test fails):" << std::endl;

    // default arguments
    //
    char const * cargv[] =
    {
        "tests/unittests/invalid_parameters",
        "--ignore-parameters",
        nullptr
    };
    const int argc = sizeof(cargv) / sizeof(cargv[0]) - 1;
    char **argv = const_cast<char **>(cargv);

    // no options available
    //
    const advgetopt::option options_empty_list[] =
    {
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
    //
    const advgetopt::option options_no_name_list[] =
    {
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
    //
    const advgetopt::option options_2chars_minimum_list[] =
    {
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
    //
    const advgetopt::option options_2chars_minimum2_list[] =
    {
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
    //
    const advgetopt::option options_defined_twice_list[] =
    {
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
    //
    const advgetopt::option options_short_defined_twice_list[] =
    {
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
    //
    const advgetopt::option options_two_default_multiple_arguments_list[] =
    {
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
    //
    const advgetopt::option options_two_default_arguments_list[] =
    {
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
    //
    const advgetopt::option options_mix_of_default_list[] =
    {
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
    //
    const advgetopt::option options_no_defaults_list[] =
    {
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
        //
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

        unittest::push_expected_log("error: no default options defined; we do not know what to do of \"this\"; standalone parameters are not accepted by this program.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }
    {
        // a '-' by itself is a problem when there is no default because it
        // is expected to represent a filename (stdin)
        //
        char const * sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "-",
            nullptr
        };
        const int sub_argc(sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1);
        char **sub_argv(const_cast<char **>(sub_cargv));

        unittest::push_expected_log("error: no default options defined; thus - is not accepted by this program.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }
    {
        // the -- by itself would be fine, but since it represents a
        // transition from arguments to only filenames (or whatever the
        // program expects as default options) it generates an error if
        // no default options are accepted
        //
        const char *sub_cargv[] =
        {
            "tests/unittests/AdvGetOptUnitTests::invalid_parameters",
            "--verbose",
            "--", // already just by itself it causes problems
            nullptr
        };
        const int sub_argc = sizeof(sub_cargv) / sizeof(sub_cargv[0]) - 1;
        char **sub_argv = const_cast<char **>(sub_cargv);

        unittest::push_expected_log("error: no default options defined; thus -- is not accepted by this program.");
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

        unittest::push_expected_log("error: no default options defined; thus -- is not accepted by this program.");
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

        unittest::push_expected_log("error: option -v is not supported.");
        advgetopt::getopt opt(options_no_defaults, sub_argc, sub_argv);
    }

    // check -- when default does not allowed environment variables
    //
    const advgetopt::option options_no_defaults_in_envvar_list[] =
    {
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

        unittest::push_expected_log("error: option - is not supported in the environment variable.");
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

        unittest::push_expected_log("error: default options are not supported in the environment variable.");
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

        unittest::push_expected_log("error: option -- is not supported in the environment variable.");
        advgetopt::getopt opt(options_no_defaults_in_envvar, sub_argc, sub_argv);
    }

    // unnknown long options
    //
    const advgetopt::option valid_options_unknown_command_line_option_list[] =
    {
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

        unittest::push_expected_log("error: option --verbose is not supported.");
        advgetopt::getopt opt(valid_options_unknown_command_line_option, sub_argc, sub_argv);
    }

    // illegal short or long option in variable
    //
    const advgetopt::option options_illegal_in_variable_list[] =
    {
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

        unittest::push_expected_log("error: option --verbose is not supported in the environment variable.");
        unittest::push_expected_log("error: option --ignore-parameters is not supported.");
        advgetopt::getopt opt(options_illegal_in_variable, argc, argv);
    }
    {
        // short
        unittest::obj_setenv env("ADVGETOPT_TEST_OPTIONS=-v");

        unittest::push_expected_log("error: option -v is not supported in the environment variable.");
        unittest::push_expected_log("error: option --ignore-parameters is not supported.");
        advgetopt::getopt opt(options_illegal_in_variable, argc, argv);
    }

    // configuration file options must have a long name
    //
    const advgetopt::option configuration_long_name_missing_list[] =
    {
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
    //
    const advgetopt::option valid_options_list[] =
    {
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
            unittest::push_expected_log("error: option name from \"valid param\" on line 2 in configuration file \""
                                    + config_filename
                                    + "\" cannot include a space, missing assignment operator?");
            unittest::push_expected_log("error: option --ignore-parameters is not supported.");
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
            unittest::push_expected_log("error: option name from \" valid param \" on line 2 in configuration file \""
                                    + config_filename
                                    + "\" cannot include a space, missing assignment operator?");
            unittest::push_expected_log("error: option --ignore-parameters is not supported.");
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
            unittest::push_expected_log("error: no option name in \" = valid param\" on line 2 from configuration file \""
                                    + config_filename
                                    + "\", missing name before = sign?");
            unittest::push_expected_log("error: option --ignore-parameters is not supported.");
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
            unittest::push_expected_log("error: option names in configuration files cannot start with a dash or an underscore in \"--valid=param\" on line 2 from configuration file \""
                                    + config_filename
                                    + "\".");
            unittest::push_expected_log("error: option --ignore-parameters is not supported.");
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
            unittest::push_expected_log("error: unknown option \"invalid\" found in configuration file \""
                                    + config_filename
                                    + "\".");
            unittest::push_expected_log("error: option --ignore-parameters is not supported.");
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
            unittest::push_expected_log("error: option \"command\" is not supported in configuration files (found in \""
                                    + config_filename
                                    + "\").");
            unittest::push_expected_log("error: option --ignore-parameters is not supported.");
            advgetopt::getopt opt(valid_options, argc, argv);
        }
    }

#if 0
// in version 2.x we do not have this case anymore

    // one of the options has an invalid mode; explicit option
    {
        const advgetopt::option options_list[] =
        {
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
        unittest::push_expected_log("error: invalid number (123abc) in parameter --long.");
        l = opt.get_long("long");
        CATCH_REQUIRE(l == -1);
        unittest::push_expected_log("error: 123 is out of bounds (1..9 inclusive) in parameter --out-of-bounds.");
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
            //
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

            unittest::push_expected_log("error: option --long expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
        {
            // again with the lone -l (no long name)
            //
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

            unittest::push_expected_log("error: option --not-in-v2-though expects an argument.");
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
            //
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

            unittest::push_expected_log("error: option --filenames expects an argument.");
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

            unittest::push_expected_log("error: option --filenames expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
    }

    // required multiple without arguments, short name only
    {
        advgetopt::option const options_list[] =
        {
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

            unittest::push_expected_log("error: option --filenames expects an argument.");
            advgetopt::getopt opt(options, argc2, argv2);
        }
    }
}


// vim: ts=4 sw=4 et
