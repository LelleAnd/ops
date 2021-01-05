@pushd %~dp0
@echo Compiling all IDL's ...
@call ..\Common\idl\build.bat
@if errorlevel 1 goto :error

@call ..\Examples\Build_Idls.bat
@if errorlevel 1 goto :error

@call ..\Tools\OPSBridge\idl\build.bat
@if errorlevel 1 goto :error
@popd
@goto :eof

:error
@popd
