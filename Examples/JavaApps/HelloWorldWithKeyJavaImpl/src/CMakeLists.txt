
# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

set(APP_SRC
  helloworldjavaimpl/MainPublish.java
)

add_jar(MainPublishWithKey ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/HelloWorld.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT helloworldjavaimpl/MainPublish
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${PROJECT_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)
add_dependencies(MainPublishWithKey HelloWorld)

set(APP_SRC
  helloworldjavaimpl/MainSubscribe.java
)

add_jar(MainSubscribeWithKey ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/HelloWorld.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT helloworldjavaimpl/MainSubscribe
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${PROJECT_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)
add_dependencies(MainSubscribeWithKey HelloWorld)

install(FILES
  ${PROJECT_BINARY_DIR}/bin/MainPublishWithKey-${PROJECT_VERSION}.jar
  ${PROJECT_BINARY_DIR}/bin/MainSubscribeWithKey-${PROJECT_VERSION}.jar
  ${OPSJLIB_JAR}
  ${CONFIGURATIONLIB_JAR}
  ${PROJECT_BINARY_DIR}/bin/HelloWorld.jar
  DESTINATION examples/helloworld
)
