#!/bin/sh
#
# Sample script to run make without having to retype the long path each time
# This will work if you built the environment using our ~/bin/build-snap script

PROCESSORS=`nproc`

case $1 in
"-l")
	make -C ../../../BUILD/contrib/advgetopt 2>&1 | less -SR
	;;

"-v")
	VERBOSE=1 make -C ../../../BUILD/contrib/advgetopt 2>&1 | less -SR
	;;

"-d")
	rm -rf ../../../BUILD/contrib/advgetopt/doc/advgetopt-doc-2.0.tar.gz \
	       ../../../BUILD/contrib/advgetopt/doc/advgetopt-doc-2.0
	make -j${PROCESSORS} -C ../../../BUILD/contrib/advgetopt
	;;

"-t")
	(
		if make -C ../../../BUILD/contrib/advgetopt
		then
			../../../BUILD/contrib/advgetopt/tests/unittest --warn NoTests --progress --tmp `pwd`/tmp/advgetopt "$2"
		fi
	) 2>&1 | less -SR
	;;

"-i")
	make -C ../../../BUILD/contrib/advgetopt install
	;;

"")
	make -j${PROCESSORS} -C ../../../BUILD/contrib/advgetopt
	;;

*)
	echo "error: unknown command line option $1"
	exit 1
	;;

esac
