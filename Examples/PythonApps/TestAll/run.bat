@setlocal
@set OPS_PATH=%~dp0\..\..\..\Python
@set GEN_PATH=%~dp0\..\..\OPSIdls\TestAll\Generated\Python
@rem -- @set GEN_PATH2=%GEN_PATH%\TestAll
@rem -- @set PYTHONPATH=%OPS_PATH%;%GEN_PATH%;%GEN_PATH2%;%PYTHONPATH%
@set PYTHONPATH=%OPS_PATH%;%GEN_PATH%;%PYTHONPATH%
@echo %PYTHONPATH%
@python VerifySerDes.py
@rem @python -m cProfile VerifySerDes.py
