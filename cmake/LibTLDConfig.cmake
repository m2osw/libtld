# - Find LibTLD
#
# LIBTLD_FOUND        - System has LibTLD
# LIBTLD_INCLUDE_DIRS - The LibTLD include directories
# LIBTLD_LIBRARIES    - The libraries needed to use LibTLD
# LIBTLD_DEFINITIONS  - Compiler switches required for using LibTLD
#
# License:
#
# Copyright (c) 2011-2025  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/libtld
# contact@m2osw.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

find_path(
    LIBTLD_INCLUDE_DIR
        libtld/tld.h

    PATHS
        ENV LIBTLD_INCLUDE_DIR
)

find_library(
    LIBTLD_LIBRARY
        tld

    PATHS
        ${LIBTLD_LIBRARY_DIR}
        ENV LIBTLD_LIBRARY
)

mark_as_advanced(
    LIBTLD_INCLUDE_DIR
    LIBTLD_LIBRARY
)

set(LIBTLD_INCLUDE_DIRS ${LIBTLD_INCLUDE_DIR})
set(LIBTLD_LIBRARIES    ${LIBTLD_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibTLD
    REQUIRED_VARS
        LIBTLD_INCLUDE_DIR
        LIBTLD_LIBRARY
)

# vim: ts=4 sw=4 et
