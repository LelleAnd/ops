@pushd %~dp0
@call ..\..\Tools\opsc\opsc.bat -g ALL -B ALL -gpr ../../../../Ada/ -o Generated *.idl -p opsidls
@if errorlevel 1 goto :error
@popd
@goto :eof

:error
@echo ERROR DETECTED
@pause
@popd
