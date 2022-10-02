
set(PUB_NAME HelloWorldWithKeyPub)
set(SUB_NAME HelloWorldWithKeySub)

set(COMMON_SRC
  ../ConfigFileHelper.cpp
  ../ConfigFileHelper.h
)

get_target_property(HELLOWORLD_GENERATED_PATH HelloWorld GENERATED_PATH)
include_directories(${HELLOWORLD_GENERATED_PATH}/Cpp)

#
add_executable(${PUB_NAME} HelloTopic_pub.cpp ${COMMON_SRC})
add_executable(${SUB_NAME} HelloTopic_sub.cpp ${COMMON_SRC})

# generate HelloWorld ops idl project if necessary
add_dependencies(${PUB_NAME} HelloWorld)
add_dependencies(${SUB_NAME} HelloWorld)

set_target_properties(${PUB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${SUB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_link_libraries(${PUB_NAME} ${Boost_LIBRARIES} ops-static tinyxml2::tinyxml2)
target_link_libraries(${SUB_NAME} ${Boost_LIBRARIES} ops-static tinyxml2::tinyxml2)

if(MSVC)

elseif(MINGW)
  target_link_libraries(${PUB_NAME} ws2_32 mswsock)
  target_link_libraries(${SUB_NAME} ws2_32 mswsock)
else()
  target_link_libraries(${PUB_NAME} pthread rt)
  target_link_libraries(${SUB_NAME} pthread rt)
endif()

install(TARGETS ${PUB_NAME} ${SUB_NAME} DESTINATION examples/helloworld)
