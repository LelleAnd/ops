
add_subdirectory(src)

if(WIN32)
  # put bat script in build directory so we can invoke it directly without installing
  configure_file(opsc.bat.in ${PROJECT_BINARY_DIR}/bin/opsc.bat)

  # install a generic opsc.bat that will run opsc.jar
  install(FILES opsc.bat DESTINATION ${OPS_JAR_REL_INSTALL_PATH})

  # install a specialized opsc.bat that will run opsc-version.jar
  install(FILES ${PROJECT_BINARY_DIR}/bin/opsc.bat
    DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME opsc-${OPS_VERSION}.bat
    )
else()
  # put opsc script in build dir so we can invoke it directly without installing
  ##configure_file(opsc.sh ${PROJECT_BINARY_DIR}/bin/opsc.sh COPYONLY)
  file(COPY opsc.sh DESTINATION ${PROJECT_BINARY_DIR}/bin
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )

  # install-time installation of opsc.sh script
  install(FILES opsc.sh DESTINATION ${OPS_JAR_REL_INSTALL_PATH}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
endif()
