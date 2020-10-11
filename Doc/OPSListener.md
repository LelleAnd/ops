# OPS Listener #
To listen to OPS traffic, OPS comes with a command line tool called ***OPSListener***.
The following shows the available command arguments:

```
D:\OPS\ops4>deploy\bin\OPSListener.exe -?

OPSListener Version 2020-09-27

  This program can subscribe to any OPS topic and it is possible to choose what information to present and in which order.
  This can be used to test if / verify that topics are published.
  Topic names can use wildcards (eg. ".*zz.*" means any topic with 'zz' any where in the name).


Usage:
  OPSListener [-v] [-?] [-c ops_cfg_file [-c ops_cfg_file [...]]]
              [-j json_file [-j json_file [...]]]
              [-t] [-pA | -p<option_chars>] [-d [num]]
              [-a arg_file [-a arg_file [...]]]
              [-GA | -G domain [-G domain [...]]]
              [-IA | -I domain [-I domain [...]] [-O]]
              [-SA | -S domain [-S domain [...]]]
              [-D default_domain] [-C [-E]] [-u] [-n] Topic [Topic ...]

    -?                 Shows a short description
    -a arg_file        File with command line arguments
    -c ops_config_file Specifies an OPS configuration file to use
                       If none given, the default 'ops_config.xml' is used
    -C                 Do a publication ID check
    -d [num]           Dump message content in hex (the part derived from OPSObject)
    -D default_domain  Default domain name to use for topics given without domain name
                       If none given, the default 'SDSDomain' is used
                       A new default can be given between topics
    -E                 If -C given, minimize normal output
    -G domain          Subscribe to Debug Request/Response from given domain
    -GA                Subscribe to Debug Request/Response from all domains in given configuration files
    -I domain          Subscribe to Participant Info Data from given domain
    -IA                Subscribe to Participant Info Data from all domains in given configuration files
    -j json_file       Specifies a JSON-file with OPS Message descriptions (generated from opsc)
    -n                 Don't subscribe to topics following
    -O                 if -I or -IA given, only show arriving and timed out participants
    -p<option_chars>   Defines for received messages, which fields to print and in which order
                 n       Publisher Name
                 i       Publication Id
                 T       Topic Name
                 y       Type
                 s       Sparebytes Size
                 k       Key
                 S       Source IP::Port
                 v       Version mask OPSMessage
                 V       Version mask contained OPSObject
    -pA                Short for all option chars in the program default order
    -S domain          Subscribe to all topics in given domain
    -SA                Subscribe to all topics in all domains in given configuration files
    -t                 Print receive time for each message
    -u                 Force subscription to UDP static route topics (may interfere with real subscriber)
    -v                 Verbose output during parsing of command line arguments
```

The source for the tool is in the OPS _Tools_ folder and the built binary in the _deploy/bin_ folder.
