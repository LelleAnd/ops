@pushd %~dp0
@start ..\deploy\bin\OPSListener.exe -c OPSIdls\PizzaProject\ops_config.xml -SA -d -j OPSIdls\PizzaProject\Generated\JSON\PizzaProject.json
@call :STARTTEST CSharpApps\OPSTest\OPSTest\ bin\debug\OPSTest.exe "start"
@call :STARTTEST AdaApps\PizzaTest\ pizzatest_main.exe "start"
@call :STARTTEST CppApps\PizzaTest\ debug\PizzaTest.exe "start"
@call :STARTTEST DelphiApps\PizzaTest\ Win32\Debug\PizzaTest.exe "start"
@call :STARTTEST JavaApps\OPSTestApp\ PizzaTest.jar "start java -jar"
@call :STARTTEST PythonApps\PizzaTest\ run.bat "call"
@popd
@GOTO :EOF

@rem ------- Args: relpath exepath cmd
:STARTTEST
@pushd .
@cd %1
if exist %2 ( %~3 %2 )
@popd
