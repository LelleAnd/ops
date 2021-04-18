@SETLOCAL
@pushd "%~dp0\..\Cpp\Visual C++\OPSCrossPlatformCppSolution_2019"

@set target=
@if %1.==. goto :noarg
@set target=-t:Rebuild
:noarg

msbuild -m %target% -p:Configuration="Debug";Platform=x86
@if errorlevel 1 goto :error
msbuild -m %target% -p:Configuration="Debug DLL";Platform=x86
@if errorlevel 1 goto :error
msbuild -m %target% -p:Configuration="Release";Platform=x86
@if errorlevel 1 goto :error
msbuild -m %target% -p:Configuration="Release DLL";Platform=x86
@if errorlevel 1 goto :error
msbuild -m %target% -p:Configuration="Debug";Platform=x64
@if errorlevel 1 goto :error
msbuild -m %target% -p:Configuration="Debug DLL";Platform=x64
@if errorlevel 1 goto :error
msbuild -m %target% -p:Configuration="Release";Platform=x64
@if errorlevel 1 goto :error
msbuild -m %target% -p:Configuration="Release DLL";Platform=x64
@if errorlevel 1 goto :error

@popd
@goto :eof

:error
@echo Building FAILED !!!!
@popd
@goto :eof
