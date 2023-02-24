#!/bin/bash -e
#
# Run all the tests in order

NAME=`basename $0`
BUILD=true
if test "$NAME" = "unittest" -a ! -d debian
then
	BUILD_PATH="`pwd`/BUILD"
else
	BUILD_PATH="`cd ../.. && pwd`/BUILD/Debug/contrib/libtld"
fi

SOURCE=`pwd`
while test -n "${1}"
do
	case "$1" in
	"--build")
		shift
		BUILD=true
		;;

	"--no-build")
		shift
		BUILD=false
		;;

	"--progress")
		shift
		;;

	"--sanitize"|"-s")
		shift
		BUILD_PATH="../../BUILD/Sanitize/contrib/libtld"
		;;

	"--source-dir")
		shift
		SOURCE="$1"
		shift
		;;

	"--tmp-dir")
		# At this time we do not use this, although we should have
		# tests to verify the compiler and then it will be useful
		shift
		TEMPDIR="$1"
		shift
		;;

	"--version")
		${BUILD_PATH}/tests/tld_test_versions "${SOURCE}"
		exit 0
		;;

	"--warn")
		# ignore
		shift
		shift
		;;

	"-"*)
		echo "error: unknown command line option \"$1\" to run_all_tests.sh" >&2
		exit 1
		;;

	*)
		echo "error: test names ($1) are not supported yet" >&2
		exit 1
		;;

	esac
done


############################################################################
# By default we want to make sure we have the latest, use --no-build to avoid
# rebuilding each time
if $BUILD
then
	echo "--- build libtld"
	./mk
fi

############################################################################
echo "--- running tests found in $BUILD_PATH from `pwd`"

############################################################################
echo "--- tld_internal_test"
${BUILD_PATH}/tests/tld_internal_test

############################################################################
echo "--- tld_test"
${BUILD_PATH}/tests/tld_test

############################################################################
echo "--- tld_test_emails"
${BUILD_PATH}/tests/tld_test_emails

############################################################################
echo "--- tld_test_full_uri"
${BUILD_PATH}/tests/tld_test_full_uri

############################################################################
echo "--- tld_test_validate.sh"
(
	cd "${SOURCE}"
	tests/tld_test_validate.sh "${BUILD_PATH}/tools"
)

############################################################################
echo "--- tld_test_godaddy.sh"
(
	cd "${SOURCE}"
	tests/tld_test_godaddy.sh "${BUILD_PATH}/tools"
)

############################################################################
echo "--- tld_test_object"
${BUILD_PATH}/tests/tld_test_object

############################################################################
echo "--- tld_test_tld_names"
(
	cd "${SOURCE}"
	${BUILD_PATH}/tests/tld_test_tld_names
)

############################################################################
echo "--- tld_test_domain_lowercase"
${BUILD_PATH}/tests/tld_test_domain_lowercase

############################################################################
echo "--- tld_test_versions"
${BUILD_PATH}/tests/tld_test_versions "${SOURCE:-`pwd`}"


############################################################################
echo "--- success!"
