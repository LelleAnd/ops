# VerifyOPSConfig #
To check OPS configuration files, OPS comes with a command line tool called ***VerifyOPSConfig***.
The following shows the available command arguments:

```

  Usage:
    VerifyOPSConfig [-?] [-debug] [-noNameCheck] <filename>
    VerifyOPSConfig [-?] [-debug] -buildinfo
    VerifyOPSConfig [-?] [-debug] -host
    VerifyOPSConfig [-?] [-debug] -list [<name>]
    VerifyOPSConfig [-?] [-debug] -resolve <name>

    -?                Help
    -debug            Print some debug info during work
    -noNameCheck      Skip hostname translation check

    -buildinfo        Show build info used when building OPS
    -host             Show host name
    -list [<name>]    List known interfaces for host or given name
    -resolve <name>   Try to find IP address

  Version 2025-11-23

  Program verifies that:
    - XML syntax is OK for OPS usage
    - OPS required entries exist
    - Domain names are unique
    - Channel names are unique within the given domain
    - Topic names are unique within the given domain

  Program also checks for some common mistakes and miss-configurations.
```

The source for the tool is in the OPS _Tools_ folder and the built binary in the _deploy/bin_ folder.
