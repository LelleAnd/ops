
# the Exe name
set(EXE_NAME OPSListener)

set(EXE_SRCS
  COpsConfigHelper.h
  FilterMessage.h
  MessageDump.cpp
  MessageDump.h
  OPSListener.cpp
  SdsSystemTime.cpp
  SdsSystemTime.h
)

# generate the library target
add_executable(${EXE_NAME}
    ${EXE_SRCS}
)

target_link_libraries(${EXE_NAME} ${Boost_LIBRARIES} ops-static tinyxml2::tinyxml2)

if(WIN32)
  target_link_libraries(${EXE_NAME} winmm)
  if(MINGW)
    target_link_libraries(${EXE_NAME} ws2_32 mswsock)
  endif()
else()
  target_link_libraries(${EXE_NAME} pthread rt)
endif()

# exe installation
set(INSTALL_BINDIR bin)

install(TARGETS ${EXE_NAME}
  RUNTIME DESTINATION ${INSTALL_BINDIR}
)
