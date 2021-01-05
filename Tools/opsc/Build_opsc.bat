@pushd %~dp0
@echo Building opsc ...
@javac -version
@IF NOT EXIST "build/classes" (
	mkdir "build/classes"
)
@IF NOT EXIST dist (
	mkdir dist
)

@echo Copying ..\..\Libs\ConfigurationLib\dist\ConfigurationLib.jar
@copy /Y "..\..\Libs\ConfigurationLib\dist\ConfigurationLib.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@echo Copying ..\..\Java\OPSJLib\dist\OPSJLib.jar
@copy /Y "..\..\Java\OPSJLib\dist\OPSJLib.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@echo Copying ..\OPSCompilerLib\dist\OPSCompilerLib.jar
@copy /Y "..\OPSCompilerLib\dist\OPSCompilerLib.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@echo Copying ..\OPSCompilerLib\dist\OPSCompilerLib.jar
@copy /Y "..\IDLParser\dist\IDLParser.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@echo Copying ..\NBOPSIDLSupport\dist\IDLTemplates.jar
@copy /Y "..\NBOPSIDLSupport\dist\IDLTemplates.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@javac -cp "dist\ConfigurationLib.jar;dist\OPSJLib.jar;dist\IDLParser.jar;dist\OPSCompilerLib.jar;dist\IDLTemplates.jar" @"src/files.txt" -d "build/classes"
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@rem Generate the manifest file needed below
@echo Class-Path: OPSCompilerLib.jar IDLParser.jar ConfigurationLib.jar OPSJLib.jar IDLTemplates.jar > dist\MANIFEST.MF

@jar cfme "dist/opsc.jar" "dist/MANIFEST.MF"  "opsc/OpsCompiler" -C "build/classes/" .
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@goto :BUILD_OK

:BUILD_FAILED
@echo ------ BUILD FAILED ------
@pause

:BUILD_OK
@popd
