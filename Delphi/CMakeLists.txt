
## example: "4.3.3" --> OPS_VERSION_NUMBER = 40303
math(EXPR OPS_VERSION_NUMBER "(10000 * ${OPS_VERSION_MAJOR}) + (100 * ${OPS_VERSION_MINOR}) + ${OPS_VERSION_PATCH}")

## Configure Delphi file with the constant values
configure_file(uOps.Version.pas.in ${CMAKE_CURRENT_SOURCE_DIR}/Source/uOps.Version.pas)
