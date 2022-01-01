#!/bin/bash -e
#
# Run all the tests in order

NAME=`basename $0`
if test "$NAME" = "unittest"
then
	BUILD_PATH=`pwd`/BUILD/tests
else
	BUILD_PATH=../../BUILD/Debug/contrib/libtld/tests
fi

echo "--- tld_internal_test"
${BUILD_PATH}/tld_internal_test
echo "--- tld_test"
${BUILD_PATH}/tld_test
echo "--- tld_test_emails"
${BUILD_PATH}/tld_test_emails
echo "--- tld_test_full_uri"
${BUILD_PATH}/tld_test_full_uri
echo "--- tld_test_godaddy.sh"
(cd tests/; ./tld_test_godaddy.sh ../${BUILD_PATH}/tools)
echo "--- tld_test_object"
${BUILD_PATH}/tld_test_object
echo "--- cp public_suffix_list.dat"
cp tests/public_suffix_list.dat .
echo "--- tld_test_tld_names"
${BUILD_PATH}/tld_test_tld_names
echo "--- tld_test_domain_lowercase"
${BUILD_PATH}/tld_test_domain_lowercase
echo "--- tld_test_versions"
${BUILD_PATH}/tld_test_versions `pwd`
rm public_suffix_list.dat

