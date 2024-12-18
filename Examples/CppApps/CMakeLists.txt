
add_subdirectory(Example1)
add_subdirectory(Echotime)
add_subdirectory(GenericPub)

add_subdirectory(HelloRequesterCppImpl)

add_subdirectory(HelloWorldCppImpl)
add_subdirectory(HelloWorldWithKeyCppImpl)

add_subdirectory(PizzaTest)

add_subdirectory(TestAllC++Test/TestAllC++Test)

if(GTEST_FOUND)
  add_subdirectory(UnitTestExample)
endif()
