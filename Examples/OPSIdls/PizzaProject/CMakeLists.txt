
set(BUILD_NAME PizzaProject)

# list of source files that, when changed, require regeneration of target
set(PIZZA_SRCS
    opsproject/project.properties
    src/pizza/CapricosaData.idl
    src/pizza/PizzaData.idl
    src/pizza/VessuvioData.idl
    src/pizza/special/Cheese.idl
    src/pizza/special/ExtraAllt.idl
    src/pizza/special/LHCData.idl
    src/pizza/special/Parmesan.idl
    src/pizza/extra/Secret.idl
    )

# the output file that we want
set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/PizzaProject.jar)

set(GENERATED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Generated)

add_custom_command(OUTPUT ${JAR_OUTPUT}
  COMMAND ${OPSC_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR} -g ALL -gpr ../../../../../Ada/ -s pyinit -s pypack
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GENERATED_PATH}/Java/PizzaProject.jar ${JAR_OUTPUT}
  DEPENDS ${PIZZA_SRCS} opsc
  COMMENT "Generating ${JAR_OUTPUT} from ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(${BUILD_NAME} DEPENDS ${JAR_OUTPUT})

file(INSTALL ops_config.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/pizzatest)

set_target_properties(${BUILD_NAME} PROPERTIES GENERATED_PATH ${GENERATED_PATH})
