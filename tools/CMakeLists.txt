# Copyright (c) 2006-2022  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/advgetopt
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


##
## build a file of command line options compatible with advgetopt
##
project(build-file-of-options)

add_executable(${PROJECT_NAME}
    build_file_of_options.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ADVGETOPT_INCLUDE_DIRS}
        ${LIBEXCEPT_INCLUDE_DIRS}
        ${SNAPDEV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    advgetopt
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    COMPONENT
        runtime
)


##
## hide warning command line tool
##
project(hide-warnings)

add_executable(${PROJECT_NAME}
    hide_warnings.cpp
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    COMPONENT
        runtime
)



##
## edit-config command line tool
##
project(edit-config)

add_executable(${PROJECT_NAME}
    edit_config.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ADVGETOPT_INCLUDE_DIRS}
        ${LIBEXCEPT_INCLUDE_DIRS}
        ${SNAPDEV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    advgetopt
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    COMPONENT
        runtime
)

# vim: ts=4 sw=4 et nocindent
