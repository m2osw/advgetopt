# Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved.
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
## AdvGetopt library
##
project(advgetopt)

execute_process(
    COMMAND
        g++ --version

    COMMAND
        sed -e "2,$ d"

    OUTPUT_VARIABLE
        ADVGETOPT_COMPILER_VERSION

    OUTPUT_STRIP_TRAILING_WHITESPACE
)


# Put the version in the header file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

include_directories(
    ${CPPTHREAD_INCLUDE_DIRS}
    ${LIBEXCEPT_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
    ${LIBUTF8_INCLUDE_DIRS}
)

add_library( ${PROJECT_NAME} SHARED
    advgetopt.cpp
    advgetopt_access.cpp
    advgetopt_config.cpp
    advgetopt_data.cpp
    advgetopt_options.cpp
    advgetopt_usage.cpp
    conf_file.cpp
    license_gpl2.cpp
    option_info.cpp
    option_info_ref.cpp
    utils.cpp
    validator.cpp
    version.cpp
)


target_link_libraries(${PROJECT_NAME}
    ${CPPTHREAD_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
    ${LIBUTF8_LIBRARIES}
    pthread
)


set_target_properties( ${PROJECT_NAME} PROPERTIES
    VERSION
        ${LIBADVGETOPT_VERSION_MAJOR}.${LIBADVGETOPT_VERSION_MINOR}
    SOVERSION
        ${LIBADVGETOPT_VERSION_MAJOR}
)

install(
    TARGETS
        ${PROJECT_NAME}
    RUNTIME DESTINATION
        bin
    LIBRARY DESTINATION
        lib
    ARCHIVE DESTINATION
        lib
)

install(
    FILES
        advgetopt.h
        conf_file.h
        exception.h
        flags.h
        licenses.h
        option_info.h
        options.h
        utils.h
        validator.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.h
    DESTINATION
        include/advgetopt
)

# vim: ts=4 sw=4 et nocindent
