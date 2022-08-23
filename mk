#!/bin/sh
#
# See the snapcmakemodules project for details about this script
#     https://github.com/m2osw/snapcmakemodules

# Additional arguments to run the tests
#
#export PROJECT_TEST_ARGS="--tmp `pwd`/tmp/advgetopt"

# In case you are a programmer and have this parameter defined in your
# shell, it breaks the tests so just remove the parameter here
#
unset ADVGETOPT_OPTIONS_FILES_DIRECTORY

if test -x ../../cmake/scripts/mk
then
	../../cmake/scripts/mk $*
else
	echo "error: could not locate the cmake mk script"
	exit 1
fi

