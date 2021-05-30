#!/bin/sh
#
# Sample script to run make without having to retype the long path each time
# This will work if you built the environment using our ~/bin/build-snap script

PROCESSORS=`nproc`
DEBUG_PATH=../../BUILD/Debug/contrib/advgetopt
RELEASE_PATH=../../BUILD/Release/contrib/advgetopt

case $1 in
"-l")
	make -C ${DEBUG_PATH} 2>&1 | less -SR
	;;

"-v")
	VERBOSE=1 make -C ${DEBUG_PATH} 2>&1 | less -SR
	;;

"-d")
	rm -rf ${DEBUG_PATH}/doc/advgetopt-doc-2.0.tar.gz \
	       ${DEBUG_PATH}/doc/advgetopt-doc-2.0
	make -j${PROCESSORS} -C ${DEBUG_PATH}
	;;

"-t")
	(
		if make -C ${DEBUG_PATH}
		then
			${DEBUG_PATH}/tests/unittest --warn NoTests --progress --tmp `pwd`/tmp/advgetopt "$2"
		fi
	) 2>&1 | less -SR
	;;

"-i")
	make -C ${DEBUG_PATH} install
	;;

"-r")
	make -j${PROCESSORS} -C ${RELEASE_PATH} install
	;;

"")
	make -j${PROCESSORS} -C ${DEBUG_PATH}
	;;

*)
	echo "error: unknown command line option $1"
	exit 1
	;;

esac
