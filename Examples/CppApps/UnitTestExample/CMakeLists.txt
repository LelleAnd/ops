
set(APP_NAME UnitTestExample)

get_target_property(TESTALL_GENERATED_PATH TestAll GENERATED_PATH)
include_directories(${TESTALL_GENERATED_PATH}/Cpp)
# The Gtest headers are at:
include_directories(${GTEST_INCLUDE_DIRS})

set(APP_SRCS
  UnitTestExample.cpp
  ClassUnderTest.h
  ClassUnderTest_Test.cpp
)

add_executable(${APP_NAME} ${APP_SRCS})

# generate TestAll ops idl project if necessary
add_dependencies(${APP_NAME} TestAll)

set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX "-dbg")
#set_target_properties(${LIB_NAME} PROPERTIES RELEASE_POSTFIX "-opt")

target_link_libraries(${APP_NAME} ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ops-static)

# under visual studio the time functions uses function in Winmm.lib
if(MSVC)
  target_link_libraries(${APP_NAME} Winmm)
elseif(MINGW)
  target_link_libraries(${APP_NAME} ws2_32 mswsock)
else()
  target_link_libraries(${APP_NAME} pthread rt)
endif()

install(TARGETS ${APP_NAME} DESTINATION examples/unittest)
install(FILES ops_config.xml DESTINATION examples/unittest)
