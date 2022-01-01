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

##
## TLD compiler
##
project(tldc)
add_executable(${PROJECT_NAME}
    tldc.cpp
)
target_link_libraries(${PROJECT_NAME}
    tld
)
install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
set(LIBTLD_TLD_EXTENSIONS_SOURCE ${CMAKE_INSTALL_PREFIX}/share/libtld/tlds)
set(LIBTLD_TLD_EXTENSIONS_DESTINATION /var/lib/libtld/tlds)
install(
    CODE "if(NOT EXISTS ${LIBTLD_TLD_EXTENSIONS_SOURCE}/extensions)
              EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${LIBTLD_TLD_EXTENSIONS_SOURCE})
              EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIBTLD_TLD_EXTENSIONS_DESTINATION} ${LIBTLD_TLD_EXTENSIONS_SOURCE}/extensions)
          endif()"
)

##
## TLD verification command line tool
##
project(validate-tld)
add_executable(${PROJECT_NAME}
    validate_tld.cpp
)
target_link_libraries(${PROJECT_NAME}
    tld
)
install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT runtime
)

# vim: ts=4 sw=4 et
