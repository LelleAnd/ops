
set(CONFIGLIB_SRC
  configlib/AddElementCallback.java
  configlib/ArchiverIn.java
  configlib/ArchiverInOut.java
  configlib/ArchiverOut.java
  configlib/Bus.java
  configlib/ConfigUtil.java
  configlib/Deserializable.java
  configlib/Driver.java
  configlib/Passanger.java
  configlib/Person.java
  configlib/SerializableCompositeFactory.java
  configlib/SerializableFactory.java
  configlib/Serializable.java
  configlib/SerializableOld.java
  configlib/TestClass1.java
  configlib/TestClass2.java
  configlib/TestConfigClass.java
  configlib/TestInout.java
  configlib/XMLArchiverIn.java
  configlib/XMLArchiverOut.java
  configlib/XMLConfigLoader.java
  configlib/exception/FormatException.java
)

add_jar(ConfigurationLib ${CONFIGLIB_SRC}
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

# in cmake-3.1 (at least) the install_jar is broken for Windows
if(UNIX)
  install_jar(ConfigurationLib ${OPS_JAR_REL_INSTALL_PATH})
else()
  # install manually
  set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/ConfigurationLib-${OPS_VERSION}.jar)
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME ConfigurationLib.jar)
endif()


if(BUILD_DOCUMENTATION)
  create_javadoc(configlib
    PACKAGES configlib configlib.exception
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "ConfigurationLib documentation"
    DOCTITLE "<h1>ConfigurationLib documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
