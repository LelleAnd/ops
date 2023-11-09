@pushd %~dp0
@call ..\..\..\Tools\opsc\opsc.bat -g ALL -o Generated src/*.idl
@if errorlevel 1 goto :error
@popd
@goto :eof

:error
@echo ERROR DETECTED
pause
@popd
