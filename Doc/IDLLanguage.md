# The OPS IDL Language #
## What is IDL? ##
IDL stands for Interface Definition Language and is a common name for programming language neutral languages used in middlewares to enable support for multiple programming languages (E.g. C++ and Java). In OPS the IDL language is simple and is only used to define data classes. Once a data class is defined in IDL, it can be “compiled” into the desired target programming languages that will be used in your distributed application, see [IDL Builder](IDLCompilerTutorial.md) and [IDL Compiler](IDLCommandlineCompiler.md).

## Basics ##
The OPS IDL language resembles simplified Java and is due to its minimalistic design easy to learn and read. As mentioned above, the language is made for designing data classes, and that is in fact also the only thing it can do. A data class definition in OPS IDL must contain the following:

  * One and only one package definition
  * One and only one class definition

```
package samples;
class SampleData
{

}
```

The package definition consists of the reserved word **package** followed by the name of the package for the data class. Package in OPS IDL is equivalent to package in Java or namespace in C++ or C#. The package shall be expressed as a single word or as a series of dot separated words, e.g. “samples” or “samples.subsamples” etc and the package declaration shall be ended with a “;”.

The class definition is the reserved word **class** followed by a single word for the class name followed by a body marked with an opening and a closing brace, “{     }”.

Within the body of the class, an arbitrary number of enum type definitions, constants and data fields can be declared.
```
enum Order { UNDEFINED, START, STOP };

const int max = 42;

double d;
```
An enum type definition is the reserved word **enum** followed by the enum name and a list of enumeration values within braces.

A constant is the reserved word **const** followed by the type, a name and assignment of a value. Constants can only be of the OPS IDL defined basic types.

A data field can be of any of the OPS IDL defined basic types, an array of a basic type, another user defined data class or an array of such a data class.

The following table shows a listing of all available OPS IDL types and their corresponding representations in common programming languages:

| OPS IDL | C++ | Java | C#  | Delphi | Serialized on the network |
| ------- | --- | ---- | --- | ------ | ------------------------- |
| package | namespace | package | namespace | Unit | -            |
| class   | class | class | class | Class | -                     |
| enum    | enum class | enum | enum | ( , , ) | 2 bytes          |
| const   | static const | static final | const | const | -       |
| boolean | bool | boolean | bool | Boolean | 1 byte              |
| byte    | char | byte | byte | Byte | 1  byte                   |
| short   | int16 | short | short | Int16 | 2 bytes               |
| int     | int32 | int | int | Int32 | 4 bytes                   |
| long    | int64 | long| long | Int64 | 8 bytes                  |
| float   | float | float| float | Single | 4 byte                |
| double  | double | double | double | Double | 8 bytes           |
| string  | std::string | String | string | AnsiString | 4 bytes (size) + 1 byte per character (8-bit) |
| string`<size>` | fixed_string`<size>` | String | string | AnsiString | 4 bytes (size) + 1 byte per character (8-bit) |
| T       | T  | T  | T | T | size_of( T ) |
| T`[]` | std::vector< T > | java.util.Vector< T > | List< T > | array of T | 4 bytes (size) + size_of( T ) per element |
| T`[size]` | T`[size]` | java.util.Vector< T > | List< T > | array`[0..size-1]` of T | 4 bytes (size) + size_of( T ) per element |

where **size_of( T )** is the serialized size of a string for the data type plus the serialized size of all fields in T. If element _optNonVirt_ is set to true in the _Domain_ section of the [OPS configuration file](OpsConfig.md), the string for the data type is sent as an empty string for non-virtual fields, see description of inheritance below.

Defined constants are never sent on the network. They are just generated to the target language source files to be used by the users code.

IDL code example:

```
package samples;
class SampleData
{
   const int max = 42;

   boolean boo;
   byte b;
   short sh;
   int i;
   long l;
   float f;
   double d;
   string s;
   string<25> s25;
   UserData uData;

   enum Order { UNDEFINED, START, STOP };
   Order command;

   boolean[] boos;
   byte[] bytes;
   short[] shorts;
   int[] ints;
   long[] longs;
   float[] floats;
   double[] doubles;
   string[] strings;
   string<43>[] s43vect;
   UserData[] uDatas;

   int[42] intarr;
}
```

This yields the following generated code in [C++](SampleDataCpp.md) and [Java](SampleDataJava.md).

## Inheritance ##

In addition to the simple approach above, OPS IDL also supports inheritance to some extent. A class can extend one other class (multiple inheritance is not supported):

```
package samples;
class ChildData extends SampleData
{

}
```

Which in this case makes ChildData extend all fields from SampleData. See also [Version handling](VersionHandling.md) of OPS IDLs.

As we saw in the sample above it is possible to declare fields of another user defined type, for example, we could declare a field of our new type as follows:

`ChildData childData;`

This is of course OK, but the inheritance does not give us much more then saving a few lines of IDL.
What we normally would like to do when using inheritance is to have some sort of polymorphism. And the natural way to do that would seem to be to declare the field of the base type:

1.
`SampleData childData;`

While this is a valid declaration it does not give us the opportunity to use the field as a ChildData. This because OPS tries to use static memory allocation as often as possible and this type of declaration will generate code (if allowed by the target language) that allocates the field inline with the enclosing class rather than as a separate memory block and thus not allow it to change implementation dynamically.
To tell the OPS code generators to allow for dynamic memory allocation and support for polymorphism the keyword **virtual** is used like this:

2.
`virtual SampleData childData;`

To clearify things a bit let us see what the two ways of declaring a field would affect the code generation in C++.

In case 1. The field would be declared the same way in C++ as in IDL:

`SampleData childData;`

And thus have its default constructor called on creation of the enclosing class.

Case 2. on the other hand, would generate a pointer declaration like this:

`SampleData* childData;`

Which then allows the implemation of the pointer to be decided and changed by the user dynamically.

Arrays are treated in the same manner, to declare an array of objects whos implementation is unknown at compile time use the following approach:

`virtual SampleData[] childDatas;`

To see a useful situation when, how and why to use inheritance in OPS, have a look at the WeatherStationExample.

## Comments ##

Two types of comments are allowed in OPS IDL:

1. `//This is a comment for the rest of the line...`
2. `/* This is an ended comment, that can stretch over several lines */`

More than just beeing two ways of accomplishing the same thing, a comment as in 1. stays in the IDL and will not be visible in the generated code while comments like in 2. will be comments transferred to the target language.

## Compile directives ##

There is a special form of comments that is used to give instructions to the OPS code generators. These starts with *//@* followed by a key word and optionally a value and look like this:  

`//@ directive [ = value | = low .. high ]`

| Directive | Values | Default | Applies to | Description |
|-----------|--------|---------|------------|-------------|
| toplevel  | true / false | true | class | Publishers/Subscribers are only generated for toplevel classes. This directive is currently only supported for Ada, C++ and Java. |
| nofactory | - | - | class | Class is only ment to be a base class in a class hierarchy, so don't generate factory code for it (i.e. an object of this type can't be created by the OPS factory and therefore no field can be of this type). This also implies that the *toplevel* directive is false. |
| onlydefinition | - | - | class | Class only contains definitions (constants and enums) and should not be part of data objects published. Useful for defining common constants and enums. |
| range     | int or float number range | - | field | Validation code for the field will be generated. Valid field values are: 'low' <= value <= 'high'. See also [message validation](MessageValidation.md). |
| version   | number | - | field | The field is only valid for versions >= 'number'. Valid range for 'number' is 0..255. Several version directives can be given for a field. See also [version handling](VersionHandling.md) of IDLs. |
| version   | number range | - | field | The field is only valid for the range: 'low' <= version <= 'high'. Several version directives can be given for a field. See also [version handling](VersionHandling.md) of IDLs. |

Example of usage:

```
package samples;
//@ toplevel = false
class UserData
{
  //@range = -42 .. 42
  int A;

  //@version = 1 .. 4 //@version = 10
  int optional_B;
}
```

#### see also ####
[IDL Builder](IDLCompilerTutorial.md) for how to edit and compile OPS IDL files.

[IDL Compiler](IDLCommandlineCompiler.md) for how to use the command line compiler to compile OPS IDL files.
