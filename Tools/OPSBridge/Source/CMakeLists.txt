
# the Exe name
set(EXE_NAME OPSBridge)


set(EXE_SRCS
BridgeConfig.cpp
BridgeConfig.h
BridgeLog.h
BridgeMessages.h
BridgeTypeFactory.h
CBridge.cpp
CBridge.h
Configuration.h
CSocketTransport.cpp
CSocketTransport.h
CTcpTransportClient.cpp
CTcpTransportClient.h
CTcpTransportServer.cpp
CTcpTransportServer.h
CTransport.cpp
CTransport.h
CWorkerThread.cpp
CWorkerThread.h
OpsBridge.cpp
RawMcUdp.cpp
RawMcUdp.h
SEHException.cpp
SEHException.h
)

if(MSVC)
  # We need /EHa exception handling
  string(REGEX REPLACE "/EH[ascr]+" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(APPEND CMAKE_CXX_FLAGS " /EHa")
  message(STATUS ">>> " ${EXE_NAME} ": CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
  message(STATUS ">>> " ${EXE_NAME} ": CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
  message(STATUS ">>> " ${EXE_NAME} ": CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
endif()

# generate the library target
add_executable(${EXE_NAME}
    ${EXE_SRCS}
)
add_dependencies(${EXE_NAME} opsbridgeidls)

target_link_libraries(${EXE_NAME} ${Boost_LIBRARIES} ops-static)

if(WIN32)
  target_link_libraries(${EXE_NAME} winmm psapi)
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
