
set(PROG_NAME testall-csharp)
set(CONSOLE_NAME testall-csharp-console)

set(IDLS_GEN_PATH ${PROJECT_SOURCE_DIR}/Examples/OPSIdls/TestAll/Generated/CSharp)
set(IDLS_SOURCE
  ${IDLS_GEN_PATH}/TestAll/BaseData.cs
  ${IDLS_GEN_PATH}/TestAll/ChildData.cs
  ${IDLS_GEN_PATH}/TestAll/Definitions.cs
  ${IDLS_GEN_PATH}/TestAll/Fruit.cs
  ${IDLS_GEN_PATH}/TestAll/NoData.cs
  ${IDLS_GEN_PATH}/TestAll/TestAllTypeFactory.cs
  ${IDLS_GEN_PATH}/TestAll/TestData.cs
)

set_source_files_properties(${IDLS_SOURCE} PROPERTIES GENERATED TRUE)

set(PROG_CS_SOURCE
  ${IDLS_SOURCE}
  TestCode.cs
  Form1.cs
  Form1.Designer.cs
  Form1.resx
  Program.cs
  TestAll.csproj
)
set(CONSOLE_CS_SOURCE
  ${IDLS_SOURCE}
  TestCode.cs
  VerifySerDes.cs
)

if(WIN32)
  if(${CMAKE_VERSION} VERSION_GREATER "3.8.0")
    ## message(">>>>> CMAKE_VERSION= " ${CMAKE_VERSION})
    enable_language(CSharp)

    # Define the DLL target, including all relevant project files.
    add_executable(${PROG_NAME} ${PROG_CS_SOURCE})
    add_executable(${CONSOLE_NAME} ${CONSOLE_CS_SOURCE})

    add_dependencies(${PROG_NAME} TestAll)
    add_dependencies(${CONSOLE_NAME} TestAll)

    # Set the .NET Framework version for the target.
    set_property(TARGET ${PROG_NAME} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION ${OPS_DOT_NET_Framework_version})
    set_property(TARGET ${CONSOLE_NAME} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION ${OPS_DOT_NET_Framework_version})

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

    set_property(TARGET ${CONSOLE_NAME} PROPERTY VS_DOTNET_REFERENCES
      "Microsoft.CSharp"
      "System"
      "System.Core"
      "System.Data"
      "System.Deployment"
      "System.Drawing"
      "System.Xml"
    )
    set_target_properties(${CONSOLE_NAME} PROPERTIES DEBUG_POSTFIX "-dbg")
    set_target_properties(${CONSOLE_NAME} PROPERTIES RELEASE_POSTFIX "-opt")
    target_link_libraries(${CONSOLE_NAME} PUBLIC ops-csharp)
    add_dependencies(${CONSOLE_NAME} ops-csharp)

    install(TARGETS ${PROG_NAME} ${CONSOLE_NAME} ops-csharp DESTINATION examples/testall)

  endif()
endif()
