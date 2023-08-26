
<p align="center">
<img alt="advgetopt" title="Advance getopt, a C++ library to handle your command line options and configuration files seamlessly."
src="https://snapwebsites.org/sites/snapwebsites.org/files/images/advgetopt-277x277.png" width="277" height="277"/>
</p>

# Introduction

The `advgetopt` library provides the `advgetopt::getopt` class, which
parses command line options (from `argc`/`argv`). It is very advanced and
is capable of reading many different types of options with a letter
(`-h`) and a word (`--verbose`) with no parameters, one parameter, any
number of parameters, and a set of "filenames," parameters without
a flag first. The values can be checked as integers, sizes (1Mb), durations
(3h15min), floating points, and against your own regex.

The class automatically accepts input from the command line, environment
variables, and configuration files.


# Understood Options

By default, the command line options understood by your application are
found in a static array. That array is verified and compiled by the C++
compiler. It then gets transferred in a vector at runtime. That vector
can be completed by command line options found in configuration files.
You can also dynamically add command line options, although there is
rarely a need for such (i.e. the `--help` option is one exception handled
by the library where addition help options are added for each group you
define).

The array is built with one entry per option. If you have options that
make use of different names but have exactly the same result, then you
create multiple entries in your array and mark the additional entries
as `GETOPT_FLAG_ALIAS` and reference the first entry.

Options can be used on the command line, in environment variables,
or in configuration files. You can select exactly which option is going
to apply to what is found on the command line, in the configuration file,
or the environment variables using the following flags:

    GETOPT_FLAG_COMMAND_LINE
    GETOPT_FLAG_ENVIRONMENT_VARIABLE
    GETOPT_FLAG_CONFIGURATION_FILE

Further, you probably do not want all the options to be shown whenever
an error is detected. For this reason there is a flag to tell the library
which options has to be shown on an error:

    GETOPT_FLAG_SHOW_USAGE_ON_ERROR


# Command Line

The `getopt` class accepts the `argc`/`argv` parameters directly from your
`main()` function. Of course, you can also build another array and use the
`getopt` class at any time.

Here is an example from iplock. This is how the ipload tool gets initialized.
As we can see, we pass an array of statically defined options. Then we
dynamically add options from the snaplogger. At that point the library is
ready to parse the options so we call the `finish_parsing()` function.
Since we do not know what the logger added, we give it a chance to process
its command line options (i.e. change the output file, severity level, etc.)
Finally we can test our own options such as the `"verbose"` and `"quiet"`
options as shown at the end of this example.

    ipload::ipload(int argc, char * argv[])
        : f_opts(g_options_environment)
    {
        snaplogger::add_logger_options(f_opts);
        f_opts.finish_parsing(argc, argv);
        if(!snaplogger::process_logger_options(
                      f_opts
                    , "/etc/iplock/logger"
                    , std::cout
                    , !isatty(fileno(stdin))))
        {
            // exit on any error
            //
            throw advgetopt::getopt_exit("logger options generated an error.", 1);
        }

        f_verbose = f_opts.is_defined("verbose");
        f_quiet = f_opts.is_defined("quiet");
	// ...snip...
    }



# Configuration Files

You can define paths and filenames for the library to search for
configuration files. If no paths or filenames are defined then no
configuration files are loaded.

The search is complicated. To simplify the administrator's work, we
offer the special option:

    my-tool --configuration-filenames

which will list the name of each configuration file that gets loaded.
Note that includes files found in sub-directories that have a name such
as:

    /etc/project/project.d/XX-project.conf

The `XX` is a two digit number from 00 to 99. This allows all kinds of
overrides by any of the packages installed on your system. The special
number 50 is reserved for administrators.

By default, configuration files support empty lines and lines that start with
a hash that are viewed as comments. Spaces can appear before the hash.

The other entries have to be of the form:

    option_name=value

Note that in the configuration file, no `-` or `--` can appear in
front of the option name. Also the short name of an option is not allowed.

Note that options in your array of options are expected to use `"-"`
when you want to separate multiple words like in "long-option".
However, in the configuration file, you are expected to use `"_"`.
So in the configuration file it would look like this:

    # Command line:
    program --long-option 123

    # Configuration file:
    long_option=123

The system is capable of accept other operators and supports variables
within a configuration file. These features are generally not made
available at the command line level. These extra features are therefore
available only in a few tools and services. These tools and services
describe the other features they support.

The possible features are:

* Accept a space as the separator between the name of a parameter and its
  value (`long_option 123`).
* Accept a colon (`:`) as the assignment operator, instead of the equal
  sign (`long_option:123`).
* Accept variable references in the values, written as `${<name>}`.
* Accept INI like section declarations `[<name>]`.
* Accept C-like blocks as section declarations `<name> { ... }`.
* Accept C++-like scoping `<scope>::<name>=<value>`.
* Multi-line parameters with backslash, & at the end or the start of the line.
* Load all files found in a sub-directory (usually named `<project>.d`)
  sorting files by the two digit number at the start of the filename
  (`XX-<name>.conf`).


## `edit-config`

This tool can be used to edit simple Unix-like configuration files. You can
either retrieve the value of a field or set the value of a field.

    edit-config <config>.conf <field-name> [<new-value>]

Say you have a file named "/etc/log/settings.conf" with the following
parameters:

    severity=DEBUG
    path=/var/log/advgetopt/output.log
    auto_reload=false

Then runinng command:

    edit-config /etc/log/settings.conf path

prints out:

    /var/log/advgetopt/output.log

And command:

    edit-config /etc/log/settings.conf auto_reload true

replaces the value of `auto_reload` with `true`.

**Note:** make sure to quote the value if it includes special characters.

This tool is already capable of editing many different type of configuration
files, but watch out, there are still many bugs. Please test your usage
thouroughly before making it available in production.


# Numbers

The value for an option can be marked as optional or required.

There is also a way to require the option to be an integer (long). If
the option can only be a number from 0 to 2^64-1 then this is a good
idea. Note that if your option also accepts a special value such as
`none`, then you can't use this option, however, you can still attempt
to retrieve the option with the `get_long()` function.

    if(a.get_string("my-option") == "none")
    {
    	// handle "none" case
	//
	...
    }
    else
    {
    	long v(a.get_long("my-option"));
	// handle 'v' as required
    	...
    }

If the the option is not a valid integer then an exception is raised.

We also offer a `get_double()` and validators. Validators allow for
other types to be checked:

* Integer
* Double
* Duration (such as `3h 15m 45s`)
* Email
* Keyword
* Length (a set of allowed integer ranges: `3 ... 17, 55 ... 81, 100`)
* List (a set of validators)
* Regular Expression
* Size (such as `37 Kb`)

The List validator is peculiar in that it automatically gets used when
you define multiple validators in your list of validators. This is
most used with the Keywords and another validator such as the Integers
validator.

# Environment Variables

## The Global Environment Variable

The variables from the environment can be checked for parameters.

The contents of the global environment variable are viewed just as a
command line with options. There are no limits except that only options that
were authorized to appear in the variable can be used in that variable.

To authorize an option variable, set the following flag on the option:

    GETOPT_FLAG_ENVIRONMENT_VARIABLE

The reason for not letting any parameter to work in the global environment
variable (or a configuration file, if that matter) is because certain
options do not make sense in variables.

For example, if you support a `--help` option, it most certainly
would be totally useless in the environment variable.

Using an option that's not acceptable in an environment variable
breaks the command line parsing by generating an error.

One reason to support environment variables is to define things
like a path which has to be specific to each user of your tool:

    MY_PROJECT_CONF="--path /home/alexis/projects"

Note that the variable options are required to use the `-` or `--`
as expected on the command line.

## Per Option Environment Variables

If you want to use an environment variable to just hold the value of
an option, then use the environment variable name in the option.

The value is checked just like a normal option.

This features allows your services to be compatible with systems such
as Kubernetes, which make use of environment variables instead of
command line arguments. With such, you can use the configuration
file for options that do not need to be dynamically changed or are
specific to a system that runs that service (i.e. such as the local IP).


# Project Name

The `options_environment` structure accepts a project name and a group name.

By default, the project name (`f_project_name`) is used as the configuration
filename. You can redefine the filename using the `f_configuration_filename`
field. Or multiple filenames with full paths (or `~/...`) using the
`f_configuration_files`.

Finally, you can use the `f_configuration_directories` to change the set
of directories you want to search for configuration files. In most cases,
this one uses `/etc/<project-name>`. Some tools allow files to be defined
under `/usr/share`. These files are not to be edited by administrators.

The name of the group (`f_group_name`) is used to load modified configuration
files. If the `f_group_name` is not defined, then the `f_project_name` is
used instead. `.d` is appeneded to the name to generate a sub-path where
the project searches for additional configuration files.

Say your project name is "project" and your group name is "group", the
advgetopt library searches files that match the name:

    /etc/project/group.d/XX-project.conf

where XX is a number from 00 to 99. The number is important since it allows
us to sort the files and read them in order. This is useful because the last
time a parameter is set is the value the program receives when it queries
the library. This means the administrator can override values found in:

    /etc/project/project.conf

by defining a file such as:

    /etc/project/group.d/50-project.conf

Similarly, other projects can install files in the `group.d` directory with
lower or higher numbers to override the defaults (XX < 50) and possibly make
sure the administrator does not change a value (XX > 50).

_Note that although it is not currently checked, you probably should
not include slashes in your project or group name._


# Updating Configuration Files (`edit-config`)

The `edit-config` tool can be used to edit configuration files from the
command line. This works well with standard Unix files (`name=value`).
It may not do exactly what you expect for other types of files.

Note that all that is possible to do with this tool can be done with the
`libadvgetopt` library. The `conf_files.h` has the classes necessary to
do these.

The `--space` can be used to use a space instead of an equal sign between
the name nad value (`name value`). Similarly, use the `--colon` option to
support a colon instead of the equal or space (`name: value`). The `--equal`
restores the default.

Another issue is that some configuration files use dashes in their variable
names instead of the default underscore that most system use. The `--dashes`
option switches to dashes (`-`). The `--underscores` restores the default
(`_`).

The `--sub-directory` option let you handle a file from within a sub-directory
instead of the directory defined in the path to the configuration file.
With advgetopt, you can create a sub-directory just so you can edit the file
in that sub-directory instead of the original so that way we can avoid missing
developer changes at a later time. For example, a snapwebsite configuration
file could be:

    /etc/snapwebsites/server.conf     # file coming from the server package
    /etc/snapwebsites/snapwebsites.d/50-server.conf   # admin managed

In this example, the `/etc/snapwebsites/server.conf` is installed by a
package in snapwebsites. You should never edit that file. The tools loading
that file will also look in `/etc/snapwebsites/snapwebsites.d/??-server.conf`.
The `--sub-directory` option allows you to load the
`/etc/snapwebsites/server.conf` file if the
`/etc/snapwebsites/snapwebsites.d/50-server.conf` does not exist.
In most cases, though, you probably want to just update the files in
the sub-directory, so have a command line such as:

    edit-config /etc/snapwebsites/snapwebsites.d/30-server.conf name value

When the `edit-config` is not enough, you may be interested in using the
`sed` command. Here is an example that makes a backup of the configuration
file if it gets updated, it otherwise searches for an option and if the
option is already present, do nothing (`q`), otherwise appened (`$a`) a line
to the file:

    sed \
        -i.bak \
        -e '/^\(include "\/etc\/bind\/ipmgr-options.conf";\)/ {s//\1/;:a;n;ba;q}' \
        -e '$ainclude "/etc/bind/ipmgr-options.conf";' \
            /etc/bind/named.conf.local

_Note: the [`ipmgr` project](https://github.com/m2osw/ipmgr) also comes with
a tool named `dns-options` allowing for the editing of DNS configuration
files. It is still in development as it doesn't yet work properly in all
situations, but works for what we currently need it for. It may also be
useful to you._


# Type of Options Supported

As we've seen, the options are supported from different sources. There are
also several types of options.

## Dash versus Underscore

It is customary to accept multi-word options with dashes when written on
the command line: `--list-all`.

However, inside a configuration file, it is much more customary to make use
of underscores: `list_all=true`.

The advgetopt library converts all the underscores found on the option names
in dashes. Internally, the names are therefore stored as `list-all` but it
properly matches the `list_all` found in configuration files.

## Long Options

The default is to offer a long option, for example `--verbose`.

Long options are always introduced by two dashes. The name of a long option
must be at least two characters.

The name of long options can be used in configuration files.

## One Letter

It is possible to use command line options with a single letter. For example,
the `-v` option is often used to be `--verbose`.

Multiple letters can be used together: `-vije`. This is equivalent to
specifying each letter on its own: `-v -i -j -e`. Single letters can be
followed by one or more parameters like long options: `-f filename`.

## Option Keys

Option can be used with a key, meaning that the same option can receive
_any_ number of values distinguished by a key.

The default syntax is like `--name:key` where `name` is the name of the
option and `key` is a string representing a key.

This feature is not optimized and it should rarely be useful. It can still
be great as it allows you to dynamically add any number of values to an
option without having to define separate option each time. Just make sure
to keep it sensible (i.e. a `--memory` option could receive keys such as
`default`, `min`, `max`, `cache`, etc. don't then use a `--memory:filename`
option to define the input filename which would have nothing to do with
the `--memory` command line option).

## One Dash (`-`)

The special argument composed of just one dash (`-`) is viewed as a default
option which, in most cases, will be viewed as an input or output file
stream indication meaning use `stdin` or `stdout` as the file.

## Two Dashes (`--`)

The special argument `--` is the _argument separator_, which separate a
set of arguments with a set of default arguments, in general filenames.
This allos you to have filenames that start with one or two dashes.
Your tool may use this feature for strings that represent something
else than filenames.

## Standalone Parameters

The list of options can include a _default option_. Standalone parameters
(those that cannot be assigned to any option) are added to the default
option.

For example, in:

    command --verbose filename.txt

The `filename.txt` would be added to the default option (assuming that the
`--verbose` is defined as a flag, so it does not eat up the argument follow
argument it).

When a parameter accepts multiple options, only another argument (a word that
start with `--` or a letter or letters that start with a `-`) stops the
processing. The **Two Dashes** can also be used to break the multiple option
argument list and start a list of standalone parameters.


# Logger Extension

The [snaplogger project](https://github.com/m2osw/snaplogger) has an
extension allowing you to add command line options that allow for
the modification of the log mechanism (such as sending the logs to
your console or syslog instead of a file.)

Further, the [eventdispatcher project](https://github.com/m2osw/eventdispatcher)
includes a **snaplogger** extension allowing logs to be sent over a network
connection.


# Hide Warnings

Many tools I use in my console generate warnings (mainly Gnome complaining
about missing this or that).

The `hide-warnings` program can start those tools and hide the warnings
so the console doesn't get filled up by totally useless messages
that I can't do anything about. (I just hope that the next version
won't have all those warnings turned on... it has been years of hope!)

By default, the `hide-warnings` works with a few tools as it defines
the following regular expression which works well enough for most
tools.

    char const * const  g_default_regex = "gtk-warning|gtk-critical|glib-gobject-warning|^$";

Here are a few others I use because the default is not quite enough
for those tools:

    alias gvim="hide-warnings gvim"
    alias inkscape="hide-warnings --regex 'warning|^$' inkscape"
    alias meld="hide-warnings meld"
    alias gimp="hide-warnings --regex 'cannot change name of operation class|glib-gobject-warning|gtk-warning|^$' gimp"
    alias acroread="hide-warnings acroread"
    alias libreoffice="hide-warnings --regex 'WARNING|^$' libreoffice"

As we can see the `gvim` and `meld` work as is. For inkscape, there
were so many warnings that I just turn them all off. The worst is
certainly the Gimp and I still get warnings when loading certain
images. Hopefully you can reuse those aliases too.


# Dependencies

See debian/control for the latest. Here are the dependencies as of
December 2021:

    cmake
    cppthread-dev
    debhelper
    doxygen
    graphviz
    libboost-dev
    libexcept-dev
    libutf8-dev
    snapcatch2
    snapcmakemodules
    snapdev

Non-system dependencies can all be found in our
[m2osw Organization](https://github.com/m2osw).


# License

The source is covered by the MIT license. The debian folder is covered
by the GPL 2.0.


# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/advgetopt/issues).


_This file is part of the [snapcpp project](https://snapwebsites.org/)._
