#!/bin/sh
#
# Sample script to run make without having to retype the long path each time
# This will work if you built the environment using our ~/bin/build-snap script

PROCESSORS=`nproc`
PROJECT_PATH=../../BUILD/Debug/contrib/advgetopt

case $1 in
"-l")
	make -C ${PROJECT_PATH} 2>&1 | less -SR
	;;

"-v")
	VERBOSE=1 make -C ${PROJECT_PATH} 2>&1 | less -SR
	;;

"-d")
	rm -rf ${PROJECT_PATH}/doc/advgetopt-doc-2.0.tar.gz \
	       ${PROJECT_PATH}/doc/advgetopt-doc-2.0
	make -j${PROCESSORS} -C ${PROJECT_PATH}
	;;

"-t")
	(
		if make -C ${PROJECT_PATH}
		then
			${PROJECT_PATH}/tests/unittest --warn NoTests --progress --tmp `pwd`/tmp/advgetopt "$2"
		fi
	) 2>&1 | less -SR
	;;

"-i")
	make -C ${PROJECT_PATH} install
	;;

"-r")
	make -j${PROCESSORS} -C ../../BUILD/Release/contrib/advgetopt install
	;;

"")
	make -j${PROCESSORS} -C ${PROJECT_PATH}
	;;

*)
	echo "error: unknown command line option $1"
	exit 1
	;;

esac
