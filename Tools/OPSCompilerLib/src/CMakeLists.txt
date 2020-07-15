
set(OPSCOMPILERLIB_SRC
  parsing/AbstractTemplateBasedIDLCompiler.java
  parsing/IDLClass.java
  parsing/IDLCompiler.java
  parsing/IDLEnumType.java
  parsing/IDLField.java
  parsing/TopicInfo.java
  parsing/Topic.java
  gui/IDLCompilerSettingsPlugin.java
)

add_jar(OPSCompilerLib ${OPSCOMPILERLIB_SRC}
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

# in cmake-3.1 (at least) the install_jar is broken for Windows
if(UNIX)
  install_jar(OPSCompilerLib ${OPS_JAR_REL_INSTALL_PATH})
else()
  # install manually
  set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/OPSCompilerLib-${OPS_VERSION}.jar)
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME OPSCompilerLib.jar)
endif()

if(BUILD_DOCUMENTATION)
  create_javadoc(OPSCompilerLib
    PACKAGES gui parsing
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "OPSCompilerLib documentation"
    DOCTITLE "<h1>OPSCompilerLib documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
