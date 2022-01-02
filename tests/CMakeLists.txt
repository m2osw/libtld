# Copyright (c) 2011-2022  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/libtld
# contact@m2osw.com
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

##
## TLD tests
##

# Tool to generate the \u... characters for UTF-8 domain names
project(hex)
add_executable(${PROJECT_NAME}
    hex.c
)

# Test the library directly
project(tld_internal_test)
add_executable(${PROJECT_NAME}
    tld_internal_test.cpp
)
# WARNING: we really need to depend on the `tld_data` target, but that is
#          not possible in a parallel cmake; instead we have to depend on
#          a previous item to create a chain of dependencies
add_dependencies(${PROJECT_NAME}
    tld_data
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test the library as an end user
project(tld_test)
add_executable(${PROJECT_NAME}
    tld_test.c
)
target_link_libraries(${PROJECT_NAME}
    tld
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test the tld_valid_uri() function
project(tld_test_full_uri)
add_executable(${PROJECT_NAME}
    tld_test_full_uri.c
)
target_link_libraries(${PROJECT_NAME}
    tld
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test the C++ tld_object
project(tld_test_object)
add_executable(${PROJECT_NAME}
    tld_test_object.cpp
)
target_link_libraries(${PROJECT_NAME}
    tld
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test TLD names as defined by Mozilla
project(tld_test_tld_names)
add_executable(${PROJECT_NAME}
    tld_test_tld_names.cpp
)
target_link_libraries(${PROJECT_NAME}
    tld
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test emails validity
project(tld_test_emails)
add_executable(${PROJECT_NAME}
    tld_test_emails.cpp
)
target_link_libraries(${PROJECT_NAME}
    tld
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test tld_domain_to_lowercase() validity
project(tld_test_domain_lowercase)
add_executable(${PROJECT_NAME}
    tld_test_domain_lowercase.c
)
target_link_libraries(${PROJECT_NAME}
    tld
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test versions validity
project(tld_test_versions)
add_executable(${PROJECT_NAME}
    tld_test_versions.c
)
target_link_libraries(${PROJECT_NAME}
    tld
)
add_test(
    NAME
        ${PROJECT_NAME}

    COMMAND
        ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

##
## Install "run_all_tests.sh" as "unittest" for coverage compatibility
##
project(unittest)
set(TLD_UNITTEST ${PROJECT_BINARY_DIR}/unittest)
add_custom_command(
    OUTPUT
        ${TLD_UNITTEST}

    COMMAND
        cp --verbose run_all_tests.sh ${TLD_UNITTEST}

    MAIN_DEPENDENCY
        run_all_tests.sh

    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}
)
add_custom_target(${PROJECT_NAME} ALL
    DEPENDS
        ${TLD_UNITTEST}
)

###
### Run all the tests
###
#project(run_libtld_tests)
#add_custom_target(run_libtld_tests
#    COMMAND tests/run_all_tests.sh --source-dir "${CMAKE_SOURCE_DIR}"
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    DEPENDS
#        tld_internal_test
#        tld_test
#        tld_test_domain_lowercase
#        tld_test_emails
#        tld_test_full_uri
#        tld_test_object
#        tld_test_tld_names
#        tld_test_versions
#)

# vim: ts=4 sw=4 et
