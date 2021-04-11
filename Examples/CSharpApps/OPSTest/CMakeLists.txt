
set(PROG_NAME opstest-csharp)

set(IDLS_GEN_PATH ${PROJECT_SOURCE_DIR}/Examples/OPSIdls/PizzaProject/Generated/CSharp)
set(IDLS_SOURCE
  ${IDLS_GEN_PATH}/pizza/special/Cheese.cs
  ${IDLS_GEN_PATH}/pizza/special/ExtraAllt.cs
  ${IDLS_GEN_PATH}/pizza/special/LHCData.cs
  ${IDLS_GEN_PATH}/pizza/special/Parmesan.cs
  ${IDLS_GEN_PATH}/pizza/CapricosaData.cs
  ${IDLS_GEN_PATH}/pizza/PizzaData.cs
  ${IDLS_GEN_PATH}/pizza/VessuvioData.cs
  ${IDLS_GEN_PATH}/PizzaProject/PizzaProjectTypeFactory.cs
  ${IDLS_GEN_PATH}/sds/MessageHeaderData.cs
)

set_source_files_properties(${IDLS_SOURCE} PROPERTIES GENERATED TRUE)

set(PROG_CS_SOURCE
  ${IDLS_SOURCE}
  OPSTest/Form1.cs
  OPSTest/Form1.Designer.cs
  OPSTest/Form1.resx
  OPSTest/OPSTest.csproj
  OPSTest/Program.cs
)

if(WIN32)
  if(${CMAKE_VERSION} VERSION_GREATER "3.8.0")
    ## message(">>>>> CMAKE_VERSION= " ${CMAKE_VERSION})
    enable_language(CSharp)

    # Define the DLL target, including all relevant project files.
    add_executable(${PROG_NAME} ${PROG_CS_SOURCE})
    add_dependencies(${PROG_NAME} PizzaProject)

    # Set the .NET Framework version for the target.
    set_property(TARGET ${PROG_NAME} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
    set_property(TARGET ${PROG_NAME} PROPERTY WIN32_EXECUTABLE FALSE)
    set_property(TARGET ${PROG_NAME} PROPERTY VS_CONFIGURATION_TYPE ClassLibrary)
    # Set the C# language version (defaults to 3.0 if not set).
    set(CMAKE_CSharp_FLAGS "/langversion:7.3")
    # Add in some .NET reference libraries.
    set_property(TARGET ${PROG_NAME} PROPERTY VS_DOTNET_REFERENCES
      "Microsoft.CSharp"
      "System"
      "System.Core"
      "System.Data"
      "System.Deployment"
      "System.Drawing"
      "System.Net.Http"
      "System.Xml"
      "System.Windows.Forms"
      "System.Web"
    )
    set_target_properties(${PROG_NAME} PROPERTIES DEBUG_POSTFIX "-dbg")
    set_target_properties(${PROG_NAME} PROPERTIES RELEASE_POSTFIX "-opt")

    target_link_libraries(${PROG_NAME} PUBLIC ops-csharp)
    add_dependencies(${PROG_NAME} ops-csharp)

    install(TARGETS ${PROG_NAME} ops-csharp DESTINATION examples/pizzatest)

  endif()
endif()
