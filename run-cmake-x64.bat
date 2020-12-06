@rem NOTE requires CMAKE and a MSBUILD command shell to work

@SETLOCAL

@rem define build directories if not already defined
@IF NOT DEFINED OPS_BUILD_BOOTSTRAP_DIR set OPS_BUILD_BOOTSTRAP_DIR=%~dp0\build.bootstrap-x64
@IF NOT DEFINED OPS_BUILD_DBG_DIR set OPS_BUILD_DBG_DIR=%~dp0\build.debug-x64
@IF NOT DEFINED OPS_BUILD_OPT_DIR set OPS_BUILD_OPT_DIR=%~dp0\build.opt-x64
@IF NOT DEFINED OPS_INSTALL_PREFIX set OPS_INSTALL_PREFIX=%~dp0\deploy-x64

@echo Using OPS_BUILD_BOOTSTRAP_DIR = %OPS_BUILD_BOOTSTRAP_DIR%
@echo Using OPS_BUILD_DBG_DIR = %OPS_BUILD_DBG_DIR%
@echo Using OPS_BUILD_OPT_DIR = %OPS_BUILD_OPT_DIR%
@echo Using OPS_INSTALL_PREFIX = %OPS_INSTALL_PREFIX%

@rem Perform the bootstrap process that generates source files needed by the other targets
@pushd %~dp0
@IF NOT EXIST %OPS_BUILD_BOOTSTRAP_DIR% mkdir %OPS_BUILD_BOOTSTRAP_DIR%
@cd %OPS_BUILD_BOOTSTRAP_DIR%
cmake -DCMAKE_BUILD_TYPE=Bootstrap -DCMAKE_INSTALL_PREFIX=%OPS_INSTALL_PREFIX% -A x64 %~dp0
@IF ERRORLEVEL 1 goto :BUILD_FAILED
cmake --build . --target ALL_BUILD --config Debug
@IF ERRORLEVEL 1 goto :BUILD_FAILED
cmake -DBUILD_TYPE=Bootstrap -DCMAKE_INSTALL_PREFIX=%OPS_INSTALL_PREFIX% -P cmake_install.cmake
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@popd

@rem build and install Debug
@pushd %~dp0
@IF NOT EXIST %OPS_BUILD_DBG_DIR% mkdir %OPS_BUILD_DBG_DIR%
@cd %OPS_BUILD_DBG_DIR%
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%OPS_INSTALL_PREFIX% -DOPS_BUILD_UNITTESTS=NO -A x64 %~dp0
@IF ERRORLEVEL 1 goto :BUILD_FAILED
cmake --build . --target ALL_BUILD --config Debug
@IF ERRORLEVEL 1 goto :BUILD_FAILED
cmake -DBUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%OPS_INSTALL_PREFIX% -P cmake_install.cmake
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@popd

@rem build and install Release
@pushd %~dp0
@IF NOT EXIST %OPS_BUILD_OPT_DIR% mkdir %OPS_BUILD_OPT_DIR%
@cd %OPS_BUILD_OPT_DIR%
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%OPS_INSTALL_PREFIX% -A x64 %~dp0
@IF ERRORLEVEL 1 goto :BUILD_FAILED
cmake --build . --target ALL_BUILD --config Release
@IF ERRORLEVEL 1 goto :BUILD_FAILED
cmake -DBUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%OPS_INSTALL_PREFIX% -P cmake_install.cmake
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@popd

@echo ------ BUILD OK ------
@goto :EOF

:BUILD_FAILED
@echo ------ BUILD FAILED ------
