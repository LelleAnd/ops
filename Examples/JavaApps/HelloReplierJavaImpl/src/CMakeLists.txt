
set(APP_SRC
  helloreplierjavaimpl/Main.java
)

# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

add_jar(HelloReplierJavaImpl ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/HelloRequestReply.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT helloreplierjavaimpl/Main
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${PROJECT_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

add_dependencies(HelloReplierJavaImpl HelloRequestReply)

install(FILES
  ${PROJECT_BINARY_DIR}/bin/HelloReplierJavaImpl-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/OPSJLib-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/ConfigurationLib-${PROJECT_VERSION}.jar
  ${PROJECT_BINARY_DIR}/bin/HelloRequestReply.jar
  DESTINATION examples/requesthello
)
