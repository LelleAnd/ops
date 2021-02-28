
set(LIB_NAME ConfigurationLib)

set(LIB_SRC
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

add_jar(${LIB_NAME} ${LIB_SRC}
  VERSION ${JAR_APPEND_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

install_jar(${LIB_NAME} ${OPS_JAR_REL_INSTALL_PATH})
if(WIN32)
  # Due to broken install_jar() on windows, install jar with version manually
  install(FILES ${PROJECT_BINARY_DIR}/bin/${LIB_NAME}.jar DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME ${LIB_NAME}-${OPS_VERSION}.jar)
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
