# Copyright (c) 2021  Made to Order Software Corp.  All Rights Reserved
#
# http://snapwebsites.org/project/advgetopt
# contact@m2osw.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
################################################################################
#
# File:         AtomicNamesConfig.cmake
# Object:       Provide function to generate atomic name C++ files.
#
include( CMakeParseArguments )

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
            "atomic-names"
                    "--output-path"
                        "${CMAKE_CURRENT_BINARY_DIR}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/${ATOMIC_NAMES}"

        WORKING_DIRECTORY
            ${PROJECT_SOURCE_DIR}

        MAIN_DEPENDENCY
            ${ATOMIC_NAMES}

        # Add "atomic-names" itself (but we need a full path...
        # we may want to have a form of "find . -name atomic-names")
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
