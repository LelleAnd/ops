@pushd "%~dp0\..\Cpp\Visual C++\OPSCrossPlatformCppSolution_2019"

msbuild -m -t:Rebuild -p:Configuration="Debug";Platform=x86
@if errorlevel 1 goto :error
msbuild -m -t:Rebuild -p:Configuration="Debug DLL";Platform=x86
@if errorlevel 1 goto :error
msbuild -m -t:Rebuild -p:Configuration="Release";Platform=x86
@if errorlevel 1 goto :error
msbuild -m -t:Rebuild -p:Configuration="Release DLL";Platform=x86
@if errorlevel 1 goto :error
msbuild -m -t:Rebuild -p:Configuration="Debug";Platform=x64
@if errorlevel 1 goto :error
msbuild -m -t:Rebuild -p:Configuration="Debug DLL";Platform=x64
@if errorlevel 1 goto :error
msbuild -m -t:Rebuild -p:Configuration="Release";Platform=x64
@if errorlevel 1 goto :error
msbuild -m -t:Rebuild -p:Configuration="Release DLL";Platform=x64
@if errorlevel 1 goto :error

@popd
@goto :eof

:error
@echo Building FAILED !!!!
@popd
@goto :eof
