
# We need install_jar_exports() and VERSION_GREATER_EQUAL which came in 3.7.
cmake_minimum_required(VERSION 3.7.0 FATAL_ERROR)

# Adopt some policies for newer CMake's
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.15.0")
  ##message(">>>>> CMAKE_VERSION= " ${CMAKE_VERSION})
  # We don't want the default warning flags for VS builds
  cmake_policy(SET CMP0092 NEW)
  # We also want to be able to change the VS runtime
  cmake_policy(SET CMP0091 NEW)
endif()

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
#       TINYXML2_INSTALL_DIR        Used by CMake find_package()
#       (TINYXML2_INCLUDE_DIR       Used if no package found)
#       (TINYXML2_LIB               Used if no package found)
#
#     C++ compile flags:
#       CXXFLAGS                    Init value for CMAKE_CXX_FLAGS (C++ compile flags).
#                                   Used at configure time, then cached.
#
#     When C++ has defined USE_FIXED_LENGTH_STRINGS:
#       OPS_FIXED_OBJECT_NAME_SIZE  default: 50
#       OPS_FIXED_MESSAGE_KEY_SIZE  default: 60
#       OPS_FIXED_TYPE_ID_SIZE      default: 256
#       OPS_FIXED_CHANNEL_ID_SIZE   default: 20
#       OPS_FIXED_FILENAME_SIZE     default: 1024
#
#   CMAKE:
#     CMAKE_CXX_STANDARD            default: 11 ( 11 / 14 / 17 )
#     CMAKE_BUILD_TYPE              Bootstrap / Debug / Opt
#     BOOST_ARCH                    -
#     OPS_BUILD_CPP                 default: ON
#     OPS_BUILD_CSHARP              default: ON
#     OPS_BUILD_PYTHON              default: ON
#     OPS_BUILD_UNITTESTS           default: ON
#     OPS_BUILD_EXAMPLES            default: ON
#
#     When using OPS as a sub project, the following CMAKE vars is set in the parent scope:
#       OPSC_COMPILER               Full path to the opsc compiler (also set as an opsc target property)
#
################################################################################

if(NOT "${CMAKE_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS ">>> CMAKE_CXX_STANDARD:  " ${CMAKE_CXX_STANDARD})

option(OPS_BUILD_CPP "Enable to build C++ Core, apps and tools" ON)
option(OPS_BUILD_CSHARP "Enable to build C# Core and apps" ON)
option(OPS_BUILD_PYTHON "Enable to build Python package" ON)
option(OPS_BUILD_UNITTESTS "Enable to build the UnitTest app(s)" ON)
option(OPS_BUILD_EXAMPLES "Enable to build the example applications" ON)

message(STATUS ">>> CMAKE_BUILD_TYPE:    " ${CMAKE_BUILD_TYPE})
message(STATUS ">>> OPS_BUILD_CPP:       " ${OPS_BUILD_CPP})
message(STATUS ">>> OPS_BUILD_CSHARP:    " ${OPS_BUILD_CSHARP})
message(STATUS ">>> OPS_BUILD_PYTHON:    " ${OPS_BUILD_PYTHON})
message(STATUS ">>> OPS_BUILD_UNITTESTS: " ${OPS_BUILD_UNITTESTS})
message(STATUS ">>> OPS_BUILD_EXAMPLES:  " ${OPS_BUILD_EXAMPLES})

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
if (OPS_BUILD_UNITTESTS)
  message(STATUS "GOOGLETEST_HOME: " $ENV{GOOGLETEST_HOME})

  # help the FindGtest script locate gtest
  if(DEFINED ENV{GOOGLETEST_HOME})
    set(GTEST_ROOT $ENV{GOOGLETEST_HOME})
    list(APPEND CMAKE_PREFIX_PATH $ENV{GOOGLETEST_LIB})
  endif()
  find_package(GTest)

  message(STATUS "GTEST_INCLUDE_DIRS: " ${GTEST_INCLUDE_DIRS})
endif()

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

# On Windows the install_jar() is broken if a VERSION is given in add_jar()
# (at least upto CMake 3.18) so we conditionally set the version on install_jar()
if(WIN32)
  set(JAR_APPEND_VERSION "")
  set(JAR_APPEND_VER "")
else()
  set(JAR_APPEND_VERSION ${PROJECT_VERSION})
  set(JAR_APPEND_VER "-${PROJECT_VERSION}")
endif()

## ----------------------------------------------------------------------------
if (OPS_BUILD_CPP)
  message(STATUS "ENV: BOOST_HOME: " $ENV{BOOST_HOME})
  message(STATUS "BOOST_ARCH: " ${BOOST_ARCH})
  message(STATUS "ENV: BOOST_LIBRARY_DIR: " $ENV{BOOST_LIBRARY_DIR})
  message(STATUS "BOOST_LIBRARY_DIR: " ${BOOST_LIBRARY_DIR})

  # find Boost. If not found we can't build c++ libs
  option(Boost_DEBUG "Set to ON to debug Boost paths" OFF)

  #set(BOOST_LIB_DIAGNOSTIC ON)

  # this prevents the FindBoost.cmake to search for cmake-built Boost
  set(Boost_NO_BOOST_CMAKE ON)

  # if the Boost home environment var is set, use it to hint the FindBoost.cmake about Boost location
  if(DEFINED ENV{BOOST_HOME})
    set(BOOST_ROOT $ENV{BOOST_HOME})
  endif()
  message(STATUS "BOOST_ROOT: " ${BOOST_ROOT})

  if(BOOST_ARCH)
    set(Boost_ARCHITECTURE ${BOOST_ARCH})
  endif()

  if(MSVC)
    set(Boost_USE_STATIC_LIBS ON)
    ##set(Boost_USE_STATIC_RUNTIME ON)
    find_package(Boost MODULE COMPONENTS system thread regex)
  else()
    find_package(Boost MODULE COMPONENTS system thread)
  endif()

  ## Boost's -mt-s corresponds to VS' Runtime Library setting /MT
  ## Boost's -mt-sgd corresponds to VS' Runtime Library setting /MTd
  ## Boost's -mt corresponds to VS' Runtime Library setting /MD
  ## Boost's -mt-gd corresponds to VS' Runtime Library setting /MDd
  ## First and second can be built with ./b2 runtime-link=static threading=multi
  ## Third and fourth can be built with ./b2 runtime-link=shared threading=multi

  message(STATUS "Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
  message(STATUS "Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
  message(STATUS "Boost_LIBRARIES: " ${Boost_LIBRARIES})

  # TINYXML2_INSTALL_DIR        Used by CMake find_package()
  # (TINYXML2_INCLUDE_DIR       Used if no package found)
  # (TINYXML2_LIB               Used if no package found)

  if(TARGET tinyxml2::tinyxml2)
    # If added in a parent project, use it
    message(STATUS ">>>>>> Using tinyxml2::tinyxml2 target from parent project <<<<<<")

  else()
    find_package(tinyxml2 CONFIG COMPONENTS static PATHS ${TINYXML2_INSTALL_DIR})
    if (tinyxml2_FOUND)
      # if a package was found, use it
      message(STATUS ">>>>>> Using tinyxml2 target from found CMake package <<<<<<")
      message(STATUS "TINYXML2_INSTALL_DIR: " ${TINYXML2_INSTALL_DIR})

    else()
      # Set up our own imported target
      message(STATUS ">>>>>> DEFINING tinyxml2 as an IMPORTED target <<<<<<")
      message(STATUS "TINYXML2_INCLUDE_DIR: " ${TINYXML2_INCLUDE_DIR})
      message(STATUS "TINYXML2_LIB:         " ${TINYXML2_LIB})

      add_library(tinyxml2::tinyxml2 STATIC IMPORTED)
      set_target_properties(tinyxml2::tinyxml2 PROPERTIES
                     IMPORTED_LOCATION "${TINYXML2_LIB}"
                     IMPORTED_LOCATION_DEBUG "${TINYXML2_LIB}"
                     IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
                   )
      target_include_directories(tinyxml2::tinyxml2 INTERFACE ${TINYXML2_INCLUDE_DIR})
    endif()
  endif()

  ##include(../ops-build-tests/cmakedebug.cmake)
  ##print_target_properties(tinyxml2::tinyxml2)

endif ()

## ----------------------------------------------------------------------------
# Python, v2.7 or 3.x
if(OPS_BUILD_PYTHON)
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
endif()

## ----------------------------------------------------------------------------
# Update C++ compiler flags (warnings, exceptions, ...)
if (OPS_BUILD_CPP)
  message(STATUS ">>> CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
  message(STATUS ">>> CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
  message(STATUS ">>> CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})

  set(CMAKE_POSITION_INDEPENDENT_CODE ON)

  set(warnings "")
  if(MSVC)
    # Use at least warning level W3, User can specify higher level
    # CMake sets default value after user supplied value so it need to be removed
    # CMake 3.15 has removed the default values (Policy CMP0092)
    if(CMAKE_CXX_FLAGS MATCHES "/Wall")
      string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif()
    if(CMAKE_CXX_FLAGS MATCHES "/W4")
      string(REGEX REPLACE "/W[0-3]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif()
    if(NOT CMAKE_CXX_FLAGS MATCHES "/W([3-4]|all)")
      # Our default, if not specified from user/CMake
      string(APPEND warnings " /W3 /WX")
    endif()

    # Check and specify exception handling if not defined by user/CMake
    if(NOT CMAKE_CXX_FLAGS MATCHES "/EH[ascr]+")
      string(APPEND warnings " /EHsc")
    endif()

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
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

  message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
  message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
  message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
endif()

## ----------------------------------------------------------------------------
# Special for GCC on RasberryPI requiring extra link library
if (OPS_BUILD_CPP)
  message(STATUS ">>> CMAKE_COMPILER_IS_GNUCXX=    " ${CMAKE_COMPILER_IS_GNUCXX})
  message(STATUS ">>> CMAKE_HOST_SYSTEM_PROCESSOR= " ${CMAKE_HOST_SYSTEM_PROCESSOR})

  if(CMAKE_COMPILER_IS_GNUCXX)
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm")
      message(STATUS ">>> Adding -latomic to executable linking")
      set(CMAKE_EXE_LINKER_FLAGS "-latomic")
    endif()
  endif()
endif()

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

endif()
set(OPSJLIB_JAR ${OPSJAR_PATH}/OPSJLib${JAR_APPEND_VER}.jar)
set(CONFIGURATIONLIB_JAR ${OPSJAR_PATH}/ConfigurationLib${JAR_APPEND_VER}.jar)

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

  if(OPS_BUILD_PYTHON)
    # We only handle Python during the Bootstrap phase, since there isn't any
    # difference between debug and opt builds
    if(PYTHONINTERP_FOUND OR Python_FOUND)
      # Build python source distribution of ops
      add_subdirectory(Python)
    endif()
  endif()
endif()

# add the C++ source, only if Boost is loaded
if (OPS_BUILD_CPP)
  if(Boost_FOUND)
    add_subdirectory(Cpp)
  else()
    message(WARNING "install Boost to build OPS for C++")
  endif()
endif()

# Tools
add_subdirectory(Tools)

if(NOT OPS_BOOTSTRAP_BUILD)
  if(OPS_BUILD_CSHARP)
    add_subdirectory(CSharp)
  endif()

  ################################################################################
  # Build Unittests
  if(OPS_BUILD_UNITTESTS AND GTEST_FOUND AND CMAKE_BUILD_TYPE MATCHES "Debug")
    add_subdirectory(UnitTests)
  endif()

  ################################################################################
  # Build Examples
  # We need the opsc compiler from the Bootstrap phase to generate code for the examples
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

################################################################################
# Create cmake-files for use from other projects

set(OPS_NAMESPACE ${PROJECT_NAME}::)

if((OPS_BOOTSTRAP_BUILD) OR (DEFINED OPS_NO_SEPARATE_BOOTSTRAP))
  install_jar_exports(
    TARGETS OPSCompilerLib OPSJLib IDLParser ConfigurationLib IDLTemplates opsc
    NAMESPACE ${OPS_NAMESPACE}
    FILE jar-targets.cmake
    DESTINATION cmake/internal)
endif()
if(NOT OPS_BOOTSTRAP_BUILD)
  if(OPS_BUILD_CPP)
    install(EXPORT cpp-targets
      NAMESPACE ${OPS_NAMESPACE}
      DESTINATION cmake/internal
    )
  endif()
  if(OPS_BUILD_CSHARP AND WIN32)
    install(EXPORT csharp-targets
      NAMESPACE ${OPS_NAMESPACE}
      DESTINATION cmake/internal
    )
  endif()
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(OpsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/OpsConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake
  PATH_VARS OPSC_COMPILER
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/OpsConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/OpsConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/OpsConfigVersion.cmake
  DESTINATION cmake
)
