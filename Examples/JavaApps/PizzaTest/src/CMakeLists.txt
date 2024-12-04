
set(APP_SRC
  pizzatestimpl/Main.java
)

# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

add_jar(PizzaTestImpl ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/PizzaProject.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT pizzatestimpl/Main
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${JAR_ADD_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

add_dependencies(PizzaTestImpl PizzaProject)

install(FILES
  ${PROJECT_BINARY_DIR}/bin/PizzaTestImpl${JAR_APPEND_VER}.jar
  ${OPSJLIB_JAR}
  ${CONFIGURATIONLIB_JAR}
  ${PROJECT_BINARY_DIR}/bin/PizzaProject.jar
  DESTINATION examples/pizzatest
)
