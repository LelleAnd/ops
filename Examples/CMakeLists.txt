
# Generate code for Example idl projects
add_subdirectory(OPSIdls)

# Define path to the OPSIDL directories that we need in subdirectories
set(OPSIDL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Examples/OPSIdls)

# Java example applications
add_subdirectory(JavaApps)

# CShrap examples
add_subdirectory(CSharpApps)

# C++ example applications
# All will need the ops headers
include_directories(
  ${PROJECT_SOURCE_DIR}/Cpp/include
  ${PROJECT_SOURCE_DIR}/Common/idl/Generated/Cpp
)

add_subdirectory(CppApps)
