@SETLOCAL
@pushd %~dp0\..

@set XMLADA_BUILD=static

@call :build Examples\AdaApps\Example1 example1.gpr
@if errorlevel 1 goto :error
@call :build Examples\AdaApps\GenSub gensub.gpr
@if errorlevel 1 goto :error
@call :build Examples\AdaApps\PizzaTest pizzatest.gpr
@if errorlevel 1 goto :error
@call :build Examples\AdaApps\TestAll verifyserdes.gpr
@if errorlevel 1 goto :error
@call :build Ada\Project opsadatest.gpr
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
gprbuild -p %~2
@popd
@goto :eof
