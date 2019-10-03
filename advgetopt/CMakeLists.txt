# Copyright (c) 2006-2019  Made to Order Software Corp.  All Rights Reserved.
# https://snapwebsites.org/
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

##
## AdvGetopt library
##
project(advgetopt)

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
