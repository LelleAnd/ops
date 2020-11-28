
# We only generate the source from the idls during the Bootstrap phase
# since it generates code into the source tree
if(CMAKE_BUILD_TYPE MATCHES "Bootstrap")
  # use opsc script from build tree, will be used in all subdirs under this one
  if(WIN32)
    set(OPS_COMPILER ${PROJECT_BINARY_DIR}/bin/opsc.bat)
  else()
    set(OPS_COMPILER ${PROJECT_BINARY_DIR}/bin/opsc.sh)
  endif()

  # this one generates the internal OPS IDLs
  add_subdirectory(idl)
endif()
