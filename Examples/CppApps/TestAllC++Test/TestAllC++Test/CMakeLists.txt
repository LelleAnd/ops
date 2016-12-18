
set(APP1_NAME TestAllPub)
set(APP2_NAME TestAllSub)
set(APP3_NAME TestAllSubBase)
set(APP4_NAME VerifySerDes)

include_directories(${OPSIDL_SOURCE_DIR}/TestAll/Generated/Cpp)

add_executable(${APP1_NAME} TestAll_Publish/TestAll_Publish.cpp)
add_executable(${APP2_NAME} TestAll_Subscribe/TestAll_Subscribe.cpp)
add_executable(${APP3_NAME} TestAll_SubscribeToBase/TestAll_SubscribeToBase.cpp)
add_executable(${APP4_NAME} VerifySerDes/VerifySerDes.cpp)

# generate TestAll ops idl project if necessary
add_dependencies(${APP1_NAME} TestAll)
add_dependencies(${APP2_NAME} TestAll)
add_dependencies(${APP3_NAME} TestAll)
add_dependencies(${APP4_NAME} TestAll)

set_target_properties(${APP1_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${APP2_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${APP3_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${APP4_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_link_libraries(${APP1_NAME} ${Boost_LIBRARIES} ops-static)
target_link_libraries(${APP2_NAME} ${Boost_LIBRARIES} ops-static)
target_link_libraries(${APP3_NAME} ${Boost_LIBRARIES} ops-static)
target_link_libraries(${APP4_NAME} ${Boost_LIBRARIES} ops-static)

if(MSVC)
  # under visual studio the time functions uses function in Winmm.lib
  target_link_libraries(${APP1_NAME} Winmm)
  target_link_libraries(${APP2_NAME} Winmm)
  target_link_libraries(${APP3_NAME} Winmm)
  target_link_libraries(${APP4_NAME} Winmm)
elseif(MINGW)
  target_link_libraries(${APP1_NAME} ws2_32 mswsock)
  target_link_libraries(${APP2_NAME} ws2_32 mswsock)
  target_link_libraries(${APP3_NAME} ws2_32 mswsock)
  target_link_libraries(${APP4_NAME} ws2_32 mswsock)
else()
  target_link_libraries(${APP1_NAME} pthread)
  target_link_libraries(${APP2_NAME} pthread)
  target_link_libraries(${APP3_NAME} pthread)
  target_link_libraries(${APP4_NAME} pthread)
endif()

install(TARGETS ${APP1_NAME} ${APP2_NAME} ${APP3_NAME} ${APP4_NAME} DESTINATION examples/testall)

file(INSTALL ops_config.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/testall)