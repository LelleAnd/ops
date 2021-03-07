# Build Instructions #

## Prerequisites ##
See [Prerequisites](Prerequisites.md) for the required tools to build OPS.

## Checkout OPS ##
Use a Git client of your choice to clone the OPS repository to a directory of your choice, on windows e.g. "C:\\OPS" (OPS_DIR). You can then choose to checkout either trunk, a tag or a branch of your choice. Tags should be compilable and deployable right away. Trunk should normally build but is not sure to function properly. Refer to the latest commit comments to see if there are known issues or unfinished work in progress.

## Building with CMake ##
OPS can be built as a stand-alone project or be used as a sub-project to another CMake project.

In the stand-alone case the CMake install process will export an *OpsConfig.cmake* file with OPS targets, which can be used in the following way in a user *CMakeLists.txt* file:

```
set(Ops_DIR path_to_ops4/deploy/cmake)
find_package(Ops 4.2 REQUIRED)
message(STATUS "Found OPS version: ${Ops_VERSION}")
```

This will enable the use of OPS targets in a similar way as if OPS is added as a sub-project. The difference is that the OPS targets in the stand-alone case have an extra namespace, "OPS::".

#### Configuration ####
See the beginning of the top level [Makefile](../Makefile) and [CMakeLists.txt](../CMakeLists.txt) for some Environment and CMake symbols that can be set to affect the build process.

Building the examples from the *Examples* directory can optionally be turned off using the *cmake-gui* or by editing CMakeLists.txt. If make is used, building examples can also be controlled from the command line.

Building unit tests can also optionally be turned off in the same maner as building examples.

When using OPS as a sub-project, the options can be controlled in the following way:

```
OPTION(OPS_BUILD_UNITTESTS "" OFF)
OPTION(OPS_BUILD_EXAMPLES "" ON)
add_subdirectory(path_to_ops4 ops4)
```

#### Linux ####
In directory "OPS_DIR" type 'make'.

This will first perform a bootstrap step to generate some needed source code and then build both 'debug' and 'optimized' builds. The intermediate result is placed in subdirectories 'build.bootstrap', 'build.debug' and 'build.opt'.

#### Windows ####
Building with CMake on Windows currently requires Visual Studio.

Start a *MSBuild* command prompt, change to "OPS_DIR" and run the *run-cmake.bat* file for 32-bit binaries and *run-cmake-x64.bat* for 64-bit binaries.

This will first perform a bootstrap step to generate some needed source code and then build both 'debug' and 'optimized' builds. The intermediate result is by default placed in subdirectories 'build.bootstrap', 'build.debug' and 'build.opt' (for 64-bit builds with -x64 added to the subdirectory names). The subdirectories used can be changed by defining some symbols before running the bat-file.

#### CMake result ####
The install result will be in the subdirectory 'deploy' with the following structure:

> deploy
>  - bin
>  - cmake
>  - examples (building is optional in CMake)
>  - include
>  - lib
>  - share
>  - test

## Building without CMake ##
See [Building without CMake](BuildingWithoutCMake.md).

## Building OPS IDL Builder with Netbeans ##
Open from Netbeans "OPS_DIR\Tools\OPS IDL Builder NB", make sure you check Open Required Projects and Open as main project. Right click the main project and select "Build All". This will build several projects and output quite a lot of text in the output window. Make sure you scroll to the bottom of the output window and verify it says "BUILD SUCCESSFUL". Try the OPS IDL Builder by choosing "Run" on the main project (OPS IDL Builder NB).

Current OPS IDL Builder source has been used with Netbeans 8.0.
