
set(BUILD_NAME EchotimeIdls)

# list of source files that, when changed, require regeneration of target
set(ECHO_SRCS
    src/echotime/EchotimeData.idl
    )

# the output file that we want
set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/Echotime.jar)

set(GENERATED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Generated)

add_custom_command(OUTPUT ${JAR_OUTPUT}
  COMMAND ${OPSC_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR} -g ALL -gpr ../../../../../Ada/ -s pyinit -s pypack
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GENERATED_PATH}/Java/Echotime.jar ${JAR_OUTPUT}
  DEPENDS ${ECHO_SRCS} opsc
  COMMENT "Generating ${JAR_OUTPUT} from ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(${BUILD_NAME} DEPENDS ${JAR_OUTPUT})

file(INSTALL ops_config.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/echotime)

set_target_properties(${BUILD_NAME} PROPERTIES GENERATED_PATH ${GENERATED_PATH})
