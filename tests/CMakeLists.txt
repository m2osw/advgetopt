# Copyright (c) 2006-2021  Made to Order Software Corp.  All Rights Reserved
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

# Note that if user does not have Catch then this does not get included
# in the final output
#
project(unittest)

find_package(Catch2)
if(Catch2_FOUND)

    add_definitions(
        -DCATCH_CONFIG_PREFIX_ALL
    )

    include_directories(
        ${CMAKE_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
        ${CATCH_INCLUDE_DIR}
        ${LIBEXCEPT_INCLUDE_DIRS}
    )

    add_executable( ${PROJECT_NAME}
        catch_main.cpp

        catch_access.cpp
        catch_arguments.cpp
        catch_config.cpp
        catch_config_file.cpp
        catch_data.cpp
        catch_log_for_test.cpp
        catch_logger.cpp
        catch_option_info.cpp
        catch_option_info_ref.cpp
        catch_options_files.cpp
        catch_options_parser.cpp
        catch_options_sources.cpp
        catch_usage.cpp
        catch_utils.cpp
        catch_validator.cpp
        catch_version.cpp
    )

    target_link_libraries( ${PROJECT_NAME}
        advgetopt
    )

    set( TMPDIR "${CMAKE_BINARY_DIR}/tmp" )
    if( NOT EXISTS ${TMPDIR} )
        file( MAKE_DIRECTORY ${TMPDIR} )
    endif()

    #unset( DEBUG_WORD )
    #if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
    #    set( DEBUG_WORD "_d" )
    #endif()
    add_test( unittests ${PROJECT_NAME} --tmp ${TMPDIR} )

endif( Catch2_FOUND )

# vim: ts=4 sw=4 et
