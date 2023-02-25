# Copyright (c) 2011-2023  Made to Order Software Corp.  All Rights Reserved
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

##
## TLD parser
##
project(tld_parser)
add_executable(${PROJECT_NAME}
    ../tools/tldc.cpp

    tld_compiler.cpp
    tld_file.cpp
    tld_strings.c
)

##
## TLD data
##
project(tld_data)
set(TLD_DATA_C ${PROJECT_BINARY_DIR}/tld_data.c)
file(GLOB_RECURSE TLD_FILES ${CMAKE_SOURCE_DIR}/conf/tlds/*.ini)
add_custom_command(
    OUTPUT ${TLD_DATA_C}
    COMMAND
        "./tld_parser"
            "--source"
                "${CMAKE_SOURCE_DIR}/conf/tlds"
            "--verify"
            "--output-json"
                "--include-offsets"
            "--c-file"
                "${TLD_DATA_C}"
            "tlds.tld"
    DEPENDS tld_parser ${TLD_FILES}
)
add_custom_target(${PROJECT_NAME}
    DEPENDS ${TLD_DATA_C}
)
define_property(SOURCE
    PROPERTY GENERATED
    BRIEF_DOCS "The tld_data.c file is a table of all the TLDs defined in conf/tlds/... .ini files."
    FULL_DOCS "In the new version, the tld_data.c is always generated on the fly since it can be done with just C/C++."
)

set(LIBTLD_SOURCES
    tld.cpp
    tld_compiler.cpp
    ${TLD_DATA_C}
    tld_domain_to_lowercase.c
    tld_emails.cpp
    tld_file.cpp
    tld_object.cpp
    tld_strings.c
)

##
## TLD library
##
project(tld)
configure_file(
    tld.h.in
    ${PROJECT_BINARY_DIR}/tld.h
)
add_library(${PROJECT_NAME} SHARED
    ${LIBTLD_SOURCES}
)
add_dependencies(${PROJECT_NAME}
    tld_data
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${LIBTLD_VERSION_MAJOR}.${LIBTLD_VERSION_MINOR}
    SOVERSION ${LIBTLD_VERSION_MAJOR}
)
install(
    TARGETS
        ${PROJECT_NAME}

    LIBRARY DESTINATION
        lib

    COMPONENT
        runtime
)
install(
    FILES
        ${PROJECT_BINARY_DIR}/tld.h
        tld_compiler.h
        tld_file.h

    DESTINATION
        include/libtld

    COMPONENT
        development
)

##
## TLD static library
##
project(tld_static)
add_library(${PROJECT_NAME} STATIC
    ${LIBTLD_SOURCES}
)
add_dependencies(${PROJECT_NAME}
    tld_data
)
# We need the -fPIC to use this library as extension of PHP, etc.
set_target_properties(tld_static PROPERTIES COMPILE_FLAGS -fPIC)

install(
    TARGETS
        ${PROJECT_NAME}

    ARCHIVE DESTINATION
        lib

    COMPONENT
        development
)

# vim: ts=4 sw=4 et
