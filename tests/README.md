
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

    dev/coverage flag_argument,require_argument,require_arguments,optional_arguments,default_argument,default_arguments,manual_arguments,invalid_getopt_pointers,invalid_getopt_missing_options,invalid_getopt_missing_alias,invalid_default_options,invalid_options

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







vim: ts=4 sw=4 et
