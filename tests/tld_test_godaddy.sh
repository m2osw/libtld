#!/bin/bash -e
#
# At some point, godaddy had an easy to parse page, that's gone, but I still
# test with the old data
#
if test -d debian
then
	VERSION=`dpkg-parsechangelog --show-field Version | sed -e 's/~.*//' -e 's/\(^[0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/'`
else
	VERSION=`cd .. && dpkg-parsechangelog --show-field Version | sed -e 's/~.*//' -e 's/\(^[0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/'`
fi
echo "testing tld godaddy version ${VERSION}"
VALIDATE=
if test -z "$1"
then
	for v in . .. ../.. build/tools BUILD/tools ../BUILD/tools ../build/tools ../BUILD/libtld/tools ../build/libtld/tools
	do
		if test -x $v/validate-tld
		then
			VALIDATE=$v/validate-tld
			break
		fi
	done
else
	if test -x $1/validate-tld
	then
		VALIDATE=$1/validate-tld
	fi
fi
if test -z "$VALIDATE"
then
	echo "error: could not find validate-tld"
	exit 1;
fi
for f in `sed -e '/^#/ d' godaddy-tlds.txt`
do
	#echo check $f
	$VALIDATE http://www.m2osw$f
done
