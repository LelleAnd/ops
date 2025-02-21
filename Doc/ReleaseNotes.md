# Release Notes #
The major changes for each release tag is noted below. For minor changes see the git log.

## tags 4.3.x-y ##
Requires the tinyxml2 library which 4.2.x-y doesn't.

### 4.3.1-0 ###
* Changed default C++ standard to **C++17**. The support for some older standards still exist but will in a near future be dropped.

### 4.3.0-5 ###
* Added **'in-process' transport for Ada**. See [Defining Topics and Configuration](OpsConfig.md).
* Added Ada test program UnitTestExample.

### 4.3.0-4 ###
* Added possibility to generate **'validation' code** from compile directives in the idl-files. See [message validation](MessageValidation.md).

### 4.3.0-3 ###
* Improved support for Delphi 10.x and older.

### 4.3.0-2 ###
* Minor fixes for jar-file installs on windows.
* Added C++ test program UnitTestExample.

### 4.3.0-1 ###
* Changed to require atleast **CMake version 3.10**.

### 4.3.0-0 ###
* Changed code to use the **tinyxml2 library** instead of an internal parser.


---
## tags 4.2.x-y ##
Last version that doesn't require the tinyxml2 library.

### 4.2.0-7 ###
* Added **'in-process' transport for C++**. See [Defining Topics and Configuration](OpsConfig.md).

### 4.2.0-6 ###
* Added TCP many-2-many transport to C# and Java.

### 4.2.0-5 ###
* Added TCP many-2-many transport to Python.
* Added possibility to use hostnames instead of IP-addresses.
