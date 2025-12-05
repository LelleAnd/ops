@SETLOCAL
@pushd %~dp0\..

@call :build Examples\JavaApps\OpsTestApp Build_PizzaTest.bat
@if errorlevel 1 goto :error
@call :build Examples\JavaApps\ParseTest Build_ParseTest.bat
@if errorlevel 1 goto :error
@call :build Examples\JavaApps\TestAll Build_VerifySerDes.bat
@if errorlevel 1 goto :error

@popd
@goto :eof

:error
@echo Building FAILED !!!!
@popd
@goto :eof

@rem call :build path bat_file
:build
@if not exist %~1\%~2 goto :eof
@echo Building %~1 ...
@pushd %~1
@call %~2
@popd
@goto :eof
