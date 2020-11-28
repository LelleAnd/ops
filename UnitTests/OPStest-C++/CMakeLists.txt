
##project (OPStest)

if(NOT GTEST_FOUND)
  message(FATAL_ERROR "GOOGLETEST_HOME is undefined")
endif()

if(NOT DEFINED ENV{GCOVR_HOME})
  message(WARNING "GCOVR_HOME is undefined")
endif()

# We are going to need the boost headers
include_directories(${Boost_INCLUDE_DIRS})
# The ops C++ headers are at:
include_directories(
  ${PROJECT_SOURCE_DIR}/Cpp/include
  ${PROJECT_SOURCE_DIR}/Common/idl/Generated/Cpp
)
# The Gtest headers are at:
include_directories(${GTEST_INCLUDE_DIRS})

# the basic tests
add_subdirectory(gtest)


# we have Pizza headers in lib
include_directories(lib)

# this is the publishBin application
add_subdirectory(src)

# This is the series of test applications
add_subdirectory(UnitTests)

#if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_custom_target(UnitTests DEPENDS
	publishBin
	test-subscribeTCP
	test-subscribeUDP
	test-subscribe
	test-serialize-and-deserialize
  gtest-main
	)
#endif()
