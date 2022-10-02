
if (NOT DEFINED OPS4_CPP_ROOT)
  message ("  !!!  " FATAL " " ERROR " OPS4_CPP_ROOT must be defined")
endif()

# where are the headers
set(OPS_HEADER_PATH ${OPS4_CPP_ROOT}/include)

set(LIB_OPS_PUBLIC_HEADERS
  ${OPS_HEADER_PATH}/ArchiverInOut.h
  ${OPS_HEADER_PATH}/ByteBuffer.h
  ${OPS_HEADER_PATH}/BytesSizePair.h
  ${OPS_HEADER_PATH}/Channel.h
  ${OPS_HEADER_PATH}/ChecksumArchiver.h
  ${OPS_HEADER_PATH}/CommException.h
  ${OPS_HEADER_PATH}/ConnectStatus.h
  ${OPS_HEADER_PATH}/ConfigException.h
  ${OPS_HEADER_PATH}/DataListener.h
  ${OPS_HEADER_PATH}/DataNotifier.h
  ${OPS_HEADER_PATH}/DeadlineMissedListener.h
  ${OPS_HEADER_PATH}/DeadlineTimer.h
  ${OPS_HEADER_PATH}/DebugHandler.h
  ${OPS_HEADER_PATH}/Domain.h
  ${OPS_HEADER_PATH}/Error.h
  ${OPS_HEADER_PATH}/ErrorService.h
  ${OPS_HEADER_PATH}/ErrorWriter.h
  ${OPS_HEADER_PATH}/FilterQoSPolicy.h
  ${OPS_HEADER_PATH}/fixed_string.h
  ${OPS_HEADER_PATH}/fixed_string_support.h
  ${OPS_HEADER_PATH}/IOService.h
  ${OPS_HEADER_PATH}/KeyFilterQoSPolicy.h
  ${OPS_HEADER_PATH}/Listener.h
  ${OPS_HEADER_PATH}/Lockable.h
  ${OPS_HEADER_PATH}/memory_pool.h
  ${OPS_HEADER_PATH}/MemoryMap.h
  ${OPS_HEADER_PATH}/NetworkSupport.h
  ${OPS_HEADER_PATH}/NoSuchTopicException.h
  ${OPS_HEADER_PATH}/Notifier.h
  ${OPS_HEADER_PATH}/ops/Reply.h
  ${OPS_HEADER_PATH}/ops/Request.h
  ${OPS_HEADER_PATH}/OPSArchiverIn.h
  ${OPS_HEADER_PATH}/OPSArchiverOut.h
  ${OPS_HEADER_PATH}/OPSConfig.h
  ${OPS_HEADER_PATH}/OPSConfigRepository.h
  ${OPS_HEADER_PATH}/ops.h
  ${OPS_HEADER_PATH}/OPSEvent.h
  ${OPS_HEADER_PATH}/OPSExport.h
  ${OPS_HEADER_PATH}/OPSMessage.h
  ${OPS_HEADER_PATH}/OPSObjectFactory.h
  ${OPS_HEADER_PATH}/OPSObject.h
  ${OPS_HEADER_PATH}/OPSStringLengths.h
  ${OPS_HEADER_PATH}/OPSTypeDefs.h
  ${OPS_HEADER_PATH}/OPSUtilities.h
  ${OPS_HEADER_PATH}/Participant.h
  ${OPS_HEADER_PATH}/ParticipantInfoData.h
  ${OPS_HEADER_PATH}/ParticipantInfoDataListener.h
  ${OPS_HEADER_PATH}/PrintArchiverOut.h
  ${OPS_HEADER_PATH}/PubIdChecker.h
  ${OPS_HEADER_PATH}/Publisher.h
  ${OPS_HEADER_PATH}/ReceiveDataChannel.h
  ${OPS_HEADER_PATH}/ReceiveDataHandler.h
  ${OPS_HEADER_PATH}/Receiver.h
  ${OPS_HEADER_PATH}/ReferenceHandler.h
  ${OPS_HEADER_PATH}/RequestReply.h
  ${OPS_HEADER_PATH}/Reservable.h
  ${OPS_HEADER_PATH}/ReserveInfo.h
  ${OPS_HEADER_PATH}/Runnable.h
  ${OPS_HEADER_PATH}/SendDataHandler.h
  ${OPS_HEADER_PATH}/Sender.h
  ${OPS_HEADER_PATH}/SerializableCompositeFactory.h
  ${OPS_HEADER_PATH}/SerializableFactory.h
  ${OPS_HEADER_PATH}/SerializableInheritingTypeFactory.h
  ${OPS_HEADER_PATH}/Serializable.h
  ${OPS_HEADER_PATH}/SingleThreadPool.h
  ${OPS_HEADER_PATH}/Subscriber.h
  ${OPS_HEADER_PATH}/Thread.h
  ${OPS_HEADER_PATH}/ThreadPool.h
  ${OPS_HEADER_PATH}/TimeHelper.h
  ${OPS_HEADER_PATH}/Topic.h
  ${OPS_HEADER_PATH}/TopicInfoData.h
  ${OPS_HEADER_PATH}/Trace.h
  ${OPS_HEADER_PATH}/TraceStream.h
  ${OPS_HEADER_PATH}/Transport.h
  ${OPS_HEADER_PATH}/XMLArchiverIn.h
  ${OPS_HEADER_PATH}/XMLArchiverOut.h
)

set(OPS_SRC_PATH ${OPS4_CPP_ROOT}/source)

set(LIB_OPS_CORE_SRCS
  ${OPS_SRC_PATH}/ArchiverInOut.cpp
  ${OPS_SRC_PATH}/BasicError.h
  ${OPS_SRC_PATH}/BasicWarning.h
  ${OPS_SRC_PATH}/ByteBuffer.cpp
  ${OPS_SRC_PATH}/Channel.cpp
  ${OPS_SRC_PATH}/DataNotifier.cpp
  ${OPS_SRC_PATH}/DataSegmentPool.cpp
  ${OPS_SRC_PATH}/DataSegmentPool.h
  ${OPS_SRC_PATH}/DebugHandler.cpp
  ${OPS_SRC_PATH}/DefaultOPSConfigImpl.h
  ${OPS_SRC_PATH}/Domain.cpp
  ${OPS_SRC_PATH}/ErrorService.cpp
  ${OPS_SRC_PATH}/KeyFilterQoSPolicy.cpp
  ${OPS_SRC_PATH}/Lockable.cpp
  ${OPS_SRC_PATH}/MCReceiveDataHandler.h
  ${OPS_SRC_PATH}/MCReceiveDataHandler.cpp
  ${OPS_SRC_PATH}/McSendDataHandler.h
  ${OPS_SRC_PATH}/McUdpSendDataHandler.h
  ${OPS_SRC_PATH}/memory_pool.cpp
  ${OPS_SRC_PATH}/MultiThreadPool.h
  ${OPS_SRC_PATH}/OPSArchiverIn.cpp
  ${OPS_SRC_PATH}/OPSArchiverOut.cpp
  ${OPS_SRC_PATH}/OPSConfig.cpp
  ${OPS_SRC_PATH}/OPSConfigRepository.cpp
  ${OPS_SRC_PATH}/OPSEvent.cpp
  ${OPS_SRC_PATH}/OPSObject.cpp
  ${OPS_SRC_PATH}/OPSObjectFactory.cpp
  ${OPS_SRC_PATH}/OPSObjectFactoryImpl.h
  ${OPS_SRC_PATH}/Participant.cpp
  ${OPS_SRC_PATH}/ParticipantInfoDataListener.cpp
  ${OPS_SRC_PATH}/Publisher.cpp
  ${OPS_SRC_PATH}/ReceiveDataChannel.cpp
  ${OPS_SRC_PATH}/ReceiveDataHandler.cpp
  ${OPS_SRC_PATH}/ReceiveDataHandlerFactory.cpp
  ${OPS_SRC_PATH}/ReceiveDataHandlerFactory.h
  ${OPS_SRC_PATH}/ReceiverFactory.cpp
  ${OPS_SRC_PATH}/ReceiverFactory.h
  ${OPS_SRC_PATH}/RequestReply.cpp
  ${OPS_SRC_PATH}/Reservable.cpp
  ${OPS_SRC_PATH}/SendDataHandlerFactory.cpp
  ${OPS_SRC_PATH}/SendDataHandlerFactory.h
  ${OPS_SRC_PATH}/SerializableCompositeFactory.cpp
  ${OPS_SRC_PATH}/SerializableInheritingTypeFactory.cpp
  ${OPS_SRC_PATH}/Subscriber.cpp
  ${OPS_SRC_PATH}/TCPClientBase.h
  ${OPS_SRC_PATH}/TCPConnection.h
  ${OPS_SRC_PATH}/TCPOpsProtocol.h
  ${OPS_SRC_PATH}/TCPProtocol.h
  ${OPS_SRC_PATH}/TCPReceiveDataChannel.cpp
  ${OPS_SRC_PATH}/TCPReceiveDataChannel.h
  ${OPS_SRC_PATH}/TCPReceiveDataHandler.cpp
  ${OPS_SRC_PATH}/TCPReceiveDataHandler.h
  ${OPS_SRC_PATH}/TCPSendDataHandler.h
  ${OPS_SRC_PATH}/TCPServerBase.h
  ${OPS_SRC_PATH}/Thread.cpp
  ${OPS_SRC_PATH}/ThreadSupport.h
  ${OPS_SRC_PATH}/Topic.cpp
  ${OPS_SRC_PATH}/Trace.cpp
  ${OPS_SRC_PATH}/Transport.cpp
  ${OPS_SRC_PATH}/UDPReceiveDataHandler.cpp
  ${OPS_SRC_PATH}/UDPReceiveDataHandler.h
  ${OPS_SRC_PATH}/XMLArchiverIn.cpp
  ${OPS_SRC_PATH}/XMLArchiverOut.cpp
)
