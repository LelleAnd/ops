# Message validation #

The integer and floating point data types used in the [IDL language](IDLLanguage.md) are unconstrained 8-, 16-, 32- or 64-bit values. To add some constraints on these, the compile directive *//@range* has beeen added, see the [IDL language](IDLLanguage.md) definition.

When the directive *//@range* is added to a field, the opsc compiler will add code to the generated data class so that marked fields can be checked against the given constraints (the implicit version field and all enum type fields, will automatically get checks generated).

The user can then define if this message validation should be used and if so, how an invalid message should be handled. For backward compatibility the default is to NOT use message validation.

The default behavior can be changed on the *Participant* before any *Publisher* or *Subscriber* is created. Then it is possible to change the behavior for individual *Publishers* and *Subscribers*.

Currently this is only generated and implemented for C++, see also the [Feature List](FeatureList.md).
