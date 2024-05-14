
set(APP_NAME PizzaTestCpp)

get_target_property(PIZZAPROJECT_GENERATED_PATH PizzaProject GENERATED_PATH)
include_directories(${PIZZAPROJECT_GENERATED_PATH}/Cpp)

set(APP_SRCS
  PizzaTest.cpp
  SdsSystemTime.cpp
  SdsSystemTime.h
  ../ConfigFileHelper.cpp
  ../ConfigFileHelper.h
  targetver.h
)

add_executable(${APP_NAME} ${APP_SRCS})

# generate PizzaProject ops idl project if necessary
add_dependencies(${APP_NAME} PizzaProject)

set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_link_libraries(${APP_NAME} ${Boost_LIBRARIES} ops-static)

# under visual studio the time functions uses function in Winmm.lib
if(MSVC)
  target_link_libraries(${APP_NAME} Winmm)
elseif(MINGW)
  target_link_libraries(${APP_NAME} ws2_32 mswsock)
else()
  target_link_libraries(${APP_NAME} pthread rt)
endif()

install(TARGETS ${APP_NAME} DESTINATION examples/pizzatest)
