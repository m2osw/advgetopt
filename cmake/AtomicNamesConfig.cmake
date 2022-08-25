# Copyright (c) 2022  Made to Order Software Corp.  All Rights Reserved
#
# http://snapwebsites.org/project/advgetopt
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
#
################################################################################
#
# File:         AtomicNamesConfig.cmake
# Object:       Provide function to generate atomic name C++ files.
#
include(CMakeParseArguments)

# First try to find the one we just compiled (developer environment)
get_filename_component(CMAKE_BINARY_PARENT_DIR ${CMAKE_BINARY_DIR} DIRECTORY)
get_filename_component(CMAKE_BINARY_PARENT_DIR_NAME ${CMAKE_BINARY_PARENT_DIR} NAME)
if(${CMAKE_BINARY_PARENT_DIR_NAME} STREQUAL "coverage")
    # we have a sub-sub-directory when building coverage
    get_filename_component(CMAKE_BINARY_PARENT_DIR ${CMAKE_BINARY_PARENT_DIR} DIRECTORY)
    get_filename_component(CMAKE_BINARY_PARENT_DIR ${CMAKE_BINARY_PARENT_DIR} DIRECTORY)
endif()
find_program(
    ATOMIC_NAMES_PROGRAM
        atomic-names

    HINTS
        ${CMAKE_BINARY_PARENT_DIR}/advgetopt/tools
        ${CMAKE_BINARY_PARENT_DIR}/contrib/advgetopt/tools

    NO_DEFAULT_PATH
)

# Second, if the first find_program() came up empty handed, try again to
# find an installed version (i.e. generally under /usr/bin)
# This one is marked as REQUIRED.
find_program(
    ATOMIC_NAMES_PROGRAM
        atomic-names

    REQUIRED
)

if(${ATOMIC_NAMES_PROGRAM} STREQUAL "ATOMIC_NAMES_PROGRAM-NOTFOUND")
    message(FATAL_ERROR "atomic-names tool not found")
endif()

# This function generates three outputs files:
#
#    <name>.cpp, <name>.h and <name>_private.h
#
# From a <name>.an file which is expected to include a list of atomic names.
#
# You can define public and private names which are directly used with
# `g_...` names or indirectly through a `get_name()` function. The
# `get_name()` function has to be used from outside your project.
# (i.e. those are public names, their value may change under your feet)
#
# \param[in] ATOMIC_NAME_FILENAME  The name of input file.
#
function(AtomicNames ATOMIC_NAMES)
    cmake_parse_arguments(PARSE_ARGV 1 "" "" "" "")

    file(RELATIVE_PATH RELATIVE_SOURCE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    string(REPLACE "/" "_" INTRODUCER ${RELATIVE_SOURCE_DIR})

    project(${INTRODUCER}_${ATOMIC_NAMES}_AtomicNames)

    get_filename_component(ATOMIC_NAMES_BASENAME ${ATOMIC_NAMES} NAME_WE)

    # Add support for an ${ARG_OPTIONS} parameter to the command
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/${ATOMIC_NAMES_BASENAME}.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/${ATOMIC_NAMES_BASENAME}.h

        COMMAND
            echo "--- atomic names starting ---"

        COMMAND
            "${ATOMIC_NAMES_PROGRAM}"
                    "--output-path"
                        "${CMAKE_CURRENT_BINARY_DIR}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/${ATOMIC_NAMES}"
                    "--verbose"

        COMMAND
            echo "--- atomic names is okay? ---"

        COMMAND
            ls "${CMAKE_CURRENT_BINARY_DIR}"

        WORKING_DIRECTORY
            ${PROJECT_SOURCE_DIR}

        MAIN_DEPENDENCY
            ${ATOMIC_NAMES}

        # Here "atomic-names" is not enough, a full path would be required
        # but since we have the `find_program + REQUIRED` earlier, we're good
        #DEPENDS
        #    "atomic-names"
    )

    add_custom_target(${PROJECT_NAME}
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/${ATOMIC_NAMES_BASENAME}.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/${ATOMIC_NAMES_BASENAME}.h
    )
endfunction()

# vim: ts=4 sw=4 et
