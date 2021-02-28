
set(LIB_NAME IDLParser)

set(LIB_SRC
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

add_jar(${LIB_NAME} ${LIB_SRC}
  INCLUDE_JARS OPSCompilerLib OPSJLib
  VERSION ${JAR_APPEND_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

install_jar(${LIB_NAME} ${OPS_JAR_REL_INSTALL_PATH})
if(WIN32)
  # Due to broken install_jar() on windows, install jar with version manually
  install(FILES ${PROJECT_BINARY_DIR}/bin/${LIB_NAME}.jar DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME ${LIB_NAME}-${OPS_VERSION}.jar)
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
