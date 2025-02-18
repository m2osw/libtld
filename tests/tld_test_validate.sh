#!/bin/bash -e
#
# This is a script used to verify the validate-tld command line tool
#

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

if ! test -x "${VALIDATE}"
then
	echo "error: ${VALIDATE} is not an executable; you supplied the wrong path?"
	exit 1;
fi

echo "found validate-tld here: ${VALIDATE}"

#${VALIDATE} --version

set +e
VERSION=`${VALIDATE} --version`
set -e
echo "testing validate-tld tool version ${VERSION}"


############################################################################
if ${VALIDATE} -h
then
    echo "\`validate-tld -h\` did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} --help
then
    echo "\`validate-tld --help\` did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} -l
then
    echo "\`validate-tld -l\` did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} --list
then
    echo "\`validate-tld --list\` did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} -v --version
then
    echo "\`validate-tld -v --version\` did not exit with an error";
    exit 1;
fi

############################################################################
if ! ${VALIDATE} 'http://www.m2osw.com/validate/tld?perfect=work'
then
    echo "\`validate-tld <valid URI>\` did exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} 'http://www.m2osw.comm/validate/tld?perfect=work'
then
    echo "\`validate-tld <invalid TLD>\` did not exit with an error";
    exit 1;
fi

############################################################################
if ! ${VALIDATE} 'http://valid.com.mx/'
then
    echo "\`validate-tld <valid TLD>\` failed with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} 'http://.com.mx/missing/domain'
then
    echo "\`validate-tld <only TLD is invalid>\` did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} 'http://edu.mx/missing/domain'
then
    echo "\`validate-tld <only TLD is invalid>\` did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} 'http://edu.mx:443/missing/domain'
then
    echo "\`validate-tld <only TLD is invalid>\` did not exit with an error";
    exit 1;
fi

############################################################################
if ! ${VALIDATE} 'http://.other.mx/valid/domain'
then
    echo "\`validate-tld <valid domain and TLD>\` exited with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} --schemes ftp http://www.m2osw.com/bad/scheme
then
    echo "\`validate-tld --schemes ftp <wrong scheme>\` did not exit with an error";
    exit 1;
fi

############################################################################
if ! ${VALIDATE} --schemes ftp ftp://www.m2osw.com/matching/scheme
then
    echo "\`validate-tld --schemes ftp <valid URI>\` did exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} 'mailto:"alexis@m2osw.com'
then
    echo "\`validate-tld <invalid email>\` did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} 'mailto:alexis@m2osw.com"'
then
    echo "\`validate-tld <invalid email>\` did not exit with an error";
    exit 1;
fi

############################################################################
if ! ${VALIDATE} mailto:alexis@m2osw.com
then
    echo "\`validate-tld <valid email>\` did exit with an error";
    exit 1;
fi

############################################################################
if ! ${VALIDATE} -v mailto:alexis@m2osw.com
then
    echo "\`validate-tld -v <valid email>\` did exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} --verbose
then
    echo "\`validate-tld --verbose\` (no TLD) did not exit with an error";
    exit 1;
fi

############################################################################
if ${VALIDATE} --schemes
then
    echo "\`validate-tld --schemes\` (no scheme parameter) did not exit with an error";
    exit 1;
fi

# considered valid!
exit 0
