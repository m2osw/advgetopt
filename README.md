
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

The command line options. that your application understands, are created
in a static array. By this we mean that it gets created at compile time
and the array is just there at runtime. (This does not prevent you from
creating dynamic command line options, but I do not know of very many
software that have command line options changing over time, do you?)

The array is built with one entry per option. If you have options that
make use of different names but have exactly the same result, then you
will need multiple entries in your array. The additional entries can
then make use of the `GETOPT_FLAG_ALIAS` and reference the first entry.

Options can be used on the command line, in your environment variable,
or in a configuration file. You can select exactly which option is going
to apply to what is found on the command line, in the configuration file,
or the environment variables using the following flags:

    GETOPT_FLAG_COMMAND_LINE
    GETOPT_FLAG_ENVIRONMENT_VARIABLE
    GETOPT_FLAG_CONFIGURATION_FILE

Further, you probably do not want all the options to be shown whenever
an error is detected. For this reason there is a flag to tell that
this or that option has to be shown on an error:

    GETOPT_FLAG_SHOW_USAGE_ON_ERROR


# Command Line

The `getopt` class accepts the `argc`/`argv` parameters directly from your
`main()` function. Of course, you can also build another array and use the
`getopt` class at any time.


# Configuration Files

You can pass a vector of paths with filenames. If the vector is
empty then no configuration files are checked. Otherwise the filenames
are expected to be full paths, even though it is not required to be.

Configuration files support empty lines and lines that start with
a hash that are viewed as comments. Spaces can appear before the hash.

The other entries have to be of the form:

    option_name=value

Note that in the configuration file, no `-` or `--` can appear in
front of the option name.

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
available at the command line level. The available of these extra
features are therefore available only in a few tools and services.
These tools and services will describe the other features they support.

The possible features are:

* Accept a space as the separator between the name of a parameter and its
  value.
* Accept a colon (`:`) as the assignment operator, instead of the equal
  sign.
* Accept variable references in the values, written as `${<name>}`.
* Accept INI like section declarations `[<name>]`.
* Accept C-like like section declarations `<name> { ... }`.
* Accept C++-like scoping `<scope>::<name>=<value>`.
* Multi-line parameters with backslash, & at the end or the start of the line.
* Load all files found in a sub-directory (usually named `<project>.d`)
  sorting files by the two digit number at the start of the filename
  (`XX-<name>.conf`).


## `edit-config`

This tool can be used to edit simple Unix-like configuration file. You can
either retrieve the value of a field or set the value of a field.

    edit-config <config>.conf <field-name> [<new-value>]

Say you have file named "/etc/log/settings.conf" with the following
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


# Numbers

The options can be marked as optional or required.

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

If the the option is not a valid integer then the function automatically
calls `usage()` which ends with `exit(1)`.

We also offer a `get_double()` and validators. Validators allow for
other types to be checked:

* Integers
* Doubles
* Durations (such as `3m 15m 45s`)
* Size (such as `<number> Kb`)
* Regular Expression (any regex)


# Environment Variables

## The Global Environment Variable

The variables from the environment can be checked for parameters.

The contents of the global environment variable are viewed just as a
command with options. There are no limits except that only options that
were authorized to appear in the variable can be used in that variable.
To authorize an option variable, set the following flag:

    GETOPT_FLAG_ENVIRONMENT_VARIABLE

The reason for not letting any parameter to work in the global environment
variable (or a configuration file, if that matter) is because certain
options do not make sense in variables.

For example, if you support a `--help` option, it most certainly
would be totally useless in the environment variable.

Using an option that's not acceptable in an environment variable
will break the command line parsing by generating an error.

One reason to support environment variables is to define things
like a path which has to be specific to each user of your tool:

    MY_PROJECT_CONF="--path /usr/bin"

Note that the command line option is required with the `-` or `--`
as expected.

## Per Option Environment Variables

If you want to use an environment variable to just hold the value of
an option, then use the environment variable name in the option.

The value will be checked just like a normal option.

This features allows your services to be compatible with systems such
as Kubernetes, which make use of environment variables instead of
command line arguments. With such, you can use the configuration
file for options that do not need to be dynamically changed or are
specific to a system that runs that service (i.e. such as the local IP).


# Project Name

You can define the name of your project in the list of configuration
files. This is done by adding the `@` character at the start and end
of the string as in:

    "@advgetopt@" // project name is "advgetopt"

The name of the project is used to load configuration files by adding
the name and a `.d` to the path. Say you have a configuration path
set to `/etc/my-project/file.conf` and you defined a project name
`sub-name` then the code will look for additional configuration files
named:

    /etc/my-project/sub-name.d/XX-file.conf

where XX is a number from 00 to 99.

At times, the project name is not practical because we want several
services to make use of the same directory. In that case, we offer a
group name instead. That allows to either separate the files or instead
group them under the same sub-directory.

_Note that although it is not currently checked, you probably should
not include slashes in your project name._


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

_Note: the `ipmgr` project also comes with a tool named `dns-options`
allowing for the editing of DNS configuration files. It is still in
development as it doesn't yet work properly in all situations, but
works for what we currently need it for. It may also be useful to you._

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
about missing this or that.)

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
