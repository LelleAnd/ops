# OPS – Installation #

- - -
NOTE: This page contains a lot of outdated info and need to be updated
- - -

## Set Up Your Environment ##
Make sure you have a Java SE Development Kit (JDK, version 8 or higher) installed on
your system. The tools you use to create systems with OPS are all running in Java and
use the Java compiler to generate pre-compiled code to use for debugging. If you know
you will only use OPS for C++ and C# and do not need any debugging capabilities, it may
be sufficient to only have a Java runtime environment installed. It is strongly
recommended to install a JDK though.

To build or run the OPS IDL Builder, JDK version 8 or higher is required.

If you do not have a JDK installed on your system follow these steps:
  1. Download the latest JDK from oracle.com, (Tip! Select a bundle with Netbeans and you are ready to open the OPS sample Netbeans Java projects!)       http://www.oracle.com/technetwork/java/index.html
  1. Install it, you will need administrator rights on your system.
  1. When installed, add the newly created directory
 > "C:\Program Files\Java\jdk.jdk.version\bin" to your user environment variable
 > called path, you can do this without being an administrator of your system.

---

## Build OPS Binaries ##

Look at the [build instructions](BuildInstructions.md) to build OPS from source.

---

## Run the IDL Builder and Build TestAll Example Project ##
* Open OPS IDL Builder, from OPS\_DIR\Tools\OPS IDL Builder\bin\ops\_idl\_builder\_nb.exe
* Use the OPS IDL Builder to open TestAll sample IDL project (File->Open
Project...
* browse to OPS\_DIR\Examples\OPSIdls\TestAll,
* check open as main project and click Open Project.

Once open, expand "TestAll" and "testall" package folder and checkout the IDL files making up this example.

When ready, build the project by hitting the blue build hammer or right click your project and select "Build".

When you build your project, a dos prompt will pop up showing you the progress of building java jar files. This output is quite verbose and it can be hard to see if it succeeds or not.

Checkout the following video to see what it should look like. If it does
not work, it is probably because you have not put your Java JDK on your path correctly
and it will output something like "javac is not a command or program".

(Note, click the video to see it on YouTube in fullscreen)

<a href='http://www.youtube.com/watch?feature=player_embedded&v=AxutJA2-DNQ' target='_blank'><img src='http://img.youtube.com/vi/AxutJA2-DNQ/0.jpg' width='425' height=344 /></a>

You can read more about the OPS IDL Builder [here](IDLCompilerTutorial.md).

## Use the OPS Debugger to Get Going With Some Pub-Sub ##

* Open the OPS Debugger (double click OPS\_DIR\Tools\OPS Debugger\OPSDebugger2.jar)
* Once open, click "File->Open Project..." and browse to OPS\_DIR\Examples\OPSIdls\TestAll\DebugProject.xml
* To the right you can now see a drop down list with topics (ChildTopic in this case).
* Drag the Topic from the tree view on Subscribers or Publishers to subscribe to, or publish on the topic.

Watch this video to see how to get some action:

<a href='http://www.youtube.com/watch?feature=player_embedded&v=W3bupBA16VM' target='_blank'><img src='http://img.youtube.com/vi/W3bupBA16VM/0.jpg' width='425' height=344 /></a>

## Build and Run C++ Examples ##
Now, open
OPS\_DIR\Examples\TestAllC++Test\TestAllC++Test\TestAllC++Test.sln
with MS Visual Studio 2008 C++ Express or pro.
You vill see that the solution contains two projects:
TestAll\_Subscribe  and
TestAll\_Publish

Build the solution and make sure you get no errors (at some installations you will get
linker warnings, this is a known issue.)
When build succeds, hit run and both projects should open and start communicating.

## Build and Run Java Examples ##

---
Note, this instruction assumes you have [Netbeans IDE](http://www.netbeans.org) installed.
if you haven't, you can still use the sources provided under OPS\_DIR\TestAllJavaTest\src
to build with whatever tool you like to use. Just make sure you link
OPS\_DIR\Java\OPSJLib.jar.

---

Open Netbeans IDE. Once open click File->Open Project... and browse to
OPS\_DIR\TestAllJavaTest
and open TestAllJavaTestPublisher and TestAllJavaTestSubscriber
Build the projects and run them one by one (Right click project and select Run).
In the ouput windows you should now see an indication that communication is working.
Try starting the OPS Debugger and the C++ subscriber and publisher examples from above
and see that they get and send data to Java and vice verca.

---

Note, if you see a "Reference Problem" alert when opening the projects, click "Resolve"
and browse to the missing jar files (OPS\_DIR\Java\OPSJLib.jar and
OPS\_DIR\Examples\TestAll\Generated\TestAll.jar)

---

If you've come this far, you are ready create some apps of your own, good luck!
