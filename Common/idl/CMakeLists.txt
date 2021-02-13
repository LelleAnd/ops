
set(BUILD_NAME opsidls)

# list of source files that, when changed, require regeneration of target
set(IDL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/OPSConstants.idl
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugRequestResponseData.idl
    ${CMAKE_CURRENT_SOURCE_DIR}/SendAckPatternData.idl
    )

# the output files that we want are a several, but we use one for the check
set(GENERATED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Generated)
set(WANTED_OUTPUT ${GENERATED_PATH}/Cpp/opsidls/DebugRequestResponseData.h)

if((OPS_BOOTSTRAP_BUILD) OR (DEFINED OPS_NO_SEPARATE_BOOTSTRAP))
  # We only generate the source from the idls during the Bootstrap phase
  # since it generates code into the source tree
  add_custom_command(OUTPUT ${WANTED_OUTPUT}
    COMMAND ${OPSC_COMPILER} -g ALL -B ALL -gpr ../../../../Ada/ -o ${GENERATED_PATH} -p opsidls ${IDL_SRCS}
    DEPENDS ${IDL_SRCS} opsc
    COMMENT "Generating source files from IDL's in ${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
    )
endif()

add_custom_target(${BUILD_NAME} ALL DEPENDS ${WANTED_OUTPUT})
add_dependencies(${BUILD_NAME} opsc)

set_target_properties(${BUILD_NAME} PROPERTIES GENERATED_PATH ${GENERATED_PATH})
