
set(JARSEARCH_SRC
  jarsearch/JarSearcher.java
  jarsearch/util/FileHelper.java
)

add_jar(JarSearch ${JARSEARCH_SRC}
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

# in cmake-3.1 (at least) the install_jar is broken for Windows
if(UNIX)
  install_jar(JarSearch ${OPS_JAR_REL_INSTALL_PATH})
else()
  # install manually
  set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/JarSearch-${OPS_VERSION}.jar)
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME JarSearch.jar)
endif()

if(BUILD_DOCUMENTATION)
  create_javadoc(jarsearch
    PACKAGES jarsearch
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "JarSearch documentation"
    DOCTITLE "<h1>JarSearch documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
