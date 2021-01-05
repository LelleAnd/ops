@pushd %~dp0
@echo Building needed libraries ...
@call ..\Libs\ConfigurationLib\Build_Lib.bat
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@call ..\Java\OPSJLib\Build_Lib.bat
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@call ..\Tools\OPSCompilerLib\Build_Lib.bat
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@call ..\Tools\IDLParser\Build_Lib.bat
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@call ..\Tools\NBOPSIDLSupport\Build_Lib.bat
@IF ERRORLEVEL 1 goto :BUILD_FAILED

@echo Building IDL Command-Line Compiler ...
@call ..\Tools\opsc\Build_opsc.bat
@IF ERRORLEVEL 1 goto :BUILD_FAILED

:BUILD_FAILED
@popd
