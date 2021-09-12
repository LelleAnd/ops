
set(BUILD_NAME DerivedIDLs)

# list of source files that, when changed, require regeneration of target
set(CURRENT_SRCS
    opsproject/project.properties
    src/DerivedIDLs/FooData.idl
    )

# the output file that we want
set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/DerivedIDLs.jar)

set(GENERATED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Generated)

set(BASEIDLS_GENERATED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../BaseIDLs/Generated)
set(BASEIDLS_IDL_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../src/)

# -ref ${BASEIDLS_IDL_SOURCE_PATH}
# -pypack ${BASEIDLS_GENERATED_PATH}/Python/BaseIDLs.opsc

add_custom_command(OUTPUT ${JAR_OUTPUT}
  COMMAND ${OPSC_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR} -g ALL -gpr ../../../../../../Ada/ -s pyinit -s pypack
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GENERATED_PATH}/Java/DerivedIDLs.jar ${JAR_OUTPUT}
  DEPENDS ${CURRENT_SRCS} opsc
  COMMENT "Generating ${JAR_OUTPUT} from ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(${BUILD_NAME} ALL DEPENDS ${JAR_OUTPUT})
add_dependencies(${BUILD_NAME} BaseIDLs)

set_target_properties(${BUILD_NAME} PROPERTIES GENERATED_PATH ${GENERATED_PATH})
