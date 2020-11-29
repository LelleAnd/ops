
# We are going to need the OPS headers
include_directories(
  ${PROJECT_SOURCE_DIR}/Cpp/include
  ${PROJECT_SOURCE_DIR}/Common/idl/Generated/Cpp
)

# We are going to need the boost headers
include_directories(${Boost_INCLUDE_DIRS})

# with msvc and mingw we need to set a platform...
if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0501)

  # and get rid of some bogus warnings with visual studio
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  endif()
endif()

################################################################################
# Build the source

# add the source
add_subdirectory(Source)
