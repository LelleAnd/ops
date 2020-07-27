  if(MINGW)
    message(STATUS "No support for CSharp when building with MinGW toolchain yet")
    return()
  endif()

  if(CMAKE_CL_64)
    find_program(CSCOMPILER NAMES csc.exe gmcs mcs
      HINTS
        ENV PATH
  #      $ENV{WINDIR}/Microsoft.NET/Framework64/v4.0.30319
  #      /sw/tools/mono/2.6.1
      PATH_SUFFIXES bin
      )
  else()
    find_program(CSCOMPILER NAMES csc.exe gmcs mcs
      HINTS
        ENV PATH
  #      $ENV{WINDIR}/Microsoft.NET/Framework/v4.0.30319
  #      /sw/tools/mono/2.6.1
      PATH_SUFFIXES bin
      )
  endif()

  # We need C# language version 7.3 or later which for now only is supported by Visual Studio 2017 version 15.7 and later
  # (mono version 5.20.1.19 has only support for language version upto 7.2)
  if(CSCOMPILER AND WIN32)
    # add the source
    add_subdirectory(Ops)
  else()
    message(STATUS "CSharp compiler not found...")
  endif()
