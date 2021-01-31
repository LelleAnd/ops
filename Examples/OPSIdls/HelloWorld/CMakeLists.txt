
# list of source files that, when changed, require regeneration of target
set(HELLO_SRCS
    opsproject/project.properties
    src/hello/HelloData.idl
    )

# the output file that we want
set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/HelloWorld.jar)

add_custom_command(OUTPUT ${JAR_OUTPUT}
  COMMAND ${OPSC_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR} -g ALL -gpr ../../../../../Ada/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Generated/Java/HelloWorld.jar ${JAR_OUTPUT}
  DEPENDS ${HELLO_SRCS} ${OPSC_TARGET}
  COMMENT "Generating ${JAR_OUTPUT} from ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(HelloWorld DEPENDS ${JAR_OUTPUT})

file(INSTALL ops_config.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/helloworld)
