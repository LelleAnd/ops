
set(BUILD_NAME opsidls)

# list of source files that, when changed, require regeneration of target
set(IDL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/OPSConstants.idl
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugRequestResponseData.idl
    )

# the output files that we want are a several, but we use one for the check
set(WANTED_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Generated/Cpp/opsidls/DebugRequestResponseData.h)

add_custom_command(OUTPUT ${WANTED_OUTPUT}
  COMMAND ${OPS_COMPILER} -g ALL -B ALL -gpr ../../../../Ada/ -o ${CMAKE_CURRENT_SOURCE_DIR}/Generated -p opsidls ${IDL_SRCS}
  DEPENDS ${IDL_SRCS} opsc
  COMMENT "Generating source files from IDL's in ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(${BUILD_NAME} ALL DEPENDS ${WANTED_OUTPUT})
add_dependencies(${BUILD_NAME} opsc)
