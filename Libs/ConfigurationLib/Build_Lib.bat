@pushd %~dp0
@echo Building ConfigurationLib ...
@javac -version
@IF NOT EXIST "build/classes" (
	mkdir "build/classes"
)
@IF NOT EXIST dist (
	mkdir dist
)

@rem @javac -Xlint:unchecked @"src/files.txt" -d "build/classes"
@rem @javac -Xlint:deprecation @"src/files.txt" -d "build/classes"
@rem @javac -Xlint @"src/files.txt" -d "build/classes"
@javac @"src/files.txt" -d "build/classes"
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@jar cf "dist/ConfigurationLib.jar" -C "build/classes/" .
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@goto :BUILD_OK

:BUILD_FAILED
@echo ------ BUILD FAILED ------
@pause

:BUILD_OK
@popd
