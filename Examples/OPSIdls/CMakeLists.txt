
if(DEFINED OPS_NO_SEPARATE_BOOTSTRAP)
  set(OPSC_TARGET opsc)
endif()

# this one generates all example IDLs
add_subdirectory(ChatExample)
add_subdirectory(HelloRequestReply)
add_subdirectory(HelloWorld)
add_subdirectory(MultiLibs/BaseIDLs)
add_subdirectory(MultiLibs/DerivedIDLs)
add_subdirectory(PizzaProject)
add_subdirectory(TestAll)
add_subdirectory(WeatherStationExample)
