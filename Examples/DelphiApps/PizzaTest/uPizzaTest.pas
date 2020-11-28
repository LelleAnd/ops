unit uPizzaTest;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.ExtCtrls, Vcl.StdCtrls,
  System.SyncObjs,
  System.Generics.Collections,
  uOps.Error,
  uOps.Participant,
  uOps.Subscriber,
  pizza.PizzaData,
  pizza.VessuvioData,
  pizza.special.ExtraAllt,
  pizza.special.Cheese,
  Vcl.CheckLst;

type
  TAbsHelper = class(TObject)
  public
    function HasPublisher : Boolean; virtual; abstract;
    function HasSubscriber : Boolean; virtual; abstract;
    procedure CreatePublisher(part : TParticipant; topicName : string); virtual; abstract;
    procedure DeletePublisher(doLog : Boolean = True); virtual; abstract;
    procedure StartPublisher; virtual; abstract;
    procedure StopPublisher; virtual; abstract;
    procedure Write; virtual; abstract;
    procedure CreateSubscriber(part : TParticipant; topicName : string); virtual; abstract;
    procedure DeleteSubscriber(doLog : Boolean = True); virtual; abstract;
    procedure StartSubscriber; virtual; abstract;
    procedure StopSubscriber; virtual; abstract;
    procedure SetDeadlineQos(timeoutMs : Int64); virtual; abstract;

    procedure ClearStorage; virtual; abstract;
  end;

  TItemInfo = class(Tobject)
    Domain : string;
    TopicName : string;
    TypeName : string;

    Selected : Boolean;
    Helper : TAbsHelper;
    Part : TParticipant;

    constructor Create(dom, top, typ : string);
  end;

  TForm1 = class(TForm)
    Memo1: TMemo;
    Splitter1: TSplitter;
    Panel1: TPanel;
    Button_CreateParticipants: TButton;
    Timer1: TTimer;
    Button_DeleteParticipants: TButton;
    Button_CreatePublishers: TButton;
    Button_DeletePublishers: TButton;
    Button_CreateSubscribers: TButton;
    Button_DeleteSubscribers: TButton;
    Button_StartSubscribers: TButton;
    Button_StopSubscribers: TButton;
    Button_StartPublishers: TButton;
    Button_StopPublishers: TButton;
    Button_WriteData: TButton;
    CheckListBox1: TCheckListBox;
    LabeledEdit1: TLabeledEdit;
    Button_Clear: TButton;
    Label_NumOpsMessages: TLabel;
    Button_ClearStorage: TButton;
    LabeledEdit_Deadline: TLabeledEdit;
    Button_SetDeadline: TButton;
    LabeledEdit_SendPeriod: TLabeledEdit;
    Timer_Send: TTimer;
    LabeledEdit_PizzadataVersion: TLabeledEdit;
    procedure FormCreate(Sender: TObject);
    procedure Button_CreateParticipantsClick(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure Button_DeleteParticipantsClick(Sender: TObject);
    procedure Button_CreatePublishersClick(Sender: TObject);
    procedure Button_DeletePublishersClick(Sender: TObject);
    procedure Button_CreateSubscribersClick(Sender: TObject);
    procedure Button_DeleteSubscribersClick(Sender: TObject);
    procedure Button_StartSubscribersClick(Sender: TObject);
    procedure Button_StopSubscribersClick(Sender: TObject);
    procedure Button_StartPublishersClick(Sender: TObject);
    procedure Button_StopPublishersClick(Sender: TObject);
    procedure Button_WriteDataClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure CheckListBox1Click(Sender: TObject);
    procedure LabeledEdit1Exit(Sender: TObject);
    procedure Button_ClearClick(Sender: TObject);
    procedure Button_ClearStorageClick(Sender: TObject);
    procedure LabeledEdit_DeadlineExit(Sender: TObject);
    procedure Button_SetDeadlineClick(Sender: TObject);
    procedure LabeledEdit_SendPeriodExit(Sender: TObject);
    procedure Timer_SendTimer(Sender: TObject);
  private
    { Private declarations }
    FMsgLog : TStringList;
    FMutex : TMutex;

    FItemInfoList : TList<TItemInfo>;

    FParticipant : TParticipant;
    FOtherParticipant : TParticipant;

    FCounter : Int64;
    FFillerString : AnsiString;

    procedure printDomainInfo(part : TParticipant);
    procedure WriteToAllSelected;
    procedure UpdateListbox;

    procedure OnPizzaData(Sub: TSubscriber; obj : PizzaData);
    procedure OnVessuvioData(Sub: TSubscriber; obj : VessuvioData);
    procedure OnExtraAllt(Sub: TSubscriber; obj : ExtraAllt);

  public
    { Public declarations }
    procedure OnErrorReport(Sender : TObject; Error : TError);
    procedure LogProc(mess : string);
  end;

var
  Form1: TForm1;

implementation

uses
  uOps.Domain,
  uOps.Types,
  uOps.Topic,
  uOps.OPSObject,
  uOps.OPSMessage,
  uOps.Publisher,
  uOps.XMLArchiverOut,
  uOps.OPSConfigRepository,
  PizzaProject.PizzaProjectTypeFactory;

{$R *.dfm}

constructor TItemInfo.Create(dom, top, typ : string);
begin
  Domain := dom;
  TopicName := top;
  TypeName := typ;
  Helper := nil;
  Part := nil;
  Selected := False;
end;

type
  TClientProc<DataType: TOPSObject> = procedure(Sub : TSubscriber; obj : DataType) of object;
  TLogProc = procedure(mess : string) of object;

  THelper<DataType: TOPSObject, constructor; DataTypePublisher: TPublisher; DataTypeSubscriber: TSubscriber> = class(TAbsHelper)
  private
    FClient : TClientProc<DataType>;
    FLog : TLogProc;
	  FPub : TPublisher;
    FSub : TSubscriber;
	  FExpectedPubId : Int64;

    FStorage : TObjectList<TOPSMessage>;
    FMutex : TMutex;

    procedure OnOpsMessage(Sender : TObject; obj : TOPSMessage);
    procedure OndeadlineMissed(Sender : TObject; int : Integer);
    procedure Log(mess : string);

  public
  	Data : DataType;

    constructor Create(Client : TClientProc<DataType>; LogProc : TLogProc);
    destructor Destroy; override;

    function HasPublisher : Boolean; override;
    function HasSubscriber : Boolean; override;
    procedure CreatePublisher(part : TParticipant; topicName : string); override;
    procedure DeletePublisher(doLog : Boolean = True); override;
    procedure StartPublisher; override;
    procedure StopPublisher; override;
    procedure Write; override;
    procedure CreateSubscriber(part : TParticipant; topicName : string); override;
    procedure DeleteSubscriber(doLog : Boolean = True); override;
    procedure StartSubscriber; override;
    procedure StopSubscriber; override;
    procedure SetDeadlineQos(timeoutMs : Int64); override;
    procedure ClearStorage; override;
  end;

  TPizzaHelper = THelper<PizzaData, PizzaDataPublisher, PizzaDataSubscriber>;
  TVessuvioHelper = THelper<VessuvioData, VessuvioDataPublisher, VessuvioDataSubscriber>;
  TExtraAlltHelper = THelper<ExtraAllt, ExtraAlltPublisher, ExtraAlltSubscriber>;

constructor THelper<DataType, DataTypePublisher, DataTypeSubscriber>.Create(Client : TClientProc<DataType>; LogProc : TLogProc);
begin
  inherited Create;
  Data := DataType.Create;
  FStorage := TObjectList<TOPSMessage>.Create(False);
  FMutex := TMutex.Create;

  FExpectedPubId := -1;
  FClient := Client;
  FLog := LogProc;
end;

destructor THelper<DataType, DataTypePublisher, DataTypeSubscriber>.Destroy;
begin
  DeletePublisher(False);
  DeleteSubscriber(False);
  ClearStorage;
  FreeAndNil(FStorage);
  FreeAndNil(FMutex);
  FreeAndNil(Data);
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.Log(mess : string);
begin
  if Assigned(FLog) then FLog(mess);
end;

function THelper<DataType, DataTypePublisher, DataTypeSubscriber>.HasPublisher : Boolean;
begin
  Result := Assigned(FPub);
end;

function THelper<DataType, DataTypePublisher, DataTypeSubscriber>.HasSubscriber : Boolean;
begin
  Result := Assigned(FSub);
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.CreatePublisher(part : TParticipant; topicName : string);
var
  topic : TTopic;
begin
  if Assigned(FPub) then begin
    Log('Publisher already exist for topic ' + string(FPub.Topic.Name));

  end else begin
    try
      // Create topic, might throw ops::NoSuchTopicException
      topic := part.getTopic(topicName);

      Log('Created topic ' + string(topic.Name) +
          ' [' + string(topic.Transport) +
          '::' + string(topic.DomainAddress) +
          '::' + IntToStr(topic.Port) +
          '] ');

      Log('  InSocketBufferSize: ' + IntToStr(topic.InSocketBufferSize) + #13#10 +
          '  OutSocketBufferSize: ' + IntToStr(topic.OutSocketBufferSize) + #13#10 +
          '  SampleMaxSize: ' + IntToStr(topic.SampleMaxSize));

      // Create a publisher on that topic
      FPub := TPublisher.Create(topic);
      FPub.Name := 'Delphi Test (' + AnsiString(IntToStr(GetCurrentProcessId)) + ')';
    except
      Log('Requested topic "' + topicName + '" does not exist!!');
    end;
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.DeletePublisher(doLog : Boolean);
begin
  if Assigned(FPub) then begin
    Log('Deleting publisher for topic ' + string(FPub.Topic.Name));
  	FPub.Stop;
    FreeAndNil(FPub);
  end else begin
    if doLog then Log('Publisher must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.StartPublisher;
begin
  if Assigned(FPub) then begin
    FPub.Start;
  end else begin
    Log('Publisher must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.StopPublisher;
begin
  if Assigned(FPub) then begin
    FPub.Stop;
  end else begin
    Log('Publisher must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.Write;
begin
  if Assigned(FPub) then begin
    FPub.writeOPSObject(Data);
  end else begin
    Log('Publisher must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.CreateSubscriber(part : TParticipant; topicName : string);
var
  topic : TTopic;
begin
  if Assigned(FSub) then begin
    Log('Subscriber already exist for topic ' + string(FSub.Topic.Name));
  end else begin
    try
      // Create topic, might throw ops::NoSuchTopicException
      topic := part.getTopic(topicName);

      Log('Created topic ' + string(topic.Name) +
          ' [' + string(topic.Transport) +
          '::' + string(topic.DomainAddress) +
          '::' + IntToStr(topic.Port) +
          '] ');

      Log('  InSocketBufferSize: ' + IntToStr(topic.InSocketBufferSize) + #13#10 +
          '  OutSocketBufferSize: ' + IntToStr(topic.OutSocketBufferSize) + #13#10 +
          '  SampleMaxSize: ' + IntToStr(topic.SampleMaxSize));

      // Create a subscriber on that topic.
      FSub := TSubscriber.Create(topic);
//      FSub.SetHistoryMaxSize(10);            //TEST
      FSub.AddDataListener(OnOpsMessage);
      FSub.AddDeadlineListener(OnDeadlineMissed);
      FSub.Start;
    except
      Log('Requested topic "' + topicName + '" does not exist!!');
    end;
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.DeleteSubscriber(doLog : Boolean);
begin
  if Assigned(FSub) then begin
    if (doLog) then Log('Deleting subscriber for topic ' + string(FSub.Topic.Name));
    FSub.Stop;
    FreeAndNil(FSub);
  end else begin
    if doLog then Log('Subscriber must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.StartSubscriber;
begin
  if Assigned(FSub) then begin
    Log('Starting subscriber for topic ' + string(FSub.Topic.Name));
    FSub.Start;
  end else begin
    Log('Subscriber must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.StopSubscriber;
begin
  if Assigned(FSub) then begin
    Log('Stopping subscriber for topic ' + string(FSub.Topic.Name));
    FSub.Stop;
  end else begin
    Log('Subscriber must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.SetDeadlineQos(timeoutMs : Int64);
begin
  if Assigned(FSub) then begin
    Log('Setting deadlineQos to ' + IntToStr(timeoutMs) + ' [ms] for topic ' + string(FSub.Topic.Name));
    FSub.DeadlineQoS := timeoutMs;
  end else begin
    Log('Subscriber must be created first!!');
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.OnOpsMessage(Sender : TObject; obj : TOPSMessage);
begin
  if Assigned(obj) then begin
    // Check if we have lost any messages. We use the publicationID and that works as long as
    // it is the same publisher sending us messages.
    if FExpectedPubId >= 0 then begin
      if FExpectedPubId <> obj.PublicationID then begin
        Log('>>>>> Lost message for topic ' + string(FSub.Topic.Name) +
            '. Exp.pubid: ' + IntToStr(FExpectedPubId) + ' got: ' + IntToStr(obj.PublicationID));
      end;
    end;
    FExpectedPubId := obj.PublicationID + 1;
    FMutex.Acquire;
    try
//      obj.Reserve;        //TEST  Bump up the reservation count so object isn't deleted
//      FStorage.Add(obj);
    finally
      FMutex.Release;
    end;
    if Assigned(FClient) then begin
      FClient(FSub, obj.Data as DataType);
    end;
  end;
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.OndeadlineMissed(Sender : TObject; int : Integer);
begin
  Log('Deadline Missed for topic ' + string(FSub.Topic.Name));
end;

procedure THelper<DataType, DataTypePublisher, DataTypeSubscriber>.ClearStorage;
var
  i : Integer;
begin
  FMutex.Acquire;
  try
    // Now tell objects that we don't need them any more, which may
    // lead to they beeing deleted.
    for i := 0 to FStorage.Count - 1 do FStorage.Items[i].UnReserve;
    FStorage.Clear;
  finally
    FMutex.Release;
  end;
end;

// ----------------------------------------------------------------------------

procedure TForm1.LabeledEdit1Exit(Sender: TObject);
var
  len : Integer;
begin
  try
    len := StrToInt(Trim(LabeledEdit1.Text));
    SetLength(FFillerString, len);
  except
    LabeledEdit1.SetFocus;
  end;
end;

procedure TForm1.LabeledEdit_DeadlineExit(Sender: TObject);
begin
  try
    StrToInt(Trim(LabeledEdit_Deadline.Text));
  except
    LabeledEdit_Deadline.SetFocus;
  end;
end;

procedure TForm1.LabeledEdit_SendPeriodExit(Sender: TObject);
var
  val : Integer;
begin
  try
    val := StrToInt(Trim(LabeledEdit_SendPeriod.Text));
    Timer_Send.Interval := val;
    Timer_Send.Enabled := val > 0;
  except
    LabeledEdit_SendPeriod.SetFocus;
  end;
end;

// Thread safe
procedure TForm1.LogProc(mess : string);
begin
  FMutex.Acquire;
  try
    FMsgLog.Add(mess);
  finally
    FMutex.Release;
  end;
end;

// Must be thread safe
procedure TForm1.OnErrorReport(Sender : TObject; Error : TError);
begin
  LogProc(Error.getMessage);
end;

// Must be thread safe
procedure TForm1.OnExtraAllt(Sub: TSubscriber; obj: ExtraAllt);
var
  addr : string;
  port : Integer;
  ss : string;
begin
  Sub.getMessage.getSource(addr, port);

  if Length(obj.shs) > 1 then begin
    ss := ', shs[1]: ' + IntToStr(obj.shs[1]);
  end;

  LogProc('[Topic: ' + string(Sub.Topic.Name) +
          '] (From ' + string(addr) + ':' + IntToStr(port) +
          ') ExtraAllt:: Cheese: ' + string(obj.cheese) +
          ss +
          ', Tomato sauce: ' + string(obj.tomatoSauce) +
          ', Num strings: ' + IntToStr(Length(obj.strings)));
end;

// Must be thread safe
procedure TForm1.OnPizzaData(Sub: TSubscriber; obj: PizzaData);
var
  addr : string;
  port : Integer;
begin
  // test for sending derived objects on same topic
  if obj is VessuvioData then begin
    OnVessuvioData(Sub, obj as VessuvioData);
    Exit;
  end;

  Sub.getMessage.getSource(addr, port);

  LogProc('[Topic: ' + string(Sub.Topic.Name) +
          '] (From ' + string(addr) + ':' + IntToStr(port) +
          ') PizzaData(v' + IntToStr(obj.PizzaData_version) +
          '):: Cheese: ' + string(obj.cheese) +
          ', Tomato sauce: ' + string(obj.tomatoSauce));
end;

// Must be thread safe
procedure TForm1.OnVessuvioData(Sub: TSubscriber; obj: VessuvioData);
var
  addr : string;
  port : Integer;
begin
  // test for sending derived objects on same topic
  if obj is ExtraAllt then begin
    OnExtraAllt(Sub, obj as ExtraAllt);
    Exit;
  end;

  Sub.getMessage.getSource(addr, port);

  LogProc('[Topic: ' + string(Sub.Topic.Name) +
          '] (From ' + string(addr) + ':' + IntTostr(port) +
          ') VessuvioData:: Cheese: ' + string(obj.cheese) +
          ', Tomato sauce: ' + string(obj.tomatoSauce) +
          ', Ham length: ' + IntToStr(Length(obj.ham)));
end;

procedure TForm1.Button_ClearClick(Sender: TObject);
begin
  Memo1.Clear;
end;

procedure TForm1.Button_ClearStorageClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.ClearStorage;
  end;
end;

procedure TForm1.Button_CreateParticipantsClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  // Create participants
  FParticipant := TParticipant.getInstance('PizzaDomain');
  if not Assigned(FParticipant) then begin
    Memo1.Lines.Add('Failed to create Participant. Missing ops_config.xml ??');
    Exit;
  end;
  FParticipant.addTypeSupport(PizzaProjectTypeFactory.Create);
  printDomainInfo(FParticipant);

  FOtherParticipant := TParticipant.getInstance('OtherPizzaDomain');
  if not Assigned(FOtherParticipant) then begin
    Memo1.Lines.Add('Failed to create Participant. Missing ops_config.xml ??');
    Exit;
  end;
  FOtherParticipant.addTypeSupport(PizzaProjectTypeFactory.Create);
  printDomainInfo(FOtherParticipant);

  // Add error writers to catch internal ops errors
  FParticipant.getErrorService.addListener(OnErrorReport);
  FOtherParticipant.getErrorService.addListener(OnErrorReport);

  // Finish up our ItemInfo's
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];

    // Create helper
    if info.TypeName = PizzaData.getTypeName then begin
			info.Helper := TPizzaHelper.Create(OnPizzaData, LogProc);
    end;
		if info.TypeName = VessuvioData.getTypeName then begin
			info.Helper := TVessuvioHelper.Create(OnVessuvioData, LogProc);
    end;
		if info.TypeName = ExtraAllt.getTypeName then begin
			info.Helper := TExtraAlltHelper.Create(OnExtraAllt, LogProc);
    end;

		// Set participant
		if info.Domain = 'PizzaDomain' then begin
			info.Part := FParticipant;
    end;
		if info.Domain = 'OtherPizzaDomain' then begin
			info.Part := FOtherParticipant;
    end;
  end;
  UpdateListbox;
end;

procedure TForm1.Button_CreatePublishersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.CreatePublisher(info.part, info.TopicName);
  end;
  UpdateListbox;
end;

procedure TForm1.Button_CreateSubscribersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.CreateSubscriber(info.part, info.TopicName);
  end;
  UpdateListbox;
end;

procedure TForm1.Button_DeleteParticipantsClick(Sender: TObject);
begin
  FreeAndNil(FOtherParticipant);
  FreeAndNil(FParticipant);
  UpdateListbox;
end;

procedure TForm1.Button_DeletePublishersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.DeletePublisher;
  end;
  UpdateListbox;
end;

procedure TForm1.Button_DeleteSubscribersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.DeleteSubscriber;
  end;
  UpdateListbox;
end;

procedure TForm1.Button_SetDeadlineClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
    info.Helper.SetDeadlineQos(StrToInt(Trim(LabeledEdit_Deadline.Text)));
  end;
end;

procedure TForm1.Button_StartPublishersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.StartPublisher;
  end;
end;

procedure TForm1.Button_StartSubscribersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.StartSubscriber;
  end;
end;

procedure TForm1.Button_StopPublishersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.StopPublisher;
  end;
end;

procedure TForm1.Button_StopSubscribersClick(Sender: TObject);
var
  i : Integer;
  info : TItemInfo;
begin
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;
		info.Helper.StopSubscriber;
  end;
end;

procedure TForm1.Button_WriteDataClick(Sender: TObject);
begin
  WriteToAllSelected;
end;

procedure TForm1.CheckListBox1Click(Sender: TObject);
var
  i : Integer;
begin
  for i := 0 to CheckListBox1.Count - 1 do begin
    FItemInfoList[i].Selected := CheckListBox1.Checked[i];
  end;
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
var
  i : Integer;
  info : TItemInfo;
begin
  Timer1.Enabled := False;
  for i := 0 to FItemInfoList.Count - 1 do begin
    info := FItemInfoList[i];
		FreeAndNil(info.Helper);
    FItemInfoList[i].Free;
  end;
  FItemInfoList.Clear;
  FreeAndNil(FItemInfoList);
  FreeAndNil(FParticipant);
  FreeAndNil(FOtherParticipant);
  FreeAndNil(FMutex);
  FreeAndNil(FMsgLog);
end;

procedure TForm1.FormCreate(Sender: TObject);
var
  info : TItemInfo;
  cwd : string;
  idx : Integer;
begin
  FMsgLog := TStringList.Create;

  // Setup the OPS static error service (common for all participants, reports errors during participant creation)
  uOps.Error.gStaticErrorService.addListener(OnErrorReport);

  FMutex := TMutex.Create;
  FItemInfoList := TList<TItemInfo>.Create;

  // Setup the InfoItem list (TODO take from ops_config.xml)
  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'PizzaTopic', 'pizza.PizzaData'));
  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'VessuvioTopic', 'pizza.VessuvioData'));

  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'PizzaTopic2', 'pizza.PizzaData'));
  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'VessuvioTopic2', 'pizza.VessuvioData'));

  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'TcpPizzaTopic', 'pizza.PizzaData'));
  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'TcpVessuvioTopic', 'pizza.VessuvioData'));

  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'TcpPizzaTopic2', 'pizza.PizzaData'));
  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'TcpVessuvioTopic2', 'pizza.VessuvioData'));

  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'UdpPizzaTopic', 'pizza.PizzaData'));
  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'UdpVessuvioTopic', 'pizza.VessuvioData'));

  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'UdpPizzaTopic2', 'pizza.PizzaData'));
  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'UdpVessuvioTopic2', 'pizza.VessuvioData'));

  FItemInfoList.Add(TItemInfo.Create('OtherPizzaDomain', 'OtherPizzaTopic', 'pizza.PizzaData'));
  FItemInfoList.Add(TItemInfo.Create('OtherPizzaDomain', 'OtherVessuvioTopic', 'pizza.VessuvioData'));

  FItemInfoList.Add(TItemInfo.Create('PizzaDomain', 'ExtraAlltTopic', 'pizza.special.ExtraAllt'));

  // Add all Domain's from file(s)
  if FileExists('ops_config.xml') then begin
    LogProc('Using config file in CWD');
  end else begin
    cwd := GetCurrentDir;
    idx := Pos('Example', cwd);
    if Idx > 0 then begin
      cwd := Copy(cwd, 0, idx-1) + 'Examples/OPSIdls/PizzaProject/ops_config.xml';
      uOps.OPSConfigRepository.TOPSConfigRepository.Instance.Add(cwd);
      LogProc('Using config file: ' + cwd);
    end;
  end;

  info := FItemInfoList[0];
  info.Selected := True;

  LabeledEdit_PizzadataVersion.Text := IntToStr(pizza.PizzaData.PizzaData.PizzaData_idlVersion);
end;

procedure TForm1.UpdateListbox;
var
  i : Integer;
  info : TItemInfo;
  status : string;
begin
  CheckListBox1.Items.Clear;
  for i := 0 to FItemInfoList.Count -1 do begin
    info := FItemInfoList[i];
    if Assigned(info.Helper) then begin
      if info.Helper.HasSubscriber then begin
        status := 'S';
      end else begin
        status := '_';
      end;
      if info.Helper.HasPublisher then begin
        status := status + 'P';
      end else begin
        status := status + '_';
      end;
    end else begin
      status := '__';
    end;
    CheckListBox1.Items.Add('   ' + status + '  ' + info.Domain + '::' + info.TopicName);
    CheckListBox1.Checked[i] := info.Selected;
  end;
end;

procedure TForm1.FormShow(Sender: TObject);
begin
  Memo1.Clear;
  UpdateListbox;
end;

procedure TForm1.WriteToAllSelected;
var
  i, j : Integer;
  info : TItemInfo;
  pdver : Integer;
begin
  pdver := StrToIntDef(LabeledEdit_PizzadataVersion.Text, -1);
  for i := 0 to FItemInfoList.Count -1 do begin
    info := FItemInfoList[i];
    if not info.Selected then Continue;
    if not Assigned(info.Helper) then Continue;

    if info.TypeName = PizzaData.getTypeName then begin
      with info.Helper as TPizzaHelper do begin
        if pdver < 0 then begin
          Data.IdlVersionMask := 0;
          Data.PizzaData_version := 0;
        end else begin
          Data.IdlVersionMask := 1;
          Data.PizzaData_version := pdver;
        end;
        Data.cheese := AnsiString('Pizza from Delphi: ' + IntToStr(FCounter));
        Data.tomatoSauce := 'Tomato';
      end;
    end;

    if info.TypeName = VessuvioData.getTypeName then begin
      with info.Helper as TVessuvioHelper do begin
        Data.cheese := AnsiString('Vessuvio from Delphi: ' + IntToStr(FCounter));
        Data.ham := FFillerString;
      end;
    end;

    if info.TypeName = ExtraAllt.getTypeName then begin
      with info.Helper as TExtraAlltHelper do begin
        Data.cheese := AnsiString('ExtraAllt from Delphi: ' + IntToStr(FCounter));
        if Length(Data.strings) = 0 then begin
          Setlength(Data.strings, 1000);
          for j := 0 to 999 do Data.strings[j] := 'hej';
        end;
        Data.sh := -7;
        if Length(Data.shs) = 0 then begin
          SetLength(Data.shs, 3);
          Data.shs[0] := 17;
          Data.shs[0] := 42;
          Data.shs[0] := -63;
        end;
      end;
    end;
    info.Helper.Write;

    Inc(FCounter);
  end;
end;

procedure TForm1.printDomainInfo(part: TParticipant);
var
  dom : TDomain;
begin
	dom := part.getDomain;
  Memo1.Lines.Add('  ---------------------------  ');
  Memo1.Lines.Add('  DomainID: ' + string(dom.DomainID));
  Memo1.Lines.Add('  DomainAddress: ' + string(dom.DomainAddress));
  Memo1.Lines.Add('  InSocketBufferSize: ' + IntToStr(dom.InSocketBufferSize));
  Memo1.Lines.Add('  OutSocketBufferSize: ' + IntToStr(dom.OutSocketBufferSize));
  Memo1.Lines.Add('  MetaDataMcPort: ' + IntToStr(dom.MetaDataMcPort));
  Memo1.Lines.Add('  LocalInterface: ' + string(dom.LocalInterface));
  Memo1.Lines.Add('  TimeToLive: ' +	IntToStr(dom.TimeToLive));
end;

procedure TForm1.Timer1Timer(Sender: TObject);
var
  i : Integer;
  list : TStringList;
begin
  list := TStringList.Create;
  try
    FMutex.Acquire;
    try
      list.AddStrings(FMsgLog);
      FMsgLog.Clear;
    finally
      FMutex.Release;
    end;
    for i := 0 to list.Count - 1 do begin
      Memo1.Lines.Add(list[i]);
    end;
  finally
    list.Free;
  end;
  Label_NumOpsMessages.Caption := IntToStr(uOps.OPSMessage.gNumObjects);
end;

procedure TForm1.Timer_SendTimer(Sender: TObject);
begin
  WriteToAllSelected;
end;

end.

