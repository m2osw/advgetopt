# - Find AdvGetOpt
#
# ADVGETOPT_FOUND        - System has AdvGetOpt
# ADVGETOPT_INCLUDE_DIRS - The AdvGetOpt include directories
# ADVGETOPT_LIBRARIES    - The libraries needed to use AdvGetOpt
# ADVGETOPT_DEFINITIONS  - Compiler switches required for using AdvGetOpt
#
# License:
#
# Copyright (c) 2011-2022  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/advgetopt
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
    ADVGETOPT_INCLUDE_DIR
        advgetopt/advgetopt.h

    PATHS
        ENV ADVGETOPT_INCLUDE_DIR
)

find_library(
    ADVGETOPT_LIBRARY
        advgetopt

    PATHS
        ${ADVGETOPT_LIBRARY_DIR}
        ENV ADVGETOPT_LIBRARY
)    

mark_as_advanced(
    ADVGETOPT_INCLUDE_DIR
    ADVGETOPT_LIBRARY
)

set(ADVGETOPT_INCLUDE_DIRS ${ADVGETOPT_INCLUDE_DIR})
set(ADVGETOPT_LIBRARIES    ${ADVGETOPT_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    AdvGetOpt
    REQUIRED_VARS
        ADVGETOPT_INCLUDE_DIR
        ADVGETOPT_LIBRARY
)

# vim: ts=4 sw=4 et nocindent
