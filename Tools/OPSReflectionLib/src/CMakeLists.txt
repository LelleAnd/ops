
set(LIB_NAME OPSReflectionLib)

set(LIB_SRC
  opsreflection/JarPathConfig.java
  opsreflection/OPSFactoryConfig.java
  opsreflection/OPSFactory.java
  opsreflection/OPSReflectionSerializableFactory.java
)

add_jar(${LIB_NAME} ${LIB_SRC}
  INCLUDE_JARS ConfigurationLib JarSearch OPSCompilerLib OPSJLib
  VERSION ${JAR_ADD_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

if(WIN32)
  # Due to broken install_jar() on windows, install jar with version manually
  install(FILES ${PROJECT_BINARY_DIR}/bin/${LIB_NAME}${JAR_APPEND_VER}.jar DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${PROJECT_BINARY_DIR}/bin/${LIB_NAME}${JAR_APPEND_VER}.jar DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME ${LIB_NAME}.jar)
else()
  install_jar(${LIB_NAME} ${OPS_JAR_REL_INSTALL_PATH})
endif()

if(BUILD_DOCUMENTATION)
  create_javadoc(${LIB_NAME}
    PACKAGES opsreflection
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "OPSReflectionLib documentation"
    DOCTITLE "<h1>OPSReflectionLib documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
