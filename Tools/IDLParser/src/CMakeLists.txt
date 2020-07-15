
set(IDLPARSER_SRC
  parsing/FileParser.java
  parsing/IDLFileParser.java
  parsing/ParseException.java
  parsing/ProjectParser.java
  parsing/TypeSorter.java
  parsing/javaccparser/FileParser.java
  parsing/javaccparser/IDLParserConstants.java
  parsing/javaccparser/IDLParser.java
  parsing/javaccparser/IDLParserTokenManager.java
  parsing/javaccparser/ParseException.java
  parsing/javaccparser/ParserEventCallback.java
  parsing/javaccparser/ParserEvent.java
  parsing/javaccparser/SimpleCharStream.java
  parsing/javaccparser/Token.java
  parsing/javaccparser/TokenMgrError.java
)

add_jar(IDLParser ${IDLPARSER_SRC}
  INCLUDE_JARS OPSCompilerLib OPSJLib
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

# in cmake-3.1 (at least) the install_jar is broken for Windows
if(UNIX)
  install_jar(IDLParser ${OPS_JAR_REL_INSTALL_PATH})
else()
  # install manually
  set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/IDLParser-${OPS_VERSION}.jar)
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME IDLParser.jar)
endif()

if(BUILD_DOCUMENTATION)
  create_javadoc(javadoc_IDLParser
    PACKAGES parsing parsing.javaccparser
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "IDLParser documentation"
    DOCTITLE "<h1>IDLParser documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
