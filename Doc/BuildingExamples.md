[Examples](SimpleCpp.md) [BuildInstructions](BuildInstructions.md)

---

# Building Examples #
## Build Example IDLs ##
Under OPS_DIR/Examples, you find example IDL's in the directory **OPSIdls**.

To build these you need to first have built the ***opsc*** command-line compiler, either using CMake, see [Build Instructions](BuildInstructions.md), or without CMake, see [here](BuildingWithoutCMake.md).

To build the '**TestAll**' IDL example do the following:

On Windows do
> \> path-to-opsc\opsc.bat -P OPSIdls\TestAll

On Linux do
> $ path-to-opsc/opsc.sh -P OPSIdls/TestAll

This creates a 'Generated' directory in the TestAll directory with all generated files.
For more information about compiling IDL's see [opsc](IDLCommandlineCompiler.md).

For Windows you can also build all IDL examples at once using the bat-file *OPS_DIR/Examples/Build_Idls.bat*.

## Build C++ Examples ##
Under OPS_DIR/Examples, you find C++ examples in the directory **CppApps**.

An example using the above TestAll IDLs exist in the directory **TestAllC++Test**.

Open **TestAllC++Test.sln** in directory **TestAllC++Test**, check the boost reference to the include and library directories so they point to your boost installation and then build and run to see that your OPS installation works correctly for C++.

## Python Examples ##
Under OPS_DIR/Examples, you find Python examples in the directory **PythonApps**.

To run a Python application that uses OPS and the generated IDL files, the Python interpreter need to find the files. This can be done in several ways.

  * Installing OPS and generated files into the Python package directory
  * Setting PYTHONPATH to point to OPS_DIR/Python resp. the generated files directory
  * Copying the source directories into the test program directory

See the 'run' script-file in the example directory for how to use the PYTHONPATH method.

## Ada Examples ##
Under OPS_DIR/Examples, you find Ada examples in the directory **AdaApps**.

## Delphi Examples ##
Under OPS_DIR/Examples, you find Delphi examples in the directory **DelphiApps**.

## Java Examples ##
Under OPS_DIR/Examples, you find Java examples in the directory **JavaApps**.

## C# Examples ##
Under OPS_DIR/Examples, you find C# examples in the directory **CSharpApps**.
