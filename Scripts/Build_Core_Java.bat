@pushd %~dp0
@echo Compiling Java libraries ...
@call ..\Libs\ConfigurationLib\Build_Lib.bat
@if errorlevel 1 goto :error

@call ..\Libs\JarSearch\Build_Lib.bat
@if errorlevel 1 goto :error

@call ..\Java\OPSJLib\Build_Lib.bat
@if errorlevel 1 goto :error

@popd
@goto :eof

:error
@popd
