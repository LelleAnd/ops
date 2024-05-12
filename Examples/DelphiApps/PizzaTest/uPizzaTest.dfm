object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'PizzaTest (Version 2024-05-10)'
  ClientHeight = 516
  ClientWidth = 746
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  TextHeight = 13
  object Splitter1: TSplitter
    Left = 0
    Top = 216
    Width = 746
    Height = 3
    Cursor = crVSplit
    Align = alTop
    ExplicitTop = 129
    ExplicitWidth = 635
  end
  object Memo1: TMemo
    Left = 0
    Top = 219
    Width = 746
    Height = 297
    Align = alClient
    Lines.Strings = (
      'Memo1')
    ScrollBars = ssBoth
    TabOrder = 0
    ExplicitWidth = 742
    ExplicitHeight = 296
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 746
    Height = 216
    Align = alTop
    TabOrder = 1
    ExplicitWidth = 742
    object Label_NumOpsMessages: TLabel
      Left = 614
      Top = 14
      Width = 6
      Height = 13
      Caption = '0'
    end
    object Button_CreateParticipants: TButton
      Left = 8
      Top = 8
      Width = 105
      Height = 25
      Caption = 'Create Participants'
      TabOrder = 0
      OnClick = Button_CreateParticipantsClick
    end
    object Button_DeleteParticipants: TButton
      Left = 119
      Top = 8
      Width = 105
      Height = 25
      Caption = 'Delete Participants'
      TabOrder = 1
      OnClick = Button_DeleteParticipantsClick
    end
    object Button_CreatePublishers: TButton
      Left = 8
      Top = 39
      Width = 105
      Height = 25
      Caption = 'Create Publishers'
      TabOrder = 2
      OnClick = Button_CreatePublishersClick
    end
    object Button_DeletePublishers: TButton
      Left = 119
      Top = 39
      Width = 105
      Height = 25
      Caption = 'Delete Publishers'
      TabOrder = 3
      OnClick = Button_DeletePublishersClick
    end
    object Button_CreateSubscribers: TButton
      Left = 8
      Top = 99
      Width = 105
      Height = 25
      Caption = 'Create Subscribers'
      TabOrder = 4
      OnClick = Button_CreateSubscribersClick
    end
    object Button_DeleteSubscribers: TButton
      Left = 119
      Top = 99
      Width = 105
      Height = 25
      Caption = 'Delete Subscribers'
      TabOrder = 5
      OnClick = Button_DeleteSubscribersClick
    end
    object Button_StartSubscribers: TButton
      Left = 8
      Top = 130
      Width = 105
      Height = 25
      Caption = 'Start Subscribers'
      TabOrder = 6
      OnClick = Button_StartSubscribersClick
    end
    object Button_StopSubscribers: TButton
      Left = 119
      Top = 130
      Width = 105
      Height = 25
      Caption = 'Stop Subscribers'
      TabOrder = 7
      OnClick = Button_StopSubscribersClick
    end
    object Button_StartPublishers: TButton
      Left = 8
      Top = 70
      Width = 105
      Height = 25
      Caption = 'Start Publishers'
      TabOrder = 8
      OnClick = Button_StartPublishersClick
    end
    object Button_StopPublishers: TButton
      Left = 119
      Top = 70
      Width = 105
      Height = 25
      Caption = 'Stop Publishers'
      TabOrder = 9
      OnClick = Button_StopPublishersClick
    end
    object Button_WriteData: TButton
      Left = 648
      Top = 130
      Width = 91
      Height = 25
      Caption = 'Write Data'
      TabOrder = 10
      OnClick = Button_WriteDataClick
    end
    object CheckListBox1: TCheckListBox
      Left = 230
      Top = 8
      Width = 291
      Height = 197
      ItemHeight = 13
      TabOrder = 11
      OnClick = CheckListBox1Click
    end
    object LabeledEdit1: TLabeledEdit
      Left = 648
      Top = 103
      Width = 91
      Height = 21
      EditLabel.Width = 62
      EditLabel.Height = 13
      EditLabel.Caption = 'Vessuvio Len'
      TabOrder = 12
      Text = '0'
      OnExit = LabeledEdit1Exit
    end
    object Button_Clear: TButton
      Left = 8
      Top = 161
      Width = 105
      Height = 25
      Caption = 'Clear'
      TabOrder = 13
      OnClick = Button_ClearClick
    end
    object Button_ClearStorage: TButton
      Left = 648
      Top = 8
      Width = 91
      Height = 25
      Caption = 'Clear Storage'
      Enabled = False
      TabOrder = 14
      OnClick = Button_ClearStorageClick
    end
    object LabeledEdit_Deadline: TLabeledEdit
      Left = 556
      Top = 103
      Width = 82
      Height = 21
      EditLabel.Width = 65
      EditLabel.Height = 13
      EditLabel.Caption = 'Deadline [ms]'
      TabOrder = 15
      Text = '0'
      OnExit = LabeledEdit_DeadlineExit
    end
    object Button_SetDeadline: TButton
      Left = 556
      Top = 130
      Width = 82
      Height = 25
      Caption = 'Set Deadline'
      TabOrder = 16
      OnClick = Button_SetDeadlineClick
    end
    object LabeledEdit_SendPeriod: TLabeledEdit
      Left = 648
      Top = 64
      Width = 91
      Height = 21
      EditLabel.Width = 89
      EditLabel.Height = 13
      EditLabel.Caption = 'Send Interval [ms]'
      TabOrder = 17
      Text = '0'
      OnExit = LabeledEdit_SendPeriodExit
    end
    object LabeledEdit_PizzadataVersion: TLabeledEdit
      Left = 556
      Top = 64
      Width = 82
      Height = 21
      EditLabel.Width = 84
      EditLabel.Height = 13
      EditLabel.Caption = 'Pizzadata Version'
      TabOrder = 18
      Text = ''
    end
    object Edit1: TEdit
      Left = 556
      Top = 184
      Width = 82
      Height = 21
      TabOrder = 19
      Text = 'localhost'
    end
    object Button_GetHostAddress: TButton
      Left = 648
      Top = 182
      Width = 91
      Height = 25
      Caption = 'GetHostAddress'
      TabOrder = 20
      OnClick = Button_GetHostAddressClick
    end
  end
  object Timer1: TTimer
    Interval = 100
    OnTimer = Timer1Timer
    Left = 8
    Top = 360
  end
  object Timer_Send: TTimer
    Enabled = False
    OnTimer = Timer_SendTimer
    Left = 64
    Top = 360
  end
end
