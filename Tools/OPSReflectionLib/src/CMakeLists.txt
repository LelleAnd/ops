
set(OPSREFLECTIONLIB_SRC
  opsreflection/JarPathConfig.java
  opsreflection/OPSFactoryConfig.java
  opsreflection/OPSFactory.java
  opsreflection/OPSReflectionSerializableFactory.java
)

add_jar(OPSReflectionLib ${OPSREFLECTIONLIB_SRC}
  INCLUDE_JARS ConfigurationLib JarSearch OPSCompilerLib OPSJLib
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)


# in cmake-3.1 (at least) the install_jar is broken for Windows
if(UNIX)
  install_jar(OPSReflectionLib ${OPS_JAR_REL_INSTALL_PATH})
else()
  # install manually
  set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/OPSReflectionLib-${OPS_VERSION}.jar)
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME OPSReflectionLib.jar)
endif()

if(BUILD_DOCUMENTATION)
  create_javadoc(OPSReflectionLib
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
