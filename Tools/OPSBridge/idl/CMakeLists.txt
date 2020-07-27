
set(BUILD_NAME opsbridgeidls)

# list of source files that, when changed, require regeneration of target
set(IDL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/OpsBridgeCommandData.idl
    ${CMAKE_CURRENT_SOURCE_DIR}/OpsBridgeStatusData.idl
    ${CMAKE_CURRENT_SOURCE_DIR}/TestBlockData.idl
    )

# the output files that we want are several, but we use one for the check
set(WANTED_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Generated/Cpp/opsbridge/OpsBridgeCommandData.h)

add_custom_command(OUTPUT ${WANTED_OUTPUT}
  COMMAND ${OPSC_COMPILER} -g ALL -B ALL -o ${CMAKE_CURRENT_SOURCE_DIR}/Generated -p opsbridge ${IDL_SRCS}
  DEPENDS ${IDL_SRCS} opsc
  COMMENT "Generating source files from IDL's in ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(${BUILD_NAME} DEPENDS ${WANTED_OUTPUT})
