

add_executable(publishBin PublishData.cpp)

target_link_libraries(publishBin ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(publishBin ops-static)

if(MSVC)
  target_link_libraries(publishBin Winmm)
elseif(MINGW)
  target_link_libraries(publishBin ws2_32 mswsock)
else()
  target_link_libraries(publishBin gcov pthread rt)
endif()
