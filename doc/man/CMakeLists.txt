# Copyright (c) 2011-2022  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/libtld
# contact@m2osw.com
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#


##
## Manual Pages
##
project(man)

# We have a special install() call because we need to gzip the files
install(
    CODE "EXECUTE_PROCESS(COMMAND gzip -c9 tldc.1 > ${CMAKE_INSTALL_PREFIX}/share/man/man1/tldc.1.gz)
          EXECUTE_PROCESS(COMMAND gzip -c9 tld.3 > ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld.3.gz)
          EXECUTE_PROCESS(COMMAND gzip -c9 tld.ini.5 > ${CMAKE_INSTALL_PREFIX}/share/man/man5/tld.ini.5.gz)"
)

install(
    CODE "EXECUTE_PROCESS(COMMAND rm ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_clear_info.3.gz \
                                     ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_load_tlds.3.gz \
                                     ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_check_uri.3.gz \
                                     ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_domain_to_lowercase.3.gz \
                                     ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_status_to_string.3.gz)
          EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink tld.3.gz ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_clear_info.3.gz)
          EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink tld.3.gz ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_load_tlds.3.gz)
          EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink tld.3.gz ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_check_uri.3.gz)
          EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink tld.3.gz ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_domain_to_lowercase.3.gz)
          EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink tld.3.gz ${CMAKE_INSTALL_PREFIX}/share/man/man3/tld_status_to_string.3.gz)"
)

# vim: ts=4 sw=4 et
