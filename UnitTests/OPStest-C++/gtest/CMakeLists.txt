
set(APP_NAME gtest-main)
set(APP2_NAME gtest-mempool)
set(APP3_NAME gtest-mempool3)

set(APP_SRCS
  ByteBuffer_test.cpp
  ChecksumArchiver_test.cpp
  CreateTempOpsConfigFile.cpp
  CreateTempOpsConfigFile.h
  ErrorServiceWriter_test.cpp
  Filter_test.cpp
  fixed_string_test.cpp
  gtest_main.cpp
  listener_helpers.h
  Lockable_test.cpp
  MemoryMap_test.cpp
  MocSendReceiveDataHelpers.cpp
  MocSendReceiveDataHelpers.h
  NotifierListener_test.cpp
  ObjectFactory_test.cpp
  OpsConfigObjects_test.cpp
  OpsConfigRepo_test.cpp
  OpsEvent_test.cpp
  OpsMessage_test.cpp
  OpsObject_test.cpp
  OPSUtilities_test.cpp
  ParticipantInfo_test.cpp
  PubIdChecker_test.cpp
  Publisher_test.cpp
  PubSub_Test.cpp
  RAII_ioServ.h
  Reservable_test.cpp
  SerDesObjects.h
  Serialization_test.cpp
  SetupOPSConfig.cpp
  SetupOPSConfig.h
  Subscriber_test.cpp
  TCPOpsProtocol_test.cpp
  ThreadPool_test.cpp
)

set(APP2_SRCS
  gtest_main.cpp
  memory_pool_nd_test.cpp
)

set(APP3_SRCS
  gtest_main.cpp
  memory_pool_test.cpp
)

# We need also the internal OPS C++ headers from:
include_directories(
  ${PROJECT_SOURCE_DIR}/Cpp/source
)

add_executable(${APP_NAME} ${APP_SRCS})
add_executable(${APP2_NAME} ${APP2_SRCS})
add_executable(${APP3_NAME} ${APP3_SRCS})

target_link_libraries(${APP_NAME} ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ops-static)
target_link_libraries(${APP2_NAME} ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ops-static)
target_link_libraries(${APP3_NAME} ${Boost_LIBRARIES} ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ops-static)

if(MSVC)
  target_link_libraries(${APP_NAME} Winmm)
  target_link_libraries(${APP2_NAME} Winmm)
  target_link_libraries(${APP3_NAME} Winmm)
elseif(MINGW)
  target_link_libraries(${APP_NAME} ws2_32 mswsock)
  target_link_libraries(${APP2_NAME} ws2_32 mswsock)
  target_link_libraries(${APP3_NAME} ws2_32 mswsock)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  target_link_libraries(${APP_NAME} gcov pthread rt)
  target_link_libraries(${APP2_NAME} gcov pthread rt)
  target_link_libraries(${APP3_NAME} gcov pthread rt)
endif()

install(TARGETS ${APP_NAME} DESTINATION test)
install(TARGETS ${APP2_NAME} DESTINATION test)
install(TARGETS ${APP3_NAME} DESTINATION test)
