
set(APP_NAME EchotimeCpp)

get_target_property(ECHOTIME_GENERATED_PATH EchotimeIdls GENERATED_PATH)
include_directories(${ECHOTIME_GENERATED_PATH}/Cpp)

set(APP_SRCS
  Echotime.cpp
  ../ConfigFileHelper.cpp
  ../ConfigFileHelper.h
)

add_executable(${APP_NAME} ${APP_SRCS})

# generate Echotime ops idl project if necessary
add_dependencies(${APP_NAME} EchotimeIdls)

set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX "-dbg")
#set_target_properties(${LIB_NAME} PROPERTIES RELEASE_POSTFIX "-opt")

target_link_libraries(${APP_NAME} ${Boost_LIBRARIES} ops-static)

# under visual studio the time functions uses function in Winmm.lib
if(MSVC)
  target_link_libraries(${APP_NAME} Winmm)
elseif(MINGW)
  target_link_libraries(${APP_NAME} ws2_32 mswsock)
else()
  target_link_libraries(${APP_NAME} pthread rt)
endif()

install(TARGETS ${APP_NAME} DESTINATION examples/echotime)
