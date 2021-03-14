
set(OPS_LIB_NAME ops-csharp)

get_target_property(OPSIDLS_GENERATED_PATH opsidls GENERATED_PATH)

set(OPSIDLS_GEN_PATH ${OPSIDLS_GENERATED_PATH}/CSharp/opsidls)
set(OPSIDLS_SOURCE
  ${OPSIDLS_GEN_PATH}/DebugRequestResponseData.cs
  ${OPSIDLS_GEN_PATH}/OPSConstants.cs
  ${OPSIDLS_GEN_PATH}/opsidlsTypeFactory.cs
  ${OPSIDLS_GEN_PATH}/SendAckPatternData.cs
)

set_source_files_properties(${OPSIDLS_SOURCE} PROPERTIES GENERATED TRUE)

set(OPS_CS_SOURCE
  ${OPSIDLS_SOURCE}
  AckData.cs
  BlockingBuffer.cs
  Channel.cs
  DeadlineMissedFilterQoSPolicy.cs
  DeadlineNotifier.cs
  DefaultOPSConfigImpl.cs
  DefaultSubscriberFilterQoSPolicy.cs
  Domain.cs
  Error.cs
  Event.cs
  Exceptions.cs
  FilterQoSPolicy.cs
  Globals.cs
  InetAddress.cs
  InProcessTransport.cs
  KeyFilterQoSPolicy.cs
  Listener.cs
  Locals.cs
  McSendDataHandler.cs
  McUdpSendDataHandler.cs
  MessageFilter.cs
  MessageFilterSet.cs
  MulticastReceiver.cs
  MulticastSender.cs
  Notifier.cs
  OPSConfig.cs
  OPSConfigRepository.cs
  OPSMessage.cs
  OPSObject.cs
  Participant.cs
  ParticipantFactory.cs
  ParticipantInfoData.cs
  ParticipantInfoDataListener.cs
  Publisher.cs
  QualityOfService.cs
  QualityOfServicePolicy.cs
  ReadByteBuffer.cs
  ReceiveDataHandler.cs
  ReceiveDataHandlerFactory.cs
  Receiver.cs
  ReceiverFactory.cs
  ReliabilityQoS.cs
  Reply.cs
  Request.cs
  RequestReply.cs
  SendDataHandler.cs
  SendDataHandlerFactory.cs
  Sender.cs
  Subscriber.cs
  SubscriberFilterQoSPolicy.cs
  TcpClient.cs
  TcpClientReceiver.cs
  TcpSendDataHandler.cs
  TcpSenderList.cs
  TcpServerSender.cs
  Topic.cs
  TopicInfoData.cs
  Transport.cs
  UdpReceiver.cs
  UdpSender.cs
  WriteByteBuffer.cs
  Archiver/ArchiverInOut.cs
  Archiver/FormatException.cs
  Archiver/OPSArchiverIn.cs
  Archiver/OPSArchiverOut.cs
  Archiver/OPSObjectFactory.cs
  Archiver/SerializableCompositeFactory.cs
  Archiver/Serializable.cs
  Archiver/SerializableFactory.cs
  Archiver/XMLArchiverIn.cs
  Archiver/XMLArchiverOut.cs
  Utilities/EasyThread.cs
  Utilities/EmailLogger.cs
  Utilities/EventLogLogger.cs
  Utilities/ExceptionLogger.cs
  Utilities/Logger.cs
  Utilities/ObserverPattern.cs
  Utilities/TextFileLogger.cs
  Utilities/WebsiteLogger.cs
  Utilities/WindowLogger.cs
)

if(WIN32)
  if(${CMAKE_VERSION} VERSION_GREATER "3.8.0")
    ## message(">>>>> CMAKE_VERSION= " ${CMAKE_VERSION})
    enable_language(CSharp)

    # Define the DLL target, including all relevant project files.
    add_library(${OPS_LIB_NAME} SHARED ${OPS_CS_SOURCE})

    add_dependencies(${OPS_LIB_NAME} opsidls)

    # Set the .NET Framework version for the target.
    set_property(TARGET ${OPS_LIB_NAME} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
    set_property(TARGET ${OPS_LIB_NAME} PROPERTY WIN32_EXECUTABLE FALSE)
    set_property(TARGET ${OPS_LIB_NAME} PROPERTY VS_CONFIGURATION_TYPE ClassLibrary)
    # Set the C# language version (defaults to 3.0 if not set).
    set(CMAKE_CSharp_FLAGS "/langversion:7.3")
    # Add in some .NET reference libraries.
    set_property(TARGET ${OPS_LIB_NAME} PROPERTY VS_DOTNET_REFERENCES
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
    set_target_properties(${OPS_LIB_NAME} PROPERTIES DEBUG_POSTFIX "-dbg")
    set_target_properties(${OPS_LIB_NAME} PROPERTIES RELEASE_POSTFIX "-opt")

    install(TARGETS ${OPS_LIB_NAME}
      EXPORT ops-targets
      DESTINATION bin
    )

  else()

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
      set(OPS_TARGET ${CMAKE_BINARY_DIR}/bin/${OPS_LIB_NAME}-${OPS_VERSION}-dbg.dll)
      set(OPS_CSHARP_FLAGS /debug)
    else()
      set(OPS_TARGET ${CMAKE_BINARY_DIR}/bin/${OPS_LIB_NAME}-${OPS_VERSION}-opt.dll)
      set(OPS_CSHARP_FLAGS /optimize)
    endif()

    get_filename_component(CSCOMPILER_PATH ${CSCOMPILER} DIRECTORY)
    get_filename_component(CXXCOMPILER_PATH ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(CXXCOMPILER_PATH ${CXXCOMPILER_PATH} DIRECTORY)
    if(CMAKE_CL_64)
      get_filename_component(CXXCOMPILER_PATH ${CXXCOMPILER_PATH} DIRECTORY)
    endif()

    # Locate path where the CSharp runtime and support assemblies are
    find_path(CSREFERENCE_PATH System.Data.dll HINTS ${CSCOMPILER_PATH} )
    file(TO_NATIVE_PATH ${CSREFERENCE_PATH} CSREFERENCE_NATIVEPATH)

    # Locate vsvars.bat script, it is required for cmd-line building
    find_file(VCVARSALL vcvarsall.bat HINTS ${CXXCOMPILER_PATH} )
    file(TO_NATIVE_PATH ${VCVARSALL} VCVARSALL_NATIVEPATH)
    if(NOT VCVARSALL)
      message("Failed to find vcvarsall.bat -- please locate it yourself")
      return()
    endif()

    if(CMAKE_CL_64)
      set(VSVARSALL_ARCHITECTURE x64)
    else()
      set(VSVARSALL_ARCHITECTURE x86)
    endif()

    # convert files to native file paths, i.e. with '\'
    foreach(csfile ${OPS_CS_SOURCE})
      file(TO_NATIVE_PATH ${csfile} CSNATIVEFILE)
      list(APPEND OPS_CS_SOURCE_LIST ${CSNATIVEFILE})
    endforeach()

    add_custom_command(OUTPUT ${OPS_TARGET}
      #COMMAND "${VS80COMNTOOLS}vsvars32.bat x86"
      COMMAND "${VCVARSALL}" ${VSVARSALL_ARCHITECTURE}
      COMMAND ${CSCOMPILER} /noconfig ${OPS_CSHARP_FLAGS} -langversion:7.3 /nowarn:1701,1702 /warn:4 /define:TRACE /lib:${CSREFERENCE_NATIVEPATH} /reference:System.dll,System.Drawing.dll,System.Windows.Forms.dll,System.Xml.dll,System.Linq.dll,System.Web.dll,System.Web.Services.dll /target:library /out:${OPS_TARGET} ${OPS_CS_SOURCE_LIST}
      DEPENDS ${OPS_CS_SOURCE}
      COMMENT "Building C# sources using ${CSCOMPILER}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
    add_custom_target(${OPS_LIB_NAME} ALL DEPENDS ${OPS_TARGET})

    install(FILES ${OPS_TARGET}
      EXPORT ops-targets
      DESTINATION lib/mono
    )

  endif()

else()

  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(OPS_TARGET ${CMAKE_BINARY_DIR}/bin/${OPS_LIB_NAME}-${OPS_VERSION}-dbg.dll)
  else()
    set(OPS_TARGET ${CMAKE_BINARY_DIR}/bin/${OPS_LIB_NAME}-${OPS_VERSION}-opt.dll)
  endif()

  # removed System.Net.dll for Microsoft.Net v2.0.xyz
  add_custom_command(OUTPUT ${OPS_TARGET}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CSCOMPILER} -langversion:7.3 -reference:System.dll,System.Drawing.dll,System.Windows.Forms.dll,System.Web.dll,System.Web.Services.dll -target:library -out:${OPS_TARGET} ${OPS_CS_SOURCE}
    DEPENDS ${OPS_CS_SOURCE}
    COMMENT "Building C# sources using ${CSCOMPILER}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
  add_custom_target(${OPS_LIB_NAME} ALL DEPENDS ${OPS_TARGET})

  install(FILES ${OPS_TARGET} DESTINATION bin)
endif()
