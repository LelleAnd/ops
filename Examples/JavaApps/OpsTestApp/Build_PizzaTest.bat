@pushd %~dp0
@echo Building PizzaTest ...
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

@echo Copying ..\..\OPSIdls\PizzaProject\Generated\Java\PizzaProject.jar
@copy /Y "..\..\OPSIdls\PizzaProject\Generated\Java\PizzaProject.jar" dist
@IF ERRORLEVEL 1 goto :BUILD_FAILED

javac -cp "dist/OPSJLib.jar";"dist/ConfigurationLib.jar";"dist/PizzaProject.jar" @"src/consoleapp/files.txt" -d ".obj"
@IF ERRORLEVEL 1 goto :BUILD_FAILED

jar cfm "PizzaTest.jar" "src/consoleapp/manifest_console_app.txt" -C ".obj" .
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@goto :BUILD_OK

:BUILD_FAILED
@echo ------ BUILD FAILED ------
@pause

:BUILD_OK
@popd
