
set(LIB_NAME JarSearch)

set(LIB_SRC
  jarsearch/JarSearcher.java
  jarsearch/util/FileHelper.java
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
