
# We only build Java during the Bootstrap phase, since it is needed during
# bootstrap and there isn't any difference between debug and opt builds
if(CMAKE_BUILD_TYPE MATCHES "Bootstrap")
  add_subdirectory(ConfigurationLib/src)
  add_subdirectory(JarSearch/src)
endif()
