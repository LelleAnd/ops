
# list of source files that, when changed, require regeneration of target
set(CURRENT_SRCS
    opsproject/project.properties
    src/weatherstation/BasicWeatherData.idl
    src/weatherstation/BasicWindData.idl
    src/weatherstation/ExtendedWeatherData.idl
    src/weatherstation/ExtendedWindData.idl
    )

# the output file that we want
set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/WeatherStationExample.jar)

add_custom_command(OUTPUT ${JAR_OUTPUT}
  COMMAND ${OPSC_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR} -g ALL -gpr ../../../../../Ada/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Generated/Java/WeatherStationExample.jar ${JAR_OUTPUT}
  DEPENDS ${CURRENT_SRCS} opsc
  COMMENT "Generating ${JAR_OUTPUT} from ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(WeatherStationExample ALL DEPENDS ${JAR_OUTPUT})
