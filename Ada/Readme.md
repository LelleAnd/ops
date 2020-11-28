# Ada OPS Implementation #

The Ada implementation now has full functionality (i.e. the same as C++) with the following exceptions:

  - Subscribers don't have support for checking publication ID
  - Subscribers don't expose the history buffer

The implementation uses features from *Ada 2012* and uses the *XMLAda* library for parsing
ops configuration files and the *GNAT.Sockets* library for socket communication.

The implementation has so far been tested with ***GNAT GPS 2016*** and ***GNAT Community 2020*** on *Windows 10* and
***GNAT Pro 17.1*** on *Windows* and *Linux*.

Please note that ***GNAT Community 2019 (20190517)*** isn't supported. The compiler crashes with a GNAT BUG DETECTED. This has not been investigated further.


When generating Ada code from IDL's, the [opsc](../Doc/IDLCommandlineCompiler.md) compiler
also generates a GNAT Ada project file (*projectname.gpr*). As default it relies on the
environment symbol *GPR_PROJECT_PATH* to include the path to the *ops4.gpr* file.
To avoid this, use the *-gpr* switch on [opsc](../Doc/IDLCommandlineCompiler.md) to
specify an absolute or relative path to the *ops4.gpr* file.
