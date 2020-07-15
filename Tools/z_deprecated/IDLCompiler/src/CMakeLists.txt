
set(IDLCOMPILER_SRC
  # package Exceptions
  Exceptions/MissingInputException.java
  Exceptions/TopicConfigFileMissingException.java
  # package idlcompiler
  idlcompiler/compilers/Compiler.java
  idlcompiler/compilers/CppCompiler.java
  idlcompiler/compilers/CppFactoryIDLCompiler.java
  idlcompiler/compilers/CSCompiler.java
  idlcompiler/compilers/JavaCompiler.java
  idlcompiler/compilers/NewJavaCompiler.java
  idlcompiler/compilers/plugins/PluginsLoaderException.java
  idlcompiler/compilers/plugins/PluginsLoader.java
  idlcompiler/files/IDLFile.java
  idlcompiler/files/ProjectFileGenerator.java
  idlcompiler/files/ProjectFileParser.java
  idlcompiler/files/Project.java
  idlcompiler/files/TextFile.java
  idlcompiler/files/TopicConfigFile.java
  idlcompiler/topicconfigcreators/CppTopicConfigCreator.java
  idlcompiler/topicconfigcreators/CSTopicConfigCreator.java
  idlcompiler/topicconfigcreators/JavaTopicConfigCreator.java
  idlcompiler/topicconfigcreators/RequestReplyInfo.java
  idlcompiler/topicconfigcreators/XMLConfigLoader.java
  idlcompiler/gui/CreateNewIDLPanel.java
  idlcompiler/gui/CreateProjectPanel.java
  idlcompiler/gui/FileTab.java
  idlcompiler/gui/GUIHelper.java
  idlcompiler/gui/GUI.java
  idlcompiler/gui/JarFileFilter.java
  idlcompiler/gui/OptionsTool.java
  idlcompiler/gui/ProjectFileFilter.java
  idlcompiler/gui/ProjectListComponent.java
  idlcompiler/gui/TopicConfigRow.java
  idlcompiler/gui/TopicConfigTool.java
  #idlcompiler/gui/CreateNewIDLPanel.form
  #idlcompiler/gui/CreateProjectPanel.form
  #idlcompiler/gui/FileTab.form
  #idlcompiler/gui/GUI.form
  #idlcompiler/gui/OptionsTool.form
  #idlcompiler/gui/TopicConfigRow.form
  #idlcompiler/gui/TopicConfigTool.form
  # package parsing and javaccparser
  parsing/FileParser.java
  parsing/IDLFileParser.java
  parsing/JAXMeIDLFileParser.java
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
  # package util
  util/FileHelper.java
)


add_jar(IDLCompiler ${IDLCOMPILER_SRC}
  INCLUDE_JARS OPSCompilerLib OPSJLib
  ENTRY_POINT idlcompiler/gui/GUI
  MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/MANIFEST.MF
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

# in cmake-3.1 (at least) the install_jar is broken for Windows
if(UNIX)
  install_jar(IDLCompiler ${OPS_JAR_REL_INSTALL_PATH})
else()
  # install manually
  set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/IDLCompiler-${OPS_VERSION}.jar)
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME IDLCompiler.jar)
endif()

if(BUILD_DOCUMENTATION)
  create_javadoc(IDLCompiler
    PACKAGES Exception idlcompiler.compilers idlcompiler.files parsing parsing.javaccparser util
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "IDLCompiler documentation"
    DOCTITLE "<h1>IDLCompiler documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
