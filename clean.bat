
@SETLOCAL

@rem define build directories if not already defined
@IF NOT DEFINED OPS_BUILD_BOOTSTRAP_DIR set OPS_BUILD_BOOTSTRAP_DIR=%~dp0\build.bootstrap
@IF NOT DEFINED OPS_BUILD_DBG_DIR set OPS_BUILD_DBG_DIR=%~dp0\build.debug
@IF NOT DEFINED OPS_BUILD_OPT_DIR set OPS_BUILD_OPT_DIR=%~dp0\build.opt
@IF NOT DEFINED OPS_INSTALL_PREFIX set OPS_INSTALL_PREFIX=%~dp0\deploy

@echo Using OPS_BUILD_BOOTSTRAP_DIR = %OPS_BUILD_BOOTSTRAP_DIR%
@echo Using OPS_BUILD_DBG_DIR = %OPS_BUILD_DBG_DIR%
@echo Using OPS_BUILD_OPT_DIR = %OPS_BUILD_OPT_DIR%
@echo Using OPS_INSTALL_PREFIX = %OPS_INSTALL_PREFIX%

rmdir /S /Q %OPS_BUILD_BOOTSTRAP_DIR%
rmdir /S /Q %OPS_BUILD_DBG_DIR%
rmdir /S /Q %OPS_BUILD_OPT_DIR%
rmdir /S /Q %OPS_INSTALL_PREFIX%

rmdir /S /Q %~dp0\Common\idl\Generated
rmdir /S /Q %~dp0\Tools\OPSBridge\idl\Generated

del %~dp0\Cpp\include\OPSStringLengths.h
