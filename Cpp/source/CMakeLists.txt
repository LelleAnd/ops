
# the library name
set(LIB_NAME ops)

set(OPS4_CPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Include OPS4 core source files
include(${CMAKE_CURRENT_SOURCE_DIR}/ops4-core-source-list.cmake)

# Source files implementing the lowest layer, interfacing network, etc.
# These will need to change/re-implement if boost isn't available for the platform
set(LIB_BOTTOM_LAYER_SRC
  BoostDeadlineTimerImpl.h
  BoostIOServiceImpl.h
  DeadlineTimer.cpp
  IOService.cpp
  MulticastReceiver.cpp
  MulticastReceiver.h
  NetworkSupport.cpp
  Receiver.cpp
  Sender.cpp
  TCPBoostConnection.h
  TCPClient.cpp
  TCPClient.h
  TCPServer.h
  ThreadSupport.cpp
  TimeHelper.cpp
  UDPReceiver.cpp
  UDPReceiver.h
  UDPSender.cpp
  UDPSender.h
)

# ide grouping, doesn't matter for makefile projects
source_group("Header Files" FILES ${LIB_PUBLIC_HEADERS})

# Get path to the generated common OPS idls
get_target_property(OPSIDLS_GENERATED_PATH opsidls GENERATED_PATH)

if((OPS_BOOTSTRAP_BUILD) OR (DEFINED OPS_NO_SEPARATE_BOOTSTRAP))
  # install headers separately to keep the include file hierarchy
  # Only done in Bootstrap phase
  set(HEADER_INSTALL_DIR include/${LIB_NAME})
  install(DIRECTORY ${OPS_HEADER_PATH}/ DESTINATION ${HEADER_INSTALL_DIR})
  install(DIRECTORY ${OPSIDLS_GENERATED_PATH}/Cpp/ DESTINATION ${HEADER_INSTALL_DIR})
else()
  set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/${LIB_NAME})
endif()

if(NOT OPS_BOOTSTRAP_BUILD)
  # generate the library target
  add_library(${LIB_NAME} SHARED
    ${LIB_OPS_CORE_SRCS}
    ${LIB_BOTTOM_LAYER_SRC}
    ${LIB_OPS_PUBLIC_HEADERS}
  )

  add_dependencies(${LIB_NAME} opsidls)

  if (DEFINED OPS_NO_SEPARATE_BOOTSTRAP)
    target_include_directories(${LIB_NAME} PUBLIC ${OPS4_CPP_ROOT}/include)
    target_include_directories(${LIB_NAME} PUBLIC ${OPSIDLS_GENERATED_PATH}/Cpp)
  else()
    target_include_directories(${LIB_NAME} PUBLIC ${HEADER_INSTALL_DIR})
  endif()

  set_target_properties(${LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
  # adds -DOPS_LIBRARY to commandline. Can't use add_definitions here
  set_target_properties(${LIB_NAME} PROPERTIES COMPILE_DEFINITIONS OPS_LIBRARY)
  set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX "-dbg")
  set_target_properties(${LIB_NAME} PROPERTIES RELEASE_POSTFIX "-opt")

  target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES})

  if(MINGW)
    target_link_libraries(${LIB_NAME} ws2_32 mswsock)
  endif()

  # build it again but statically this time
  set(LIB_STATIC ${LIB_NAME}-static)
  # generate the library target
  add_library(${LIB_STATIC} STATIC
    ${LIB_OPS_CORE_SRCS}
    ${LIB_BOTTOM_LAYER_SRC}
    ${LIB_OPS_PUBLIC_HEADERS}
  )

  add_dependencies(${LIB_STATIC} opsidls)

  if (DEFINED OPS_NO_SEPARATE_BOOTSTRAP)
    target_include_directories(${LIB_STATIC} PUBLIC ${OPS4_CPP_ROOT}/include)
    target_include_directories(${LIB_STATIC} PUBLIC ${OPSIDLS_GENERATED_PATH}/Cpp)
  else()
    target_include_directories(${LIB_STATIC} PUBLIC ${HEADER_INSTALL_DIR})
  endif()

  set_target_properties(${LIB_STATIC} PROPERTIES VERSION ${PROJECT_VERSION})
  set_target_properties(${LIB_STATIC} PROPERTIES COMPILE_DEFINITIONS OPS_LIBRARY_STATIC)
  set_target_properties(${LIB_STATIC} PROPERTIES DEBUG_POSTFIX "-dbg")
  set_target_properties(${LIB_STATIC} PROPERTIES RELEASE_POSTFIX "-opt")

  target_link_libraries(${LIB_STATIC} ${Boost_LIBRARIES})

  # library installation
  set(INSTALL_BINDIR bin)
  if(WIN32 AND NOT CYGWIN)
    set(INSTALL_LIBDIR bin)
    set(INSTALL_ARCHIVEDIR lib)
  else()
    set(INSTALL_LIBDIR lib${LIB_POSTFIX})
    set(INSTALL_ARCHIVEDIR lib${LIB_POSTFIX})
  endif()

  install(TARGETS ${LIB_NAME} ${LIB_STATIC}
    EXPORT ops-targets
    RUNTIME DESTINATION ${INSTALL_BINDIR}
    LIBRARY DESTINATION ${INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${INSTALL_ARCHIVEDIR}
  )
endif()
