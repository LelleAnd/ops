
Outdated information

---

# Publish Subscribe Domain Model Used by OPS #


![domain_model_overview.JPG](domain_model_overview.JPG)

The UML diagram should be read as: Data types are communicated on Topics. Publishers publish (or write) on Topics, Subscribers subscribes to (or reads from) Topics. A Topic belongs to a Domain. Participants connects to and is the entry point for an application to a Domain. A Participant use Transports to send data to other Participants on the same Domain.


Arkitektur

Design

# Transport #

# Serialization #

Nya features

Arv

virtual

getMessage

config

Participant

multicast

boost asio

header files, public and private

coding guidlines

stora paket (begränsningar)

threading

threadpool

protocol

ConfigurationLib


java lider av C++ optimerad kod (obs endast stora paket)
varför inte NIO
varför NIO.2

varför ingen writeReliable
RequestReply



Verktyg

Netbeans modules
Compilers
> exemplifiera med Java Compiler
AbstractTemplateBasedIDLCompiler
plugins till compilern - exempel generera UML till EA

CompilerLib
IDLParser
java compiler compiler
Debuggern
Ny debuggern
