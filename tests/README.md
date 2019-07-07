
# Running Coverage

In order to verify coverage properly you need to run a small set of tests
for each section of the library. One of in the library represents one
section.

Running all the tests will include a large amount of overlap, but for
many functions that means a possible lack of completeness in terms of
testing. Hence the sectioning.

The following describes each part and the list of test to run to verify
that part.

## Options Parsing (`advgetopt_options.cpp`)

We have three _parsers_:

* Options
* Arguments
* Configurations

Here we test the Options. This is the part of the library which converts
option tables in a map of objects used to parse your arguments and
configuration files.

One part of that parser is the ability to load options from a .ini file.
This makes use of the code in link with the configuration file, so it
covers some of that code, but only to the extend necessary for support
with the options.

    dev/coverage options_parser,invalid_options_parser,valid_options_files,invalid_options_files

## Arguments Parsing (`advgetopt.cpp`)

We have three _parsers_:

* Options
* Arguments
* Configurations

Here we test the Arguments. This is the part of the library which converts
command line, variable, and configuration file data to a map of named values.

Note that the Arguments does not verify the configuration file support. This
is done separately because it is really large on its own. It does test the
command line and variable, though. The variable includes parsing a string
into arguments before parsing the arguments in our map of named values.

    dev/coverage flag_argument,require_argument,require_arguments,optional_arguments,default_argument,default_arguments,manual_arguments,invalid_getopt_pointers,invalid_getopt_missing_options,invalid_getopt_missing_alias,invalid_getopt_missing_required_option,invalid_default_options,invalid_options

## Configuration Parsing (`advgetopt_config.cpp`)

We have three _parsers_:

* Options
* Arguments
* Configurations

Here we test the code handle the loading and saving from configuration files.
This is the part of the library which converts configuration file data to
a map of named values. The configuration code has the ability to accept any
argument without having a definition to match the argument. The idea being
that once in a while we need to load a configuration file without having
access to the argument definitions. This happens in many places in Snap!
However, we now have the ability to define an external definition for each
file so it is possible to verify configuration files properly from any tool.

    dev/coverage configuration_filenames,load_configuration_file,load_multiple_configurations,load_invalid_configuration_file

**Note:** Since the parsing first requires the loading of the configuration
file, these tests partially exercise the configuration loader implementation.

## Configuration Loading (`config_file.cpp`) NOT COMPLETE

This section checks the loading (and saving) of data in a configuration
file. It also exercises all the supported configuration formats.

    dev/coverage configuration_spaces,configuration_setup,config_reload_tests,config_line_continuation_tests,config_assignment_operator_tests,config_comment_tests,config_section_tests,invalid_configuration_setup,config_reload_invalid_setup,missing_configuration_file,invalid_sections,invalid_variable_name

## Program & Project Names (`advgetopt_access.cpp`)

The program name is taken from the command line first string (`argv[0]`)
which is expected to be the path to the program being run.

The project name is _hard coded_ in the option environment structure passed
to the getopt constructor. This one may be set to `nullptr` or an empty
string.

These tests make sure that all cases are checked, including a program name
with backslashes as directory separators (if no slashes are found first.)
This is a small remain from the days when advgetopt worked on MS-Windows.

    dev/coverage program_name,project_name,invalid_program_name

## Data Retieval (`advgetopt_data.cpp`)

Once the command line arguments, environment variable, and configuration
files were parsed, you want to retrieve the data. These test verify that
the data is returned to you as expected.

    dev/coverage string_access,long_access,invalid_option_name,missing_default_value,incompatible_default_value,out_of_range_value

## Usage (`advgetopt_usage.cpp`)

Whenever an error occurs or when a command line option such as `--help`
is used, the usage screens get printed. This tests verify that the usage
output works as expected.

    dev/coverage usage_function,help_string_percent,help_string_project_name,help_string_build_date,help_string_copyright,help_string_directories,help_string_environment_variable,help_string_configuration_files,help_string_license,help_string_program_name,help_string_build_time,help_string_version

## Logger (`log.cpp`)

The advgetopt uses a logger class so it can generate messages of various
levels and send them to the console or your callback. If you have a way
to send the log messages to a log file, for example, setup a callback
and redirect those messages to your log file instead.

    dev/coverage logger,logger_without_callback,invalid_logger

## Option Info (`option_info.cpp`)

The `option_info` class is used to register options taken from statically
compiled option tables. The advantage of having an object is to allow for
dynamic options to be added at run-time, which mainly happens when loading
configuration files but can also be allowed on the command line and
the environment variable.

    dev/coverage to_from_short_name,option_info_basics,option_info_flags,option_info_default,option_info_help,option_info_validator,option_info_alias,option_info_multiple_separators,option_info_add_value,option_info_set_value,option_info_section_functions,invalid_option_info

This test also verifies that the arguments can be defined in a configuration
file.

**Note:** This is strongly linked to the Options Parser above. The fact is
that many cases are handled within the option parser and as a result
the `option_info` class is not fully checked through the parser.

## Option Info References (`option_info_ref.cpp`)

The `getopt` objects can be used with the `[]` operator. When the input `this`
is not constant, the operator returns an `option_info_ref` class which allows
us to access the first value in read and write modes.

    dev/coverage option_info_ref

This verifies that the reference object is fully covered.

## Utilities (`utils.cpp`)

We have a few functions used in the library that do not really fit anywhere
specifically which we put in the utils.cpp file. These tests specifically
verify those functions.

    dev/coverage utils_unquote,utils_split,utils_insert_project_name,utils_handle_user_directory


## Validator (`validator.cpp`)

The validator classes are used to make sure that the data supplied by the
user is considered valid. For example, it may have to be an integer or
a an email address.

The following checks all the advgetopt supplied validators. You can create
some of your own, which, of course, cannot be validated here.

    dev/coverage unknown_validator,integer_validator,regex_validator,invalid_validator

## Version (`version.cpp`)

Check that the version functions work as expected. This also verifies that
you are running the test that corresponds to your version of the library.

    dev/coverage version


## Library versus Tests

    +------------------------+---------------------+--------+
    | Library Filename       | Test Filename       | Status |
    +------------------------+---------------------+--------+
    | advgetopt_access.cpp   | access.cpp          |  DONE  |
    | advgetopt_config.cpp   | config.cpp          |  DONE  |
    | advgetopt.cpp          | arguments.cpp       |  DONE  |
    | advgetopt_data.cpp     | data.cpp            |  DONE  |
    | advgetopt_options.cpp  | options_parser.cpp  |  DONE  |
    |                        | & options_files.cpp |        |
    | advgetopt_usage.cpp    | usage.cpp           |  DONE  |
    | conf_file.cpp          | config_File.cpp     |  DONE  |
    | log.cpp                | logger.cpp          |  DONE  |
    | option_info.cpp        | option_info.cpp     |  DONE  |
    | option_info_ref.cpp    | option_info_ref.cpp |  DONE  |
    | utils.cpp              | utils.cpp           |  DONE  |
    | validator.cpp          | validator.cpp       |  DONE  |
    | version.cpp            | version.cpp         |  DONE  |
    +------------------------+---------------------+--------+
    | advgetopt.h            |                     |  DONE  |
    | conf_file.h            |                     |  DONE  |
    | exception.h            |                     |  DONE  |
    | flags.h                |                     |  DONE  |
    | log.h                  |                     |  DONE  |
    | option_info.h          |                     |  DONE  |
    | options.h              |                     |  DONE  |
    | utils.h                |                     |  DONE  |
    | validator.h            |                     |  DONE  |
    | version.h.in           |                     |  DONE  |
    +------------------------+---------------------+--------+
    |                        | log_for_test.cpp    |        |
    |                        | main.cpp            |        |
    |                        | main.h              |        |
    +------------------------+---------------------+--------+


vim: ts=4 sw=4 et
