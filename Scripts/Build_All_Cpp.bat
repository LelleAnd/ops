@SETLOCAL
@pushd %~dp0\..

@set target=
@if %1.==. goto :noarg
@set target=-t:Rebuild
:noarg

@call :build Examples\CppApps\Example1 Example1.vcxproj
@if errorlevel 1 goto :error
@call :build Examples\CppApps\HelloRequesterCppImpl RequestHelloTopic_example.sln
@if errorlevel 1 goto :error
@call :build Examples\CppApps\HelloWorldCppImpl HelloTopic_example.sln
@if errorlevel 1 goto :error
@call :build Examples\CppApps\HelloWorldWithKeyCppImpl HelloTopic_example.sln
@if errorlevel 1 goto :error
@call :build Examples\CppApps\PizzaTest PizzaTest.vcxproj
@if errorlevel 1 goto :error
@call :build Examples\CppApps\TestAllC++Test\TestAllC++Test TestAllC++Test.sln
@if errorlevel 1 goto :error

@call :build Tools\DebugConsole DebugConsole.vcxproj
@if errorlevel 1 goto :error
@call :build Tools\OpsBridge\VC++\OPSBridge OpsBridge.vcxproj
@if errorlevel 1 goto :error
@call :build Tools\OpsListener\VC++\OPSListener OpsListener.vcxproj
@if errorlevel 1 goto :error
@call :build Tools\VerifyOPSConfig VerifyOPSConfig.vcxproj
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
