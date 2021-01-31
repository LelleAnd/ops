
set(APP_SRC
  testalljavatest/Main.java
)

# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

add_jar(TestAllPublisherJavaImpl ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/TestAll.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT testalljavatest/Main
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${PROJECT_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

add_dependencies(TestAllPublisherJavaImpl TestAll)

install(FILES
  ${PROJECT_BINARY_DIR}/bin/TestAllPublisherJavaImpl-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/OPSJLib-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/ConfigurationLib-${PROJECT_VERSION}.jar
  ${PROJECT_BINARY_DIR}/bin/TestAll.jar
  DESTINATION examples/testall
)
