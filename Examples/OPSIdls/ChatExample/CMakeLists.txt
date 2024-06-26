
set(BUILD_NAME ChatExample)

# list of source files that, when changed, require regeneration of target
set(CURRENT_SRCS
    opsproject/project.properties
    src/chat/ChatData.idl
    src/chat/ExtendedChatData.idl
    src/chat/ExtendedUserData.idl
    src/chat/MessageData.idl
    src/chat/UserData.idl
    )

# the output file that we want
set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/ChatExample.jar)

set(GENERATED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Generated)

add_custom_command(OUTPUT ${JAR_OUTPUT}
  COMMAND ${OPSC_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR} -g ALL -gpr ../../../../../Ada/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GENERATED_PATH}/Java/ChatExample.jar ${JAR_OUTPUT}
  DEPENDS ${CURRENT_SRCS} opsc
  COMMENT "Generating ${JAR_OUTPUT} from ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(${BUILD_NAME} ALL DEPENDS ${JAR_OUTPUT})

file(INSTALL ops_config.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/chatexample)

set_target_properties(${BUILD_NAME} PROPERTIES GENERATED_PATH ${GENERATED_PATH})
