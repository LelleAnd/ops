@SETLOCAL
@pushd %~dp0\..

@set target=
@if %1.==. goto :noarg
@set target=-t:Rebuild
:noarg

@call :build Examples\CSharpApps\OPSTest\OPSTest OPSTest.csproj
@if errorlevel 1 goto :error
@call :build Examples\CSharpApps\ParseTest ParseTest.csproj
@if errorlevel 1 goto :error
@call :build Examples\CSharpApps\TestAll TestAll.csproj
@if errorlevel 1 goto :error

@popd
@goto :eof

:error
@echo Building FAILED !!!!
@popd
@goto :eof

@rem call :build path project_file
:build
@if not exist %~1\%~2 goto :eof
@echo Building %~1 ...
@pushd %~1
msbuild %~2 %target%
@popd
@goto :eof
