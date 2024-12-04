
set(LIB_NAME opsc)

set(LIB_SRC
  # opsc
  opsc/OpsCompiler.java
  opsc/Compiler.java
  opsc/CompilerSupport.java
  opsc/CSharpCompiler.java
  opsc/CppCompiler.java
  opsc/DelphiCompiler.java
  opsc/AdaCompiler.java
  opsc/JavaCompiler.java
  opsc/JSONCompiler.java
  opsc/IDLParser.java
  opsc/ProjectProperties.java
  opsc/Property.java
  opsc/PythonCompiler.java
  opsc/JarDependency.java
  opsc/DebugProjectCompiler.java
  opsc/VisualStudio2008CppExampleCompiler.java
)

# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

add_jar(${LIB_NAME} ${LIB_SRC}
  INCLUDE_JARS OPSCompilerLib OPSJLib IDLParser ConfigurationLib IDLTemplates
  ENTRY_POINT opsc/OpsCompiler
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
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
    PACKAGES ${LIB_NAME}
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "ops idl compiler documentation"
    DOCTITLE "<h1>ops idl compiler documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
