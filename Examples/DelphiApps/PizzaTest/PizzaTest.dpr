program PizzaTest;

{$WARN SYMBOL_PLATFORM OFF}

uses
  Vcl.Forms,
  uPizzaTest in 'uPizzaTest.pas' {Form1},
  uLogger in '..\..\..\Delphi\Common\uLogger.pas',
  uNotifier in '..\..\..\Delphi\Common\uNotifier.pas',
  uRunner in '..\..\..\Delphi\Common\uRunner.pas',
  uOps.ArchiverInOut in '..\..\..\Delphi\Source\uOps.ArchiverInOut.pas',
  uOps.ByteBuffer in '..\..\..\Delphi\Source\uOps.ByteBuffer.pas',
  uOps.DeadlineTimer in '..\..\..\Delphi\Source\uOps.DeadlineTimer.pas',
  uOps.Domain in '..\..\..\Delphi\Source\uOps.Domain.pas',
  uOps.Error in '..\..\..\Delphi\Source\uOps.Error.pas',
  uOps.Exceptions in '..\..\..\Delphi\Source\uOps.Exceptions.pas',
  uOps.FilterQoSPolicy in '..\..\..\Delphi\Source\uOps.FilterQoSPolicy.pas',
  uOps.KeyFilterQoSPolicy in '..\..\..\Delphi\Source\uOps.KeyFilterQoSPolicy.pas',
  uOps.MemoryMap in '..\..\..\Delphi\Source\uOps.MemoryMap.pas',
  uOps.OpsArchiverIn in '..\..\..\Delphi\Source\uOps.OpsArchiverIn.pas',
  uOps.OpsArchiverOut in '..\..\..\Delphi\Source\uOps.OpsArchiverOut.pas',
  uOps.OPSConfig in '..\..\..\Delphi\Source\uOps.OPSConfig.pas',
  uOps.OpsMessage in '..\..\..\Delphi\Source\uOps.OpsMessage.pas',
  uOps.OpsObject in '..\..\..\Delphi\Source\uOps.OpsObject.pas',
  uOps.OPSObjectFactory in '..\..\..\Delphi\Source\uOps.OPSObjectFactory.pas',
  uOps.Participant in '..\..\..\Delphi\Source\uOps.Participant.pas',
  uOps.ParticipantInfoData in '..\..\..\Delphi\Source\uOps.ParticipantInfoData.pas',
  uOps.ParticipantInfoDataListener in '..\..\..\Delphi\Source\uOps.ParticipantInfoDataListener.pas',
  uOps.Publisher in '..\..\..\Delphi\Source\uOps.Publisher.pas',
  uOps.PublisherAbs in '..\..\..\Delphi\Source\uOps.PublisherAbs.pas',
  uOps.SerializableCompositeFactory in '..\..\..\Delphi\Source\uOps.SerializableCompositeFactory.pas',
  uOps.SerializableFactory in '..\..\..\Delphi\Source\uOps.SerializableFactory.pas',
  uOps.SerializableInheritingTypeFactory in '..\..\..\Delphi\Source\uOps.SerializableInheritingTypeFactory.pas',
  uOps.Subscriber in '..\..\..\Delphi\Source\uOps.Subscriber.pas',
  uOps.TimeHelper in '..\..\..\Delphi\Source\uOps.TimeHelper.pas',
  uOps.Topic in '..\..\..\Delphi\Source\uOps.Topic.pas',
  uOps.TopicInfoData in '..\..\..\Delphi\Source\uOps.TopicInfoData.pas',
  uOps.Types in '..\..\..\Delphi\Source\uOps.Types.pas',
  uOps.XMLArchiverIn in '..\..\..\Delphi\Source\uOps.XMLArchiverIn.pas',
  uOps.Transport.McSendDataHandler in '..\..\..\Delphi\Source\Transport\uOps.Transport.McSendDataHandler.pas',
  uOps.Transport.McUdpSendDataHandler in '..\..\..\Delphi\Source\Transport\uOps.Transport.McUdpSendDataHandler.pas',
  uOps.Transport.MulticastReceiver in '..\..\..\Delphi\Source\Transport\uOps.Transport.MulticastReceiver.pas',
  uOps.Transport.ReceiveDataHandler in '..\..\..\Delphi\Source\Transport\uOps.Transport.ReceiveDataHandler.pas',
  uOps.Transport.ReceiveDataHandlerFactory in '..\..\..\Delphi\Source\Transport\uOps.Transport.ReceiveDataHandlerFactory.pas',
  uOps.Transport.Receiver in '..\..\..\Delphi\Source\Transport\uOps.Transport.Receiver.pas',
  uOps.Transport.SendDataHandler in '..\..\..\Delphi\Source\Transport\uOps.Transport.SendDataHandler.pas',
  uOps.Transport.SendDataHandlerFactory in '..\..\..\Delphi\Source\Transport\uOps.Transport.SendDataHandlerFactory.pas',
  uOps.Transport.Sender in '..\..\..\Delphi\Source\Transport\uOps.Transport.Sender.pas',
  uOps.Transport.TCPClient in '..\..\..\Delphi\Source\Transport\uOps.Transport.TCPClient.pas',
  uOPs.Transport.TCPSendDataHandler in '..\..\..\Delphi\Source\Transport\uOPs.Transport.TCPSendDataHandler.pas',
  uOps.Transport.TCPServer in '..\..\..\Delphi\Source\Transport\uOps.Transport.TCPServer.pas',
  uOps.Transport.UDPReceiver in '..\..\..\Delphi\Source\Transport\uOps.Transport.UDPReceiver.pas',
  uOps.Transport.UDPSender in '..\..\..\Delphi\Source\Transport\uOps.Transport.UDPSender.pas',
  pizza.CapricosaData in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\pizza.CapricosaData.pas',
  pizza.PizzaData in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\pizza.PizzaData.pas',
  pizza.VessuvioData in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\pizza.VessuvioData.pas',
  pizza.special.Cheese in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\special\pizza.special.Cheese.pas',
  pizza.special.ExtraAllt in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\special\pizza.special.ExtraAllt.pas',
  pizza.special.LHCData in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\special\pizza.special.LHCData.pas',
  PizzaProject.PizzaProjectTypeFactory in '..\..\OPSIdls\PizzaProject\Generated\Delphi\PizzaProject\PizzaProject.PizzaProjectTypeFactory.pas',
  uOps.XMLArchiverOut in '..\..\..\Delphi\Source\uOps.XMLArchiverOut.pas',
  uOps.RequestReply in '..\..\..\Delphi\Source\RequestReply\uOps.RequestReply.pas',
  uOps.RequestReply.Reply in '..\..\..\Delphi\Source\RequestReply\uOps.RequestReply.Reply.pas',
  uOps.RequestReply.Request in '..\..\..\Delphi\Source\RequestReply\uOps.RequestReply.Request.pas',
  uOps.PrintArchiverOut in '..\..\..\Delphi\Source\uOps.PrintArchiverOut.pas',
  uOps.OPSConfigRepository in '..\..\..\Delphi\Source\uOps.OPSConfigRepository.pas',
  uOps.Utilities in '..\..\..\Delphi\Source\uOps.Utilities.pas',
  uSockets in '..\..\..\Delphi\Common\uSockets.pas',
  uOps.Channel in '..\..\..\Delphi\Source\uOps.Channel.pas',
  uOps.Transport in '..\..\..\Delphi\Source\uOps.Transport.pas',
  sds.MessageHeaderData in '..\..\OPSIdls\PizzaProject\Generated\Delphi\sds\sds.MessageHeaderData.pas',
  uOps.NetworkSupport in '..\..\..\Delphi\Source\uOps.NetworkSupport.pas',
  pizza.special.Parmesan in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\special\pizza.special.Parmesan.pas',
  opsidls.OPSConstants in '..\..\..\Common\idl\Generated\Delphi\opsidls\opsidls.OPSConstants.pas',
  opsidls.SendAckPatternData in '..\..\..\Common\idl\Generated\Delphi\opsidls\opsidls.SendAckPatternData.pas',
  opsidls.DebugRequestResponseData in '..\..\..\Common\idl\Generated\Delphi\opsidls\opsidls.DebugRequestResponseData.pas',
  opsidls.opsidlsTypeFactory in '..\..\..\Common\idl\Generated\Delphi\opsidls\opsidls.opsidlsTypeFactory.pas',
  pizza.extra.Secret in '..\..\OPSIdls\PizzaProject\Generated\Delphi\pizza\extra\pizza.extra.Secret.pas',
  uOps.Transport.ReceiveDataChannel in '..\..\..\Delphi\Source\Transport\uOps.Transport.ReceiveDataChannel.pas';

{$R *.res}

begin
{$IFDEF WIN32}
  ReportMemoryLeaksOnShutdown := DebugHook <> 0;
{$ENDIF}
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
