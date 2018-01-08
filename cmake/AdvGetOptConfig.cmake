# - Try to find AdvGetOpt (advgetopt)
#
# Once done this will define
#
# ADVGETOPT_FOUND        - System has AdvGetOpt
# ADVGETOPT_INCLUDE_DIRS - The AdvGetOpt include directories
# ADVGETOPT_LIBRARIES    - The libraries needed to use AdvGetOpt
# ADVGETOPT_DEFINITIONS  - Compiler switches required for using AdvGetOpt (none)

find_path( ADVGETOPT_INCLUDE_DIR advgetopt/advgetopt.h
			PATHS ENV ADVGETOPT_INCLUDE_DIR
		 )
find_library( ADVGETOPT_LIBRARY advgetopt
			PATHS ${ADVGETOPT_LIBRARY_DIR} ENV ADVGETOPT_LIBRARY
		 )
mark_as_advanced( ADVGETOPT_INCLUDE_DIR ADVGETOPT_LIBRARY )

set( ADVGETOPT_INCLUDE_DIRS ${ADVGETOPT_INCLUDE_DIR} )
set( ADVGETOPT_LIBRARIES    ${ADVGETOPT_LIBRARY}     )

include( FindPackageHandleStandardArgs )

# handle the QUIETLY and REQUIRED arguments and set ADVGETOPT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args( AdvGetOpt DEFAULT_MSG ADVGETOPT_INCLUDE_DIR ADVGETOPT_LIBRARY )

# vim: ts=4 sw=4 noexpandtab nocindent
