
<p align="center">
<img alt="Snap! Websites" src="blob/master/doc/advgetopt-277x277.png" width="277" height="277"/>
</p>

# Introduction

The `advgetopt` library provides the `advgetopt::getopt` class, which
parses command line options (from argc/argv). It is very advanced and
is capable of reading many different types of options with a letter
(-h) and a word (--verbose) with no parameters, one parameter, any
number of parameters, and a set of "filenames," parameters without
a flag first.

The class accepts input from the command line, an environment variable,
and configuration files.


# Understood Commands Options

The command line options that your application understands are created
in a static array. This means it is built at compile time and the array
is just there at runtime.

The array is built with one entry per option. If you have options that
make use of different names but have exactly the same result, then you
will need multiple entries in your array. The following entries can
then make use of the `GETOPT_FLAG_ALIAS` and reference the first entry.

All options can be used on the command line. However, you can select
exactly which option is going to apply to what we find in the
configuration file or the environment variable using the following
two files:

    GETOPT_FLAG_ENVIRONMENT_VARIABLE
    GETOPT_FLAG_CONFIGURATION_FILE

Further, you probably do not want all the flags to be shown whenever
an error is detected. For this reason there is a flag to tell that
this or that flag has to be shown on an error.

    GETOPT_FLAG_SHOW_USAGE_ON_ERROR


# Command Line

The `getopt` class accepts the argc/argv from your main() function.
Of course, you can also build another array and use the `getopt`
class at any time.


# Configuration Files

You can pass a vector of paths with filenames. If the vector is
empty then no configuration files is checked. Otherwise the filenames
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


# Environment Variables

The variables from the environment can be checked for parameters.

The content of the environment variable is viewed just as a command
with options. There is no limit except for the options that can be
used in a variable. Only those marked with this flag can appear in
environment variables:

    GETOPT_FLAG_ENVIRONMENT_VARIABLE

The reason for not letting any parameter to work in an environment
variable (or a configuration file, if that matter) is because certain
options do not make sense in variables.

For example, if you support a `--help` option, it most certainly
would be totally useless in an environment variable.

Using an option that's not acceptable in an environment variable
will break the command line parsing by generating an error.

One reason to support environment variables is to define things
like a path which has to be specific to each user of your tool:

    MY_PROJECT_CONF="--path /usr/bin"

Note that the command line option is required with the `-` or `--`
as expected. If you want to use an environment variable as a path,
then you should just get that variable content and use it as such
and not use it with `advgetopt` which views variables as a set
of command line options.


# Project Name

You can define the name of your project in the list of configuration
files. This is done by adding the `@` character at the start and end
of the string as in:

    "@advgetopt@" // project name is "advgetopt"

The name of the project is used to load configuration files by adding
the name and a `.d` to the path. Say you have a configuration path
set to `/etc/my-project/file.conf` and you defined a project name
`sub-name` then the code will look for a configuration file under:

    /etc/my-project/sub-name.d/file.conf

Note that although it is not checked, you probably should not include
slashes in your project name.


# Hide Warnings

Many tools I use in my console generate warnings.

The `hide-warnings` program can start those tools and hide the warnings
so the console doesn't get filled up by totally useless warnings
that I can't do anything about. (I just hope that the next version
won't have all those warnings turned on...)

By default, the `hide-warnings` works with a few tools as it defines
the following regular expression which works well enough for most
tools.

    char const * const  g_default_regex = "gtk-warning|gtk-critical|glib-gobject-warning|^$";

There are a few others I use because the default is not quite enough
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


# License

The source is covered by the MIT license. The debian folder is covered
by the GPL 2.0.


# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/advgetopt/issues).


_This file is part of the [snapcpp project](https://snapwebsites.org/)._
