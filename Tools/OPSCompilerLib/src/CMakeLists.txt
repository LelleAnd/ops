
set(LIB_NAME OPSCompilerLib)

set(LIB_SRC
  parsing/AbstractTemplateBasedIDLCompiler.java
  parsing/IDLClass.java
  parsing/IDLCompiler.java
  parsing/IDLEnumType.java
  parsing/IDLField.java
  parsing/TopicInfo.java
  parsing/Topic.java
  gui/IDLCompilerSettingsPlugin.java
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
  create_javadoc(${LIB_NAME}
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
