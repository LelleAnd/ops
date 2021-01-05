@pushd %~dp0
@echo Building IDLParser ...
@javac -version
@IF NOT EXIST "build/classes" (
	mkdir "build/classes"
)
@IF NOT EXIST dist (
	mkdir dist
)

@javac -cp "../OPSCompilerLib/dist/OPSCompilerLib.jar" @"src/files.txt" -d "build/classes"
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@jar cf "dist/IDLParser.jar" -C "build/classes/" .
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@goto :BUILD_OK

:BUILD_FAILED
@echo ------ BUILD FAILED ------
@pause

:BUILD_OK
@popd
