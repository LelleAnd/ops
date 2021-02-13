
# Generate code for Example idl projects
add_subdirectory(OPSIdls)

# Define path to the OPSIDL directories that we need in subdirectories
set(OPSIDL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Examples/OPSIdls)

# Java example applications
add_subdirectory(JavaApps)

# CSharp examples
add_subdirectory(CSharpApps)

# C++ example applications
add_subdirectory(CppApps)
