# Message validation #

The integer and floating point data types used in the [IDL language](IDLLanguage.md) are unconstrained 8-, 16-, 32- or 64-bit values. To add some constraints on these, the compile directive *//@range* has beeen added, see the [IDL language](IDLLanguage.md#compile-directives) definition.

When the directive *//@range* is added to a field, the opsc compiler will add code to the generated data class so that marked fields can be checked against the given constraints (the implicit version field and all enum type fields, will automatically get checks generated).

In a similar way restrictions can be put on open arrays with the directive *//@maxarrlen* and on open strings with the directive *//@maxstrlen*.

The generated method can be called in user code to check if the object instance is valid. Currently validation code is only generated for C++ and Delphi, see also the [Feature List](FeatureList.md).

## C++ specifics
The user can define if the message validation should be called automatically and if so, how an invalid message should be handled. For backward compatibility the default is to NOT call message validation.

The default behavior can be changed on the *Participant* before any *Publisher* or *Subscriber* is created. Then it is possible to change the behavior for individual *Publishers* and *Subscribers*.

## Delphi specifics
The validation code is automatically called by the *Publisher* when an object is written. If the object isn't valid, an exception is thrown and the object is NOT published.
