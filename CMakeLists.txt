
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# The main project name
project(OPS VERSION 4.2.0)

################################################################################
#
# Symbols that can be set to affect/control the build process:
#   Environment:
#     Help CMake find different tools/libs
#       GOOGLETEST_HOME             -
#       GOOGLETEST_LIB              -
#       JAVA_HOME                   -
#       Python_ROOT_DIR             -
#       BOOST_HOME or BOOST_ROOT    -
#
#     When C++ has defined USE_FIXED_LENGTH_STRINGS:
#       OPS_FIXED_OBJECT_NAME_SIZE  default: 50
#       OPS_FIXED_MESSAGE_KEY_SIZE  default: 60
#       OPS_FIXED_TYPE_ID_SIZE      default: 256
#       OPS_FIXED_CHANNEL_ID_SIZE   default: 20
#       OPS_FIXED_FILENAME_SIZE     default: 1024
#
#   CMAKE:
#     CMAKE_BUILD_TYPE              Bootstrap / Debug / Opt
#     BOOST_ARCH                    -
#     OPS_BUILD_UNITTESTS           default: ON
#     OPS_BUILD_EXAMPLES            default: ON
#
#     When using OPS as a sub project, the following CMAKE vars is set in the parent scope:
#       OPSC_COMPILER               Full path to the opsc compiler (also set as an opsc target property)
#
################################################################################

set (CMAKE_CXX_STANDARD 11)

message(STATUS ">>> CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS ">>> OPS_BUILD_UNITTESTS: " ${OPS_BUILD_UNITTESTS})
message(STATUS ">>> OPS_BUILD_EXAMPLES: " ${OPS_BUILD_EXAMPLES})

if(CMAKE_BUILD_TYPE MATCHES "Bootstrap")
  set(OPS_BOOTSTRAP_BUILD TRUE)
else()
  set(OPS_BOOTSTRAP_BUILD FALSE)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
  message(STATUS "Has a parent scope.")
  set(OPS_NO_SEPARATE_BOOTSTRAP TRUE)
  message(STATUS "Building OPS without a separate Bootstrap phase")
else()
  message(STATUS "Doesn't have a parent scope.")
  message(STATUS "Building OPS with a separate Bootstrap phase")
endif()

################################################################################
# Find external dependencies

## ----------------------------------------------------------------------------
message(STATUS "GOOGLETEST_HOME: " $ENV{GOOGLETEST_HOME})

# help the FindGtest script locate gtest
if(DEFINED ENV{GOOGLETEST_HOME})
  set(GTEST_ROOT $ENV{GOOGLETEST_HOME})
  list(APPEND CMAKE_PREFIX_PATH $ENV{GOOGLETEST_LIB})
endif()
find_package(GTest)

message(STATUS "GTEST_INCLUDE_DIRS: " ${GTEST_INCLUDE_DIRS})

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
message(STATUS "BOOST_HOME: " $ENV{BOOST_HOME})
message(STATUS "BOOST_ARCH: " ${BOOST_ARCH})

# find Boost. If not found we can't build c++ libs
option(Boost_DEBUG "Set to ON to debug Boost paths" OFF)

# this prevents the FindBoost.cmake to search for cmake-built Boost
set(Boost_NO_BOOST_CMAKE ON)

# if the Boost home environment var is set, use it to hint the FindBoost.cmake about Boost location
if(DEFINED ENV{BOOST_HOME})
  set(BOOST_ROOT $ENV{BOOST_HOME})
endif()
message(STATUS "BOOST_ROOT: " ${BOOST_ROOT})

if(BOOST_ARCH)
  ###  set(Boost_ARCHITECTURE "-x64")    ## Needed to work on GitHub CI test
  set(Boost_ARCHITECTURE ${BOOST_ARCH})
endif()

if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  find_package(Boost MODULE COMPONENTS system thread date_time regex chrono)
else()
  find_package(Boost MODULE COMPONENTS system thread)
endif()

message(STATUS "Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
message(STATUS "Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
message(STATUS "Boost_LIBRARIES: " ${Boost_LIBRARIES})

## ----------------------------------------------------------------------------
# Python, v2.7 or 3.x
if (${CMAKE_VERSION} VERSION_LESS "3.12")
  find_package(PythonInterp)
  if(PYTHONINTERP_FOUND)
    message(STATUS "Found Python interpreter version " ${PYTHON_VERSION_STRING})
    set(Python_EXECUTABLE ${PYTHON_EXECUTABLE})
  else()
    message(STATUS "No Python interpreter found. Python package will not be built.")
  endif()
else()
  find_package(Python COMPONENTS Interpreter Development)
  if(Python_FOUND)
    message(STATUS "Found Python interpreter version " ${Python_VERSION} " ID: " ${Python_INTERPRETER_ID})
  else()
    message(STATUS "No Python interpreter found. Python package will not be built.")
  endif()
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

message(STATUS "c Flags: " ${CMAKE_C_FLAGS})
message(STATUS "cxx Flags: " ${CMAKE_CXX_FLAGS})

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

## ----------------------------------------------------------------------------

if((OPS_BOOTSTRAP_BUILD) OR (DEFINED OPS_NO_SEPARATE_BOOTSTRAP))
  # use opsc script from build tree, will be used in all subdirs under this one
  if(WIN32)
    set(OPSC_COMPILER ${PROJECT_BINARY_DIR}/bin/opsc.bat)
  else()
    set(OPSC_COMPILER ${PROJECT_BINARY_DIR}/bin/opsc.sh)
  endif()
  if(hasParent)
    set(OPSC_COMPILER ${OPSC_COMPILER} PARENT_SCOPE)
  endif()

  set(OPSJAR_PATH ${PROJECT_BINARY_DIR}/bin)

  set(OPSJLIB_JAR ${OPSJAR_PATH}/OPSJLib-${PROJECT_VERSION}.jar)
  set(CONFIGURATIONLIB_JAR ${OPSJAR_PATH}/ConfigurationLib-${PROJECT_VERSION}.jar)

else()
  add_custom_target(opsc ALL COMMENT "Built during Bootstrap phase")
  # Use opsc script from Bootstrap phase, will be used in all subdirs under this one
  if(WIN32)
    find_file(OPSC_COMPILER NAMES opsc.bat PATHS ${CMAKE_INSTALL_PREFIX}/lib/ops NO_DEFAULT_PATH)
  else()
    find_file(OPSC_COMPILER NAMES opsc.sh PATHS ${CMAKE_INSTALL_PREFIX}/lib/ops NO_DEFAULT_PATH)
  endif()

  # Find Java libraries built during the Bootstrap phase
  set(OPSJAR_PATH ${CMAKE_INSTALL_PREFIX}/${OPS_JAR_REL_INSTALL_PATH})

  find_jar(OPSJLIB_JAR OPSJLib PATHS ${OPSJAR_PATH})
  find_jar(CONFIGURATIONLIB_JAR ConfigurationLib PATHS ${OPSJAR_PATH})
endif()
message(">>>> OPSC_COMPILER= " ${OPSC_COMPILER})
message(">>>> OPSJLIB_JAR= " ${OPSJLIB_JAR})
message(">>>> CONFIGURATIONLIB_JAR= " ${CONFIGURATIONLIB_JAR})

################################################################################
# Build the source

# Generate the source from the common idls into the source tree
add_subdirectory(Common)

# common parts
if((OPS_BOOTSTRAP_BUILD) OR (DEFINED OPS_NO_SEPARATE_BOOTSTRAP))
  # recurse into java source
  # We only build Java during the Bootstrap phase, since it is needed during
  # bootstrap and there isn't any difference between debug and opt builds
  if(Java_JAVAC_EXECUTABLE)
    # this builds ConfigurationLib and JarSearch
    add_subdirectory(Libs)
    # build OPSJLib
    add_subdirectory(Java)
  endif()

  # We only handle Python during the Bootstrap phase, since there isn't any
  # difference between debug and opt builds
  if(PYTHONINTERP_FOUND OR Python_FOUND)
    # Build python source distribution of ops
    add_subdirectory(Python)
  endif()
endif()

# add the C++ source, only if Boost is loaded
if(Boost_FOUND)
  add_subdirectory(Cpp)
else()
  message(WARNING "install Boost to build OPS for C++")
endif()

# Tools
add_subdirectory(Tools)

if(NOT OPS_BOOTSTRAP_BUILD)
  add_subdirectory(CSharp)

  ################################################################################
  # Build Unittests
  option(OPS_BUILD_UNITTESTS "Enable to build the UnitTest app(s)" ON)
  if(OPS_BUILD_UNITTESTS AND GTEST_FOUND AND CMAKE_BUILD_TYPE MATCHES "Debug")
    add_subdirectory(UnitTests)
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
