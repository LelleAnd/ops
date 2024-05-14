
set(PUB_NAME RequestHelloPub)
set(SUB_NAME RequestHelloSub)

set(COMMON_SRC
  ../ConfigFileHelper.cpp
  ../ConfigFileHelper.h
)

get_target_property(HELLO_GENERATED_PATH HelloRequestReply GENERATED_PATH)
include_directories(${HELLO_GENERATED_PATH}/Cpp)

#
add_executable(${PUB_NAME} RequestHelloTopic_pub.cpp ${COMMON_SRC})
add_executable(${SUB_NAME} RequestHelloTopic_sub.cpp ${COMMON_SRC})

# generate HelloRequestReply ops idl project if necessary
add_dependencies(${PUB_NAME} HelloRequestReply)
add_dependencies(${SUB_NAME} HelloRequestReply)

set_target_properties(${PUB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${SUB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_link_libraries(${PUB_NAME} ${Boost_LIBRARIES} ops-static)
target_link_libraries(${SUB_NAME} ${Boost_LIBRARIES} ops-static)

if(MSVC)

elseif(MINGW)
  target_link_libraries(${PUB_NAME} ws2_32 mswsock)
  target_link_libraries(${SUB_NAME} ws2_32 mswsock)
else()
  target_link_libraries(${PUB_NAME} pthread rt)
  target_link_libraries(${SUB_NAME} pthread rt)
endif()

install(TARGETS ${PUB_NAME} ${SUB_NAME} DESTINATION examples/requesthello)
