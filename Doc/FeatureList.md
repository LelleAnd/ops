# OPS Feature List #

The following table lists OPS Transport features and the support in different programming languages:

| Feature             | Ada | C++ | C# | Delphi | Python | Java |
| -------             | --- | --- | -- | ------ | -------|----- |
| Multicast           | X   | X   | X  | X      | X      | X    |
| Multicast ACK       | -   | X   | -  | -      | -      | -    |
| UDP Many2Many       | X   | X   | X  | X      | X      | X    |
| UDP Many2One        | X   | X   | X  | X      | X      | X    |
| TCP Many2Many       | X   | X   | X  | X      | X      | X    |
| TCP One2Many        | X   | X   | X  | X      | X      | X    |
| TCP V1              | X   | X   | X  | X      | X      | X    |
| TCP V2              | X   | X   | -  | -      | -      | -    |
| TCP V2, HB          | X   | X   | -  | -      | -      | -    |
| TCP Connect Status  | X   | (X) | -  | -      | -      | -    |
| In Process          | X   | X   | -  | -      | -      | -    |

The following table lists IDL Compile directives that only is supported by a subset of programming languages:

| Feature             | Ada | C++ | C# | Delphi | Python | Java |
| -------             | --- | --- | -- | ------ | -------|----- |
| toplevel            | X   | X   | -  | -      | -      | X    |

The following table lists some other OPS features and the support in different programming languages:

| Feature             | Ada | C++ | C# | Delphi | Python | Java |
| -------             | --- | --- | -- | ------ | -------|----- |
| Config Repository   | X   | X   | X  | X      | -      | X    |
| Debug Support       | -   | X   | -  | -      | -      | -    |
| Checksum Archiver   | X   | X   | -  | X      | X      | -    |
| PubID Checker       | -   | X   | -  | -      | -      | -    |
| Field Validation    | -   | X   | -  | -      | -      | -    |
