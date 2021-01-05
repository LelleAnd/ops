@pushd %~dp0
@echo Building VerifySerDes ...
@javac -version
@IF NOT EXIST .obj (
	mkdir .obj
)
@IF NOT EXIST dist (
	mkdir dist
)

@echo Copying ..\..\..\Java\OPSJLib\dist\OPSJLib.jar
@copy /Y "..\..\..\Java\OPSJLib\dist\OPSJLib.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@echo Copying ..\..\..\Libs\ConfigurationLib\dist\ConfigurationLib.jar
@copy /Y "..\..\..\Libs\ConfigurationLib\dist\ConfigurationLib.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@echo Copying ..\..\OPSIdls\TestAll\Generated\Java\TestAll.jar
@copy /Y "..\..\OPSIdls\TestAll\Generated\Java\TestAll.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

javac -cp "dist/OPSJLib.jar";"dist/ConfigurationLib.jar";"dist/TestAll.jar" @"src/files.txt" -d ".obj"
@IF ERRORLEVEL 1 goto :BUILD_FAILED

jar cfm "VerifySerDes.jar" "src/manifest_console_app.txt" -C ".obj" .
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@goto :BUILD_OK

:BUILD_FAILED
@echo ------ BUILD FAILED ------
@pause

:BUILD_OK
@popd
