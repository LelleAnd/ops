@SETLOCAL

@rem find out where this is script is so we can invoke jars
@set SCRIPT_PATH=%~dp0

@echo.
@echo Invoking script at: %SCRIPT_PATH%
@echo Current working directory: %CD%

@rem make it possible to use the script from the source tree, when compiler
@rem built with the windows bat-files that places jars in source tree
@IF NOT EXIST %SCRIPT_PATH%opsc.jar set SCRIPT_PATH=%SCRIPT_PATH%dist\

@IF DEFINED OPS_JAVA_PATH PATH=%OPS_JAVA_PATH:java.exe=%;%PATH%

@pushd .
@java -version
@java -jar %SCRIPT_PATH%opsc.jar %*
@popd
