#!cmake .
#
# See $ENV{APPS}/cmake/ABOUT_CMAKE.md for more information.

cmake_minimum_required( VERSION 3.23 )

project( eol VERSION 0.0 DESCRIPTION "Extract information about text file end-of-line (eol) and other \"features\"" LANGUAGES C CXX )

#-------------------------------------------------------------------------------
# Find project working tree directory containing extern/ and cmake/)
set( _dir "${CMAKE_CURRENT_SOURCE_DIR}" )
cmake_path( GET _dir ROOT_PATH _root )
while( NOT IS_DIRECTORY "${_dir}/extern" AND NOT IS_DIRECTORY "${_dir}/cmake" )
  cmake_path( GET _dir PARENT_PATH _dir )
  if( "${_dir}" STREQUAL "${_root}" )
    message( FATAL_ERROR "Unable to find project workintg tree directory!" )
  endif()
endwhile()
set( WORKTREE_DIR "${_dir}" CACHE PATH "Points to directory containing cmake/ and extern/ subdirectories for the project." )
list( PREPEND CMAKE_MODULE_PATH "${WORKTREE_DIR}/cmake" )
include( project_defaults )

# Packages for this project
#find_package( fmt REQUIRED )

set_target( eol )
add_executable( "${Target}" )
target_sources( "${Target}" PRIVATE eol.c )
#target_link_libraries( "${Target}" PRIVATE fmt::fmt )
add_test( NAME test-"${Target}" COMMAND "${Target}" {:OPTIONS:} )

set_target( mkzero )
add_executable( "${Target}" )
target_sources( "${Target}" PRIVATE mkzero.cpp )
#target_link_libraries( "${Target}" PRIVATE fmt::fmt )
add_test( NAME "${Target}-help" COMMAND "./${Target}" -h )
add_test( NAME "${Target}-one"  COMMAND "./${Target}" 1 nul-one.bin )
add_test( NAME "${Target}-1k"   COMMAND "./${Target}" 1024 nul-1k.bin )
add_test( NAME "${Target}-2000" COMMAND "./${Target}" 2000 nul-2000.bin )
add_test( NAME "${Target}-2k"   COMMAND "./${Target}" 2048 nul-2k.bin )
add_test( NAME "${Target}-rev"  COMMAND "./${Target}" nul-100.bin 100 )
add_test( NAME "${Target}-file" COMMAND "./${Target}" nul-default.bin )
add_test( NAME "${Target}-bad0" COMMAND "./${Target}" )
add_test( NAME "${Target}-badz" COMMAND "./${Target}" 0 nul-badz.bin )
add_test( NAME "${Target}-bad-" COMMAND "./${Target}" -1 nul-bad-.bin )
add_test( NAME "${Target}-bad2" COMMAND "./${Target}" 1 2 )
add_test( NAME "${Target}-bad3" COMMAND "./${Target}" 1 2 3 )
set_tests_properties(
  "${Target}-bad0"
  "${Target}-badz"
  "${Target}-bad3"
  "${Target}-bad-"
  PROPERTIES WILL_FAIL True
)

#> add_subdirectory( tests )
#> include(doxygen)
# vim:syntax=cmake:nospell
