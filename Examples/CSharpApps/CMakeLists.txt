  if(MINGW)
    message(WARNING "No support for CSharp when building with MinGW toolchain yet")
    return()
  endif()

  find_program(CSCOMPILER NAMES csc.exe csc
    HINTS
      ENV PATH
    PATH_SUFFIXES bin
  )

  message(">>>> C# Compiler= " ${CSCOMPILER} )

  if (CSCOMPILER)
    # We need C# language version 7.3 or later which for now only is supported by Visual Studio 2017 version 15.7 and later
    # (mono version 6.12.0.90 still has only support for language version upto 7.2)
    # Installing .NET on Linux gives access to the dotnet tool and a csc compiler supporting at least version 8 of C#

    execute_process(COMMAND ${CSCOMPILER} -langversion:? OUTPUT_VARIABLE csc_supported_versions ERROR_VARIABLE csc_errors)
    ## message(">>>>>>>> " ${csc_supported_versions} " <<<<<<<<")

    set(csc_support_found -1)
    string(FIND "${csc_supported_versions}" "7.3" csc_support_found)
    if (${csc_support_found} LESS 0)
      message(WARNING "CSharp compiler supporting required language version (7.3) not found...")
    else()
      # add the examples
      add_subdirectory(OPSTest)
      add_subdirectory(TestAll)
    endif()
  else()
    message(WARNING "NO CSharp compiler found...")
  endif()
