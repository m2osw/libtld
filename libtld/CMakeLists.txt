# Copyright (c) 2011-2021  Made to Order Software Corp.  All Rights Reserved
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

####
#### TLD parser
####
##project(tld_parser)
##include_directories(
##    ${CMAKE_BINARY_DIR}
##    ${tld_library_SOURCE_DIR}
##    ${QT_INCLUDES}
##)
##add_executable(${PROJECT_NAME}
##    tld_parser.cpp
##)
##target_link_libraries(${PROJECT_NAME}
##    ${QT_LIBRARIES}
##)
##
####
#### TLD data
####
##project(tld_data)
##set(TLD_DATA_C ${PROJECT_SOURCE_DIR}/tld_data.c)
##add_custom_command(
##    OUTPUT ${TLD_DATA_C}
##    COMMAND tld_parser ${PROJECT_SOURCE_DIR}
##    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
##    MAIN_DEPENDENCY tld_data.xml
##    DEPENDS tld_parser
##)
##add_custom_target(${PROJECT_NAME} ALL DEPENDS ${TLD_DATA_C})
##define_property(SOURCE
##    PROPERTY GENERATED
##    BRIEF_DOCS "The tld_data.c file is a table of all the TLDs defined in tld_data.xml"
##    FULL_DOCS "Once packaged, the tld_data.c file doesn't need to be regenerated (assuming the date and time of the source package is properly taken in account.)"
##)

##
## TLD library
##
project(tld)
configure_file(
    tld.h.in
    ${PROJECT_BINARY_DIR}/tld.h
)
add_library(${PROJECT_NAME} SHARED
    tld.c
    tld_domain_to_lowercase.c
    tld_emails.cpp
    tld_object.cpp
    tld_compiler.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${LIBTLD_VERSION_MAJOR}.${LIBTLD_VERSION_MINOR}
    SOVERSION ${LIBTLD_VERSION_MAJOR}
)
install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    COMPONENT runtime
)
install(
    FILES ${PROJECT_BINARY_DIR}/tld.h
    DESTINATION include
    COMPONENT development
)

##
## TLD static library
##
project(tld_static)
add_library(${PROJECT_NAME} STATIC
    tld.c
    tld_domain_to_lowercase.c
    tld_emails.cpp
    tld_object.cpp
    tld_compiler.cpp
)
#target_include_directories(${PROJECT_NAME}
#    PUBLIC
#        ${PROJECT_BINARY_DIR}
#        ${PROJECT_SOURCE_DIR}
#)
# We need the -fPIC to use this library as extension of PHP, etc.
set_target_properties(tld_static PROPERTIES COMPILE_FLAGS -fPIC)

install(
    TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    COMPONENT development
)

# vim: ts=4 sw=4 et
