@pushd %~dp0
@echo Building IDL Templates ...
@IF NOT EXIST dist (
	mkdir dist
)

@jar cf "dist/IDLTemplates.jar" -C "src/ops/netbeansmodules/idlsupport" templates
@IF ERRORLEVEL 1 goto :BUILD_FAILED
@goto :BUILD_OK

:BUILD_FAILED
@echo ------ BUILD FAILED ------
@pause

:BUILD_OK
@popd
