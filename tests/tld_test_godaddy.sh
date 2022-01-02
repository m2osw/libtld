#!/bin/bash -e
#
# At some point, godaddy had an easy to parse page, that's gone, but I still
# test with the old data
#
if ! test -d debian
then
	echo "error: this test is expected to be started from the top source directory."
	exit 1
fi
VERSION=`dpkg-parsechangelog --show-field Version | sed -e 's/~.*//' -e 's/\(^[0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/'`
echo "testing tld godaddy version ${VERSION}"
VALIDATE=
if test -z "$1"
then
	for v in ../../BUILD/Debug/contrib/libtld/tools ../tools
	do
		echo "info: checking \"$v/validate-tld\" `pwd`"
		if test -x $v/validate-tld
		then
			VALIDATE=$v/validate-tld
			break
		fi
	done
else
	VALIDATE=$1/validate-tld
fi
if ! test -x "$VALIDATE"
then
	echo "error: $VALIDATE is not an executable; you supply the wrong path?"
	exit 1;
fi
for f in `sed -e '/^#/ d' tests/godaddy-tlds.txt`
do
	#echo check $f
	$VALIDATE http://www.m2osw$f
done
