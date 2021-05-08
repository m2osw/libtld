#!/bin/sh
#
# Sample script to run make without having to retype the long path each time
# This will work if you built the environment using our ~/bin/build-snap script

PROJECT=libtld
PROCESSORS="`nproc`"
BUILDDIR="../../../BUILD/contrib/${PROJECT}"

# "Brief Version" -- for the documentation
VERSION=`dpkg-parsechangelog --show-field Version | sed -e 's/~.*//' -e 's/\(^[0-9]\+\.[0-9]\+\).*/\1/'`


case $1 in
"-l")
	make -C ${BUILDDIR} 2>&1 | less -SR
	;;

"-d")
	rm -rf ${BUILDDIR}/doc/${PROJECT}-doc-${VERSION}.tar.gz
	make -C ${BUILDDIR}
	;;

"-i")
	make -j${PROCESSORS} -C ${BUILDDIR} install
	;;

"-t")
	(
		if make -j${PROCESSORS} -C ${BUILDDIR}
		then
			shift
			${BUILDDIR}/tests/unittest --progress $*
		fi
	) 2>&1 | less -SR
	;;

"-r")
	make -j${PROCESSORS} -C ${BUILDDIR}
	;;

"")
	make -j${PROCESSORS} -C ${BUILDDIR}
	;;

*)
	echo "error: unknown command line option \"$1\""
	;;

esac
