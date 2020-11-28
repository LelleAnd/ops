
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# The main project name
project(OPS VERSION 4.2.0)

set (CMAKE_CXX_STANDARD 11)

################################################################################
# Find external dependencies

## ----------------------------------------------------------------------------
message("GOOGLETEST_HOME: " $ENV{GOOGLETEST_HOME})

# help the FindGtest script locate gtest
if(DEFINED ENV{GOOGLETEST_HOME})
  set(GTEST_ROOT $ENV{GOOGLETEST_HOME})
  list(APPEND CMAKE_PREFIX_PATH $ENV{GOOGLETEST_LIB})
endif()
find_package(GTest)

message("GTEST_INCLUDE_DIRS: " ${GTEST_INCLUDE_DIRS})

## ----------------------------------------------------------------------------
# We need Java 1.8 (JDK 8) or later since Java code uses lambda expressions
find_package(Java 1.8 COMPONENTS Runtime Development)

if(Java_Runtime_FOUND)
  message(STATUS "Found Java version: ${Java_VERSION}")
  include(UseJava)
endif()

if(Java_JAVAC_EXECUTABLE)
  # set JAVA_HOME if we haven't already
  if(NOT DEFINED JAVA_HOME)
    get_filename_component (JAVA_HOME "${Java_JAVAC_EXECUTABLE}" DIRECTORY)
  endif()
else()
  message(WARNING "Could not find a Java compiler")
endif()

## ----------------------------------------------------------------------------
message("BOOST_HOME: " $ENV{BOOST_HOME})

# find Boost. If not found we can't build c++ libs
option(Boost_DEBUG "Set to ON to debug Boost paths" OFF)

# this prevents the FindBoost.cmake to search for cmake-built Boost
set(Boost_NO_BOOST_CMAKE ON)

# if the Boost home environment var is set, use it to hint the FindBoost.cmake about Boost location
if(DEFINED ENV{BOOST_HOME})
  set(BOOST_ROOT $ENV{BOOST_HOME})
endif()
message ("BOOST_ROOT: " ${BOOST_ROOT})

if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  find_package(Boost MODULE COMPONENTS system thread date_time regex chrono)
else()
  set(Boost_ARCHITECTURE "-x64")    ## Needed to work on GitHub CI test 
  find_package(Boost MODULE COMPONENTS system thread)
endif()

message("Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
message("Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
message("Boost_LIBRARIES: " ${Boost_LIBRARIES})

## ----------------------------------------------------------------------------
# Python, we only support 2.7.x
find_package(PythonInterp 2.7 EXACT)
if(PYTHONINTERP_FOUND)
  message("Found Python interpreter version " ${PYTHON_VERSION_STRING})
else()
  message(WARNING "Could not find a Python interpreter")
endif()

## ----------------------------------------------------------------------------
# add compiler flags for C++11 / release / debug
if(MSVC)
#  set(warnings "/W4 /WX /EHsc")
  set(warnings "/W3 /WX /EHsc")
else()
  if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "")
  	set(CMAKE_CXX_FLAGS "-O2")
  	string(REPLACE "-O3 -DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} )
  elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
  	set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage -O0") #fPIC is set
  	string(REPLACE "-g" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} )
  endif()
#  set(warnings "-Wall -Wextra -Werror")
#  set(warnings "-Wall -Wunused-parameter -Wsign-compare -Wcast-qual -Wconversion")
  set(warnings "-Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

message ("c Flags: " ${CMAKE_C_FLAGS})
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

## ----------------------------------------------------------------------------
# If we have java
# we need ANother Tool for Language Recognition
find_jar(ANTLR_JAR antlr
  PATHS Tools/IDLCompiler/dist/lib
# [VERSIONS version1 [version2]]
  DOC "Path to antlr.jar"
  )
# Find "Java Compiler Compiler" archive
find_jar(JAVACC_JAR javacc
  PATHS Tools/IDLCompiler/dist/lib
  DOC "Path to javacc.jar"
  )
# Find jaxmejs. JaxMe is archive and is not maintained!!!
find_jar(JAXMEJS_JAR jaxmejs
  PATHS Tools/IDLCompiler/dist/lib
  VERSIONS 0.5.2
  DOC "Path to jaxmejs.jar"
  )
# Find swing-layout.jar. This is an extension to swing required by NetBeans
find_jar(SWINGLAYOUT_JAR swing-layout
  PATHS Tools/IDLCompiler/dist/lib
#  VERSIONS 1.0.4
  DOC "Path to swing-layout jar"
  )

# set path to where jars should be installed
set(OPS_JAR_REL_INSTALL_PATH lib/ops)

if(NOT CMAKE_BUILD_TYPE MATCHES "Bootstrap")
  # Use opsc script from Bootstrap phase, will be used in all subdirs under this one
  if(WIN32)
    find_file(OPSC_COMPILER NAMES opsc.bat PATHS ${CMAKE_INSTALL_PREFIX}/lib/ops NO_DEFAULT_PATH)
  else()
    find_file(OPSC_COMPILER NAMES opsc.sh PATHS ${CMAKE_INSTALL_PREFIX}/lib/ops NO_DEFAULT_PATH)
  endif()
  message(">>>> OPSC_COMPILER= " ${OPSC_COMPILER})

  # Find Java libraries built during the Bootstrap phase
  set(OPSJAR_PATH ${CMAKE_INSTALL_PREFIX}/${OPS_JAR_REL_INSTALL_PATH})

  find_jar(OPSJLIB_JAR OPSJLib PATHS ${OPSJAR_PATH})
  message(">>>> OPSJLIB_JAR= " ${OPSJLIB_JAR})
  find_jar(CONFIGURATIONLIB_JAR ConfigurationLib PATHS ${OPSJAR_PATH})
  message(">>>> CONFIGURATIONLIB_JAR= " ${CONFIGURATIONLIB_JAR})
endif()

################################################################################
# User Options

include(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
mark_as_advanced(CLEAR BUILD_DOCUMENTATION)

################################################################################
# Build the source

# common parts
add_subdirectory(Common)

# recurse into java source
if(Java_JAVAC_EXECUTABLE)
  # this builds ConfigurationLib and JarSearch
  add_subdirectory(Libs)
  # build OPSJLib
  add_subdirectory(Java)
  # build the rest
  add_subdirectory(Tools)
endif()

if(PYTHONINTERP_FOUND)
  add_subdirectory(Python)
endif()

add_subdirectory(CSharp)

if(CMAKE_BUILD_TYPE MATCHES "Bootstrap")
  # Need to add the install of C++ headers, which is independent on Debug or Opt builds
  add_subdirectory(Cpp)

else()
  # add the C++ source, only if Boost is loaded
  if(Boost_FOUND)
    add_subdirectory(Cpp)

    option(OPS_BUILD_UNITTESTS "Enable to build the UnitTest app(s)" ON)
    if(OPS_BUILD_UNITTESTS AND GTEST_FOUND AND CMAKE_BUILD_TYPE MATCHES "Debug")
      add_subdirectory(UnitTests)
    endif()

  else()
    message(WARNING "install Boost to build OPS C++ libraries")
  endif()

################################################################################
  # Build Examples
  # We need the opsc compiler from the Bootstrap phase to generate code for the examples
    option(OPS_BUILD_EXAMPLES "Enable to build the example applications" ON)
    if(OPS_BUILD_EXAMPLES AND CMAKE_BUILD_TYPE MATCHES "Debug")
      add_subdirectory(Examples)
  endif()
endif()

################################################################################
# present the packaging option only if we have the cpack command defined (effectively >= 2.6.0)
if(CMAKE_CPACK_COMMAND)
    option(OPS_BUILD_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
    if(OPS_BUILD_PACKAGES)
        set(CPACK_PACKAGE_NAME "libops")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libgcc1, libstdc++6")
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Publish-Subscribe communication library")
        set(CPACK_PACKAGE_DESCRIPTION "A LONG DESCRIPTION")
        set(CPACK_PACKAGE_VENDOR "Saab Dynamics AB")
        set(CPACK_PACKAGE_CONTACT "Anton???")
        set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_MAJOR_VERSION})
        set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_MINOR_VERSION})
        set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_PATCH_VERSION})
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME} ${PROJECT_VERSION}")
        set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
        if(WIN32)
          set(CPACK_GENERATOR NSIS ZIP)
          set(CPACK_SOURCE_GENERATOR ZIP)
        else()
          set(CPACK_GENERATOR TGZ)
          set(CPACK_SOURCE_GENERATOR TGZ)
        endif()
        set(CPACK_NSIS_MODIFY_PATH OFF)

        # disable ALL components
        set(CPACK_COMPONENT_OPS_DEV_DISABLED TRUE)
        set(CPACK_COMPONENT_TESTS_DISABLED TRUE)

        include(CPack)
    endif()
endif()
