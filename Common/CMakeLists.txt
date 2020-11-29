
  # use opsc script from build tree, will be used in all subdirs under this one
  if(WIN32)
    set(OPS_COMPILER ${PROJECT_BINARY_DIR}/bin/opsc.bat)
  else()
    set(OPS_COMPILER ${PROJECT_BINARY_DIR}/bin/opsc.sh)
  endif()

  # this one generates the internal OPS IDLs
  add_subdirectory(idl)
