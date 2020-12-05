[Examples](SimpleCpp.md) [BuildInstructions](BuildInstructions.md)

---

# OPS IDL Compiler #
Learn more about the OPS IDL language [here](IDLLanguage.md).

To compile IDL files into target languages, OPS comes with a command line tool called ***opsc***.
The following shows the available command arguments:

```
D:\OPS\ops4>Tools\opsc\opsc.bat -h

Invoking script at: D:\OPS\ops4\Tools\opsc\
Current working directory: D:\OPS\ops4

opsc -P <IDL_proj_dir> [options]
  or
opsc [options] idlfiles...

OPTIONS
  -? | -h | --help  show this help
  -b <feature>      build given feature
  -B <feature>      don't build given feature
  -d                verbose output
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
  -s <feature>      special, generate with given feature
  -t <dir>          set template directory (overrides built-in templates)

  -gpr <path>       explicit path to ops4.gpr when generating Ada
                    (default is to use GPR_PROJECT_PATH to find ops4.gpr)
  -dll <file>       used when building C# to give any dll dependencies (eg. OpsLibrary.dll)
  -jar <file>       used when building Java to give any jar dependencies

FEATURE
  for generate: ALL, ada, cpp, csharp, delphi, java, json, python, debug
  for build:    ALL, csharp, java
  for special:  mempool
```

Examples of usage:

To compile IDL's for the TestAll example, run the following command:
```
  D:\OPS\OPS4> deploy\lib\ops\opsc.bat -P Examples\OPSIdls\TestAll
```
This creates a 'Generated' directory in the TestAll directory with all generated files.

NOTE: For ***opsc*** to compile C# code to dll's with the 'correct' C# compiler, you may need to set the environment symbol **OPS_CSC_PATH** to the directory where the wanted compiler binary exist.

NOTE: For ***opsc*** to compile Java code to jar-files with the 'correct' Jaca compiler, you may need to set the environment symbol **OPS_JAVA_PATH** to the directory where the wanted ***javac*** binary exist.

Example:

```
set OPS_CSC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\Roslyn
```
