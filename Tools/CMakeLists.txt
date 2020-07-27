  add_subdirectory(OPSCompilerLib)
  add_subdirectory(IDLParser)
  add_subdirectory(OPSReflectionLib)
  add_subdirectory(opsc)
  #add_subdirectory(OPSListener)
  add_subdirectory(VerifyOPSConfig)
  #add_subdirectory(DebugConsole)
  add_subdirectory(OPSBridge)

# Install the latest template files
  set(TMPL_PATH NBOPSIDLSupport/src/ops/netbeansmodules/idlsupport/templates)
  set(OPS_TEMPLATE_FILES
    ${TMPL_PATH}/adabodyenumtemplate.tpl
    ${TMPL_PATH}/adabodypublishertemplate.tpl
    ${TMPL_PATH}/adabodysubscribertemplate.tpl
    ${TMPL_PATH}/adabodytemplate.tpl
    ${TMPL_PATH}/adabodytypefactorytemplate.tpl
    ${TMPL_PATH}/adaprojectfiletemplate.tpl
    ${TMPL_PATH}/adaspecenumtemplate.tpl
    ${TMPL_PATH}/adaspecpublishertemplate.tpl
    ${TMPL_PATH}/adaspecsubscribertemplate.tpl
    ${TMPL_PATH}/adaspectemplate.tpl
    ${TMPL_PATH}/adaspectemplatebare.tpl
    ${TMPL_PATH}/adaspectypefactorytemplate.tpl

    ${TMPL_PATH}/cppenumtemplate.tpl
    ${TMPL_PATH}/cpppublishertemplate.tpl
    ${TMPL_PATH}/cppsubscribertemplate.tpl
    ${TMPL_PATH}/cpptemplate.tpl
    ${TMPL_PATH}/cpptemplatebare.tpl
    ${TMPL_PATH}/cpptypefactorytemplate.tpl

    ${TMPL_PATH}/csenumtemplate.tpl
    ${TMPL_PATH}/cspublishertemplate.tpl
    ${TMPL_PATH}/cssubscribertemplate.tpl
    ${TMPL_PATH}/cstemplate.tpl
    ${TMPL_PATH}/cstemplatebare.tpl
    ${TMPL_PATH}/cstypefactorytemplate.tpl

    ${TMPL_PATH}/debugproj.tpl

    ${TMPL_PATH}/delphienumtemplate.tpl
    ${TMPL_PATH}/delphitemplate.tpl
    ${TMPL_PATH}/delphitemplatebare.tpl
    ${TMPL_PATH}/delphitypefactorytemplate.tpl

    ${TMPL_PATH}/javaconfig.tpl
    ${TMPL_PATH}/javaenumtemplate.tpl
    ${TMPL_PATH}/javapublishertemplate.tpl
    ${TMPL_PATH}/javasubscribertemplate.tpl
    ${TMPL_PATH}/javatemplate.tpl
    ${TMPL_PATH}/javatemplatebare.tpl
    ${TMPL_PATH}/javatypefactorytemplate.tpl

    ${TMPL_PATH}/pythonclasstemplate.tpl
    ${TMPL_PATH}/pythonclasstemplatebare.tpl
    ${TMPL_PATH}/pythonenumtemplate.tpl
    ${TMPL_PATH}/pythonpackagetemplate.tpl
    ${TMPL_PATH}/pythontypefactorytemplate.tpl

    ${TMPL_PATH}/vs_pub_cpp.tpl
    ${TMPL_PATH}/vs_pub_proj.tpl
    ${TMPL_PATH}/vs_sln.tpl
    ${TMPL_PATH}/vs_sub_cpp.tpl
    ${TMPL_PATH}/vs_sub_proj.tpl
  )

  source_group("Template Files" FILES ${OPS_TEMPLATE_FILES})

  # install template files in share
  install(FILES ${OPS_TEMPLATE_FILES}
    DESTINATION share/ops/templates
  )

  add_jar(IDLTemplates ${OPS_TEMPLATE_FILES}
    VERSION ${OPS_VERSION}
    OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin
  )

  # in cmake-3.1 (at least) the install_jar is broken for Windows
  if(UNIX)
    install_jar(IDLTemplates ${OPS_JAR_REL_INSTALL_PATH})
  else()
    # install manually
    set(JAR_OUTPUT ${PROJECT_BINARY_DIR}/bin/IDLTemplates-${OPS_VERSION}.jar)
    install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
    install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME IDLTemplates.jar)
  endif()
