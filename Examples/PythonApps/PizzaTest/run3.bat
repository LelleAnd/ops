@setlocal
@set OPS_PATH=%~dp0\..\..\..\Python
@set GEN_PATH=%~dp0\..\..\OPSIdls\PizzaProject\Generated\Python
@rem -- @set GEN_PATH2=%GEN_PATH%\PizzaProject
@rem -- @set PYTHONPATH=%OPS_PATH%;%GEN_PATH%;%GEN_PATH2%;%PYTHONPATH%
@set PYTHONPATH=%OPS_PATH%;%GEN_PATH%;%PYTHONPATH%
@echo %PYTHONPATH%
@"C:\Program Files (x86)\Microsoft Visual Studio\Shared\Python37_64\python.exe" OPS_Test.py
