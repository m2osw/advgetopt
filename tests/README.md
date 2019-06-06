
# Running Coverage

In order to verify coverage properly you need to run a small set of tests
for each section of the library. One of in the library represents one
section.

Running all the tests will include a large amount of overlap, but for
many functions that means a possible lack of completeness in terms of
testing. Hence the sectioning.

The following describes each part and the list of test to run to verify
that part.

## Options Parsing

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

## Arguments Parsing

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

## Configuration Parsing

We have three _parsers_:

* Options
* Arguments
* Configurations

Here we test the Configurations. This is the part of the library which converts
configuration file data to a map of named values. The configuration code has
the ability to accept any argument without having a definition to match the
argument. The idea being that once in a while we need to load a configuration
file without having access to the argument definitions. This happens in many
places in Snap! However, we now have the ability to define an external
definition for each file so it will be possible to verify configuration files
properly from any tool.

## Program & Project Names

The program name is taken from the command line first string (`argv[0]`)
which is expected to be the path to the program being run.

The project name is _hard coded_ in the option environment structure passed
to the getopt constructor. This one may be set to `nullptr` or an empty
string.

These tests make sure that all cases are checked, including a program name
with backslashes as directory separators (if no slashes are found first.)
This is a small remain from the days when advgetopt worked on MS-Windows.

    dev/coverage program_name,project_name

## Data Retieval

Once the command line arguments, environment variable, and configuration
files were parsed, you want to retrieve the data. These test verify that
the data is returned to you as expected.

    dev/coverage string_access,long_access,invalid_option_name,missing_default_value,incompatible_default_value,out_of_range_value

## Usage

Whenever an error occurs or when a command line option such as `--help`
is used, the usage screens get printed. This tests verify that the usage
output works as expected.

    dev/coverage usage_function,help_string_percent,help_string_project_name,help_string_build_date,help_string_copyright,help_string_directories,help_string_environment_variable,help_string_configuration_files,help_string_license,help_string_program_name,help_string_build_time,help_string_version



vim: ts=4 sw=4 et
