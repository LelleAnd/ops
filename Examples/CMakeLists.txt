
# Generate code for Example idl projects
add_subdirectory(OPSIdls)

# Java example applications
add_subdirectory(JavaApps)

if(OPS_BUILD_CSHARP)
  # CSharp examples
  add_subdirectory(CSharpApps)
endif()

if (OPS_BUILD_CPP)
  # C++ example applications
  add_subdirectory(CppApps)
endif()
