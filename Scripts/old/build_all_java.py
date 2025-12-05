import os
import shutil

toolsPath = "../Tools/"
libsPath = "../Libs/"
javaPath = "../Java/"
antCmd = "ant -autoproxy -buildfile "
#java_home_cmd = "set JAVA_HOME=C:/Program/Java/jdk1.6.0_13\n"

#First, build all basic jar-files
if(os.system(antCmd + libsPath + "ConfigurationLib/build.xml") != 0):
    print("ConfigurationLib FAILED")
if(os.system(antCmd + javaPath + "OPSJLib/build.xml") != 0):
    print("OPSJLib FAILED")
if(os.system(antCmd + libsPath + "JarSearch/build.xml") != 0):
    print("JarSearch FAILED")
if(os.system(antCmd + toolsPath + "IDLParser/build.xml") != 0):
    print("IDLParser FAILED")
if(os.system(antCmd + toolsPath + "OPSCompilerLib/build.xml") != 0):
    print("OPSCompilerLib FAILED")
if(os.system(antCmd + toolsPath + "OPSReflectionLib/build.xml") != 0):
    print("OPSReflectionLib FAILED")

#Next, copy built jar files to their wrapper libs for NB platform
extPath = "/release/modules/ext/"

shutil.copy(libsPath + "ConfigurationLib/dist/ConfigurationLib.jar", toolsPath + "ConfigurationLibWrapper" + extPath + "ConfigurationLib.jar")
shutil.copy(javaPath + "OPSJLib/dist/OPSJLib.jar", toolsPath + "OPSJLibWrapper" + extPath + "OPSJLib.jar")
shutil.copy(libsPath + "JarSearch/dist/JarSearch.jar", toolsPath + "JarSearchWrapper" + extPath + "JarSearch.jar")
shutil.copy(toolsPath + "IDLParser/dist/IDLParser.jar", toolsPath + "IDLParserWrapper" + extPath + "IDLParser.jar")
shutil.copy(toolsPath + "OPSCompilerLib/dist/OPSCompilerLib.jar", toolsPath + "OPSCompilerLibWrapper" + extPath + "OPSCompilerLib.jar")

#And build the NB platform projects... For now just open netbeans and build "OPS IDL Builder NB"


