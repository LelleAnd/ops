@pushd %~dp0
@echo Building OPSJLib ...
@javac -version
@IF NOT EXIST "build/classes" (
	mkdir "build/classes"
)
@IF NOT EXIST dist (
	mkdir dist
)

@javac -cp "../../Libs/ConfigurationLib/dist/ConfigurationLib.jar" @"src/files.txt" -d "build/classes"
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@jar cf "dist/OPSJLib.jar" -C "build/classes/" .
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@goto :BUILD_OK

:BUILD_FAILED
@echo ------ BUILD FAILED ------
@pause

:BUILD_OK
@popd
