if (OPS_BUILD_CPP)
  if(MSVC)
    add_compile_definitions(_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  endif()

  add_subdirectory(OPStest-C++)
endif()
