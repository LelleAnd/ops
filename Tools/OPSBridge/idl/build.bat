@pushd %~dp0
@call ..\..\opsc\opsc.bat -g ALL -B ALL -o Generated *.idl -p opsbridge
@if errorlevel 1 goto :error
@popd
@goto :eof

:error
@echo ERROR DETECTED
@pause
@popd
