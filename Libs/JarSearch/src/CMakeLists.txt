
set(LIB_NAME JarSearch)

set(LIB_SRC
  jarsearch/JarSearcher.java
  jarsearch/util/FileHelper.java
)

add_jar(${LIB_NAME} ${LIB_SRC}
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
    WINDOWTITLE "JarSearch documentation"
    DOCTITLE "<h1>JarSearch documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
