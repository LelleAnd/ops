# Prerequisites to Build OPS #

## Git ##
Make sure you have a git client installed.

## Java ##
Make sure you have a Java SE Development Kit (JDK, version 8 or higher) installed on
your system. The tools you use to create systems with OPS are all running in Java and
use the Java compiler to generate pre-compiled code to use for debugging. If you know
you will only use OPS for C++ and C# and do not need any debugging capabilities, it may
be sufficient to only have a Java runtime environment installed. It is strongly
recommended to install a JDK though.

If you do not have a JDK installed on your system follow these steps:
  1. Download the latest JDK from oracle.com, (Tip! Select a bundle with Netbeans and you are ready to open the OPS sample Netbeans Java projects!)       http://www.oracle.com/technetwork/java/index.html
  1. Install it, you will need administrator rights on your system.
  1. When installed, add the newly created directory
 > on Windows e.g. "C:\Program Files\Java\jdk1.8.0_05\bin"

 to your user environment variable called path, you can do this without being an administrator of your system.

## Boost (required for OPS C++ Core) ##
Make sure you have boost installed, otherwise download pre-built binaries for boost or build boost from source http://www.boost.org/users/download/.

  * Select at least the following components:
    * Boost header files
    * Boost DateTime
    * Boost Filesystem
    * Boost Regex
    * Boost System
    * Boost Thread

OPS has been used with Boost versions 1.60, 1.64, 1.66, 1.67, 1.71, 1.72, 1.74, 1.82, 1.83 and 1.84.
Versions older than 1.71 hasn't been used/tested for quite a while. It is recommended to use a more modern version.

## TinyXML-2 (required for OPS C++ Core) ##
TinyXML-2 can be downloaded from https://github.com/leethomason/tinyxml2.

OPS has been used with TinyXML-2 version 9.0.0.

## CMake ##
Version 3.10.0 or later is required to build OPS with CMake. Make sure the cmake command is in your path. CMake can be downloaded at https://cmake.org/.

OPS has been used with CMake version 3.10.0 and later (upto 3.31.1).

## Visual Studio (Windows) ##
OPS for C++ requires a C++11 compiler and has therefore only support for VS2015, VS2017, VS2019 and VS2022. Support for earlier VS versions has been dropped.

OPS for C# requires language version 7.3 or later (due to generics and constraining type parameters to System.Enum), i.e. Visual Studio 2017 version 15.7 or later.

## gcc (Linux) ##
OPS for C++ requires a C++11 compiler and has therefore only support for gcc 4.8.1 an later.

OPS has been used with 5.3.0, 5.4.0, 6.2.1, 6.4.1, 7.3.1, 9.2.0, 9.3.0, 11.3.0, 13.3 and 14.2.
Versions older than 9.2.0 hasn't been used/tested for quite a while. It is recommended to use a more modern version.

## .NET ##
To use C# on Linux, install .NET 5 or later as decribed on
https://docs.microsoft.com/en-us/dotnet/core/install/.

## Google Test (required for unit tests) ##
Google Test can be downloaded at https://github.com/google/googletest.

## JavaCC (required for rebuilding the IDL parser) ##
The Java Compiler Compiler is used to generate the IDL parser for the OPS IDL language.
JavaCC can be downloaded from http://java.net/projects/javacc.

OPS uses JavaCC version 6.0.

## Python ##
Support exist for Python 2.7 and Python 3.

## Delphi ##
OPS has been used with Delphi XE2, Delphi 10.1, 10.2, 10.3, 10.4, 11.3 and 12.1.
Versions older than 10.1 hasn't been used/tested for quite a while. It is recommended to use a more modern version.

## Ada ##
The Ada implementation requires Ada 2012 support.
OPS has been used with GNAT GPS 2016, GNAT Community 2020 and GNAT Pro 17.1.

Please note that *GNAT Community 2019 (20190517)* isn't supported. The compiler crashes with a GNAT BUG DETECTED. This has not been investigated further.

## gcovr ##
The tool can be downloaded at https://gcovr.com/en/stable/.
If command found, called to generate coverage during C++ unittests (on Linux).

## cppcheck ##
The tool can be downloaded at https://github.com/danmar/cppcheck.
If command found, called to perform some static analysis on the C++ code during C++ unittests (on Linux).
