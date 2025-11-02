[Examples](SimpleCpp.md) [BuildInstructions](BuildInstructions.md)

---

# OPS IDL Compiler #
Learn more about the OPS IDL language [here](IDLLanguage.md).

To compile IDL files into target languages, OPS comes with a command line tool called ***opsc***.
The following shows the available command arguments:

```
D:\OPS\ops4> Tools\opsc\opsc.bat -h

Invoking script at: D:\OPS\ops4\Tools\opsc\
Current working directory: D:\OPS\ops4

opsc -P <IDL_proj_dir> [options] [-ref idlfiles...]
  or
opsc [options] idlfiles... [-ref idlfiles...]

  idlfiles...       one or more names; name can be a file with extension '.idl' or
                    a directory (ended with a path separator) to be recursively searched
                    for idlfiles

OPTIONS
  -? | -h | --help  show this help
  -b <feature>      build given feature
  -B <feature>      don't build given feature
  -d[d]             verbose output
  -dump             print all parsed objects
  -fac              only generate factories for given features
  -g <feature>      generate given feature
  -G <feature>      don't generate given feature
  -idls <file>      get idl-filenames from given file
                    (one filename per line in file, paths relative to <file>)
  -o <dir>          set output directory
  -p <projname>     set project name
  -P <IDL_proj_dir> use as project directory with pre-defined subdirectories
  -parse            only parse, don't generate
  -pp <file>        name an ops IDL project.properties file
  -printProps       print system props
  -ref              remaining idlfiles specified are only used as reference (no code gen)
  -retain <feature> retain old behaviour for given feature
  -s <feature>      special, generate with given feature
  -S <feature>      special, don't generate with given feature
  -t <dir>          set template directory (overrides built-in templates)

  -gpr <path>       explicit path to ops4.gpr when generating Ada
                    (default is to use GPR_PROJECT_PATH to find ops4.gpr)
  -dll <file>       used when building C# to give any dll dependencies (eg. OpsLibrary.dll)
  -jar <file>       used when building Java to give any jar dependencies
  -pypac <file>     used when generating Python packages to give any Python package dependencies (*.opsc files)

FEATURE
  for generate: ALL, ada, cpp(*), csharp(*), delphi, java(*), json, python(*), debug
  for build:    ALL, csharp, java
  for special:  mempool, jsonver(*), pyinit, pypack
                (*) == Default enabled
  for retain:   char  -> In generated C++, use 'char' for idl byte instead of 'uint8_t'

```
## Language specifics ##
### Ada ###
When generating Ada code, ***opsc*** also generates a GNAT Ada project file (*projectname.gpr*). As default it relies on the environment symbol *GPR_PROJECT_PATH* to include the path to the *ops4.gpr* file. To avoid this, use the *-gpr* switch to specify an absolute or relative path to the *ops4.gpr* file.

### C++ ###
The special feature *mempool* is an experimental feature, generating *new* and *delete* members in the idl data classes that uses a memory pool for memory management. To define the pool, you need to add the following to a cpp-file:

```
///   <idlclassname>::memory_pool_type <idlclassname>::_pool(<max_number_of_objects>);
```

### C# ###
When building C# to a dll, any dependencies need to be given using the *-dll* switch. This includes the OPS core *OpsLibrary.dll* and any other separately built idl's.

### Java ###
When building Java to a jar-file, any dependencies need to be given using the *-jar* switch. This includes any other separately built idl's.

### JSON ###
The special feature *jsonver*, defines if the idl version feature should be reflected in the JSON file or not. This feature should normally NOT be turned off.

### Python ###
The special feature *pyinit*, generates constructors with parameters for all members (with default values). If a compiled idl inherits from a separately compiled idl, the *-ref* switch with idls may be given so that the compiler can find needed members.

The special feature *pypack*, instructs the compiler to generate the idls into a Python package. A special opsc-file, *projectname.opsc*, is also generated to be used in case other separate compiled idls refers to idls in this project. The *-pypac* switch is used to give the compiler such opsc-files.

## Example of usage: ##

To compile IDL's for the TestAll example, run the following command:
```
  D:\OPS\OPS4> deploy\lib\ops\opsc.bat -P Examples\OPSIdls\TestAll
```
This creates a 'Generated' directory in the TestAll directory with all generated files.

For other usage examples, see file
```
  <ops>\Examples\Build_Idls.bat
```

NOTE: For ***opsc*** to compile C# code to dll's with the 'correct' C# compiler, you may need to set the environment symbol **OPS_CSC_PATH** to the directory where the wanted compiler binary exist.

NOTE: For ***opsc*** to compile Java code to jar-files with the 'correct' Jaca compiler, you may need to set the environment symbol **OPS_JAVA_PATH** to the directory where the wanted ***javac*** binary exist.

Example:

```
set OPS_CSC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\Roslyn
```
