  # We only handle Python during the Bootstrap phase, since there isn't any
  # difference between debug and opt builds
  # Build python source distribution of ops

  set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/__init__.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Archiver.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Constants.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/DataAssembly.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Factory.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/OPS_Archiver.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/opsTypes.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Participant.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/ParticipantInfoData.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Print_Archiver.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Publisher.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/ReceiveDataHandler.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/SendDataHandler.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Subscriber.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/Support.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ops/XML_Archiver.py
  )

  if(WIN32)
    set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/ops-python-${PROJECT_VERSION}.zip)
  else()
    set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/ops-python-${PROJECT_VERSION}.tar.gz)
  endif()

  configure_file(setup.py.in ${CMAKE_CURRENT_SOURCE_DIR}/setup.py)

  add_custom_command(OUTPUT ${OUTPUT}
                     COMMAND ${PYTHON_EXECUTABLE} setup.py sdist -d ${CMAKE_CURRENT_BINARY_DIR}/dist
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     DEPENDS ${SRC} setup.py.in)

  add_custom_target(target ALL DEPENDS ${OUTPUT})

  if(NOT DEFINED OPS_PYTHON_INSTALL_PATH)
    install(FILES ${OUTPUT} DESTINATION lib)
  else()
    install(FILES ${SRC} DESTINATION ${OPS_PYTHON_INSTALL_PATH}/ops)
  endif()
