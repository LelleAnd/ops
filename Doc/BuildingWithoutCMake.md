# Building without CMake #
## Build opsc (command-line OPS IDL compiler) and OPS Core for Java on Windows ##
Run the bat-file 'OPS_DIR\Script\Build_Idl_Compiler.bat'. This will compile the necessary Java libraries, including *OPS Core for Java*, and the opsc compiler.

The result will be in "OPS_DIR\Tools\opsc\dist".

The opsc compiler can then be used like:
> OPS_DIR\Tools\opsc\opsc.bat -h

## Perform the necessary bootstrap step on Windows ##
Run the bat-file 'OPS_DIR\Common\idl\build.bat'. This will generate the needed source code using the opsc compiler built above.

## Build OPS Core for C++ with Visual Studio ##
Open the solution/project for your version of Visual Studio in directory

> "OPS\_DIR\Cpp\Visual C++\OPSCrossPlatformCppSolution_20xx\OPSCrossPlatformCppLib_20xx.sln"

Please note that since OPS version 4.2.0 (~feb 2019), support has been dropped for VS versions earlier than VS2015 (due to C++11 requirements).

Check that the include and library references to the Boost library corresponds with your installation (see Visual Studio project properties).

Build the solution for all configurations (both x86 and x64):

  * "Debug" - for compiler setting Multi-threaded Debug
  * "Debug DLL" - for compiler setting Multi-threaded Debug DLL
  * "Release" - for compiler setting Multi-threaded Release
  * "Release DLL" - for compiler setting Multi-threaded Release DLL

Verify they all compile alright (you will have a few warnings).

## Building OPS Core for C# with Visual Studio ##
Open the solution/project with Visual Studio (VS2017 version 15.7 or later) in directory

> "OPS_DIR\CSharp\Projects\OpsLibrary\OpsLibrary.sln"

Build the solution for all configurations:

  * "Debug" - for compiler setting Debug
  * "Release" - for compiler setting Release

## Building Examples ##
See [Building Examples](BuildingExamples.md).
