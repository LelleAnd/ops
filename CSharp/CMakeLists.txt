
if(NOT CMAKE_BUILD_TYPE MATCHES "Bootstrap")

  if(MINGW)
    message(WARNING "No support for CSharp when building with MinGW toolchain yet")
    return()
  endif()

  if(CMAKE_CL_64)
    find_program(CSCOMPILER NAMES csc.exe mcs
      HINTS
        ENV PATH
  #      $ENV{WINDIR}/Microsoft.NET/Framework64/v4.0.30319
  #      /sw/tools/mono/2.6.1
      PATH_SUFFIXES bin
      )
  else()
    find_program(CSCOMPILER NAMES csc.exe mcs
      HINTS
        ENV PATH
  #      $ENV{WINDIR}/Microsoft.NET/Framework/v4.0.30319
  #      /sw/tools/mono/2.6.1
      PATH_SUFFIXES bin
      )
  endif()

  message(">>>> C# Compiler= " ${CSCOMPILER} )

  if (CSCOMPILER)
    # We need C# language version 7.3 or later which for now only is supported by Visual Studio 2017 version 15.7 and later
    # (mono version 6.12.0.90 still has only support for language version upto 7.2)

    execute_process(COMMAND ${CSCOMPILER} -langversion:? OUTPUT_VARIABLE csc_supported_versions ERROR_VARIABLE csc_errors)
    string(FIND ${csc_supported_versions} "7.3" csc_support_found)
    if (${csc_support_found} LESS 0)
      message(WARNING "CSharp compiler supporting required language version (7.3) not found...")
    else()
      # add the source
      add_subdirectory(Ops)
    endif()
  else()
    message(WARNING "NO CSharp compiler found...")
  endif()

endif()
