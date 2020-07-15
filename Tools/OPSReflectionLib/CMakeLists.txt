
# reflection depends on JarSearch
set(CMAKE_JAVA_INCLUDE_PATH ${CMAKE_JAVA_INCLUDE_PATH} ${PROJECT_BINARY_DIR}/Libs/JarSearch/src/JarSearch.jar)

add_subdirectory(src)
