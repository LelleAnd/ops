
set(APP_SRC
  opstestapp/COpsHelper.java
  opstestapp/ILogListener.java
  opstestapp/IOpsHelper.java
  opstestapp/IOpsHelperListener.java
  consoleapp/opstestmain.java
  consoleapp/PizzaTest.java
)

# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

add_jar(PizzaTest ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/PizzaProject.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT PizzaTest
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${JAR_APPEND_VERSION}
  OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
)

add_dependencies(PizzaTest PizzaProject)

install(FILES
  ${PROJECT_BINARY_DIR}/bin/PizzaTest${JAR_APPEND_VER}.jar
  ${OPSJLIB_JAR}
  ${CONFIGURATIONLIB_JAR}
  ${PROJECT_BINARY_DIR}/bin/PizzaProject.jar
  DESTINATION examples/pizzatest
)
