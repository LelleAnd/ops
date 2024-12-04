
if((OPS_BOOTSTRAP_BUILD) OR (DEFINED OPS_NO_SEPARATE_BOOTSTRAP))

  # We only build Java during the Bootstrap phase, since it is needed during
  # bootstrap and there isn't any difference between debug and opt builds
  if(Java_JAVAC_EXECUTABLE)
    add_subdirectory(OPSCompilerLib)
    add_subdirectory(IDLParser)
    add_subdirectory(OPSReflectionLib)
    add_subdirectory(opsc)
  endif()
endif()

if(NOT OPS_BOOTSTRAP_BUILD)
  if (OPS_BUILD_CPP)
    # Tools written in C++ is compiled for both debug and opt builds
    add_subdirectory(OPSListener)
    add_subdirectory(VerifyOPSConfig)
    add_subdirectory(DebugConsole)
    add_subdirectory(OPSBridge)
  endif()
endif()

# Install the latest template files
# Only during the Bootstrap phase, since it is needed during
# bootstrap and there isn't any difference between debug and opt builds
if((OPS_BOOTSTRAP_BUILD) OR (DEFINED OPS_NO_SEPARATE_BOOTSTRAP))

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

  set(LIB_NAME IDLTemplates)

  add_jar(${LIB_NAME} ${OPS_TEMPLATE_FILES}
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
endif()
