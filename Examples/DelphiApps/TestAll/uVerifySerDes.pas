unit uVerifySerDes;

interface

type
  TLogger = procedure(str : string);

procedure Verify(Log : TLogger);

implementation

uses SysUtils, Windows,
     System.Classes,
     System.Generics.Collections,
     uOps.OPSObject,
     uOps.Participant,
     uOps.MemoryMap,
     uOps.ByteBuffer,
     uOps.ArchiverInOut,
     uOps.OpsArchiverOut,
     uOps.ChecksumArchiver,
     uOps.Topic,
     uOps.Error,
     uOps.OPSConfigRepository,
     TestAll.Definitions,
     TestAll.TestAllTypeFactory,
     TestAll.ChildData,
     TestAll.TestData,
     TestAll.Fruit;

const
  ToStr : array[Boolean] of string = ('False', 'True');
  EnumTest : Definitions.Command = Definitions.Command.PAUSE;

var
  Logger : TLogger;
  DetectedError : Boolean;

procedure Log(str : string);
begin
  if Assigned(Logger) then Logger(str);
end;

procedure LogError(str : string);
begin
  DetectedError := True;
  if Assigned(Logger) then Logger(str);
end;

procedure AssertNEQ(val, exp : Pointer; str : string = '');
begin
  if val = exp then LogError('Failed: ' + str + ', value= ' + IntToHex(UInt64(val), 8) + ', expected= ' + IntToHex(UInt64(exp), 8));
end;

function AssertEQ(val, exp : Boolean; str : string = '') : Boolean; overload;
begin
  Result := val;
  if val <> exp then LogError('Failed: ' + str + ', value= ' + ToStr[val] + ', expected= ' + ToStr[exp]);
end;

procedure AssertEQ(val, exp : AnsiString; str : string = ''); overload;
begin
  if val <> exp then LogError('Failed: ' + str + ', value= ' + string(val) + ', expected= ' + string(exp));
end;

procedure AssertEQ(val, exp : Int64; str : string = ''); overload;
begin
  if val <> exp then LogError('Failed: ' + str + ', value= ' + IntToStr(val) + ', expected= ' + IntToStr(exp));
end;

procedure AssertEQ(val, exp : Double; str : string = ''); overload;
begin
  if val <> exp then LogError('Failed: ' + str + ', value= ' + FloatToStr(val) + ', expected= ' + FloatToStr(exp));
end;

procedure AssertEQ(val, exp : ChildData.Order; str : string = ''); overload;
begin
  if val <> exp then LogError('Failed: ' + str);
end;

procedure AssertEQ(val, exp : Definitions.Command; str : string = ''); overload;
begin
  if val <> exp then LogError('Failed: ' + str);
end;

procedure AssertEQ(val : Boolean; str : string = ''); overload;
begin
  if not val then LogError('Failed: ' + str);
end;

// -----------------------------------------------------------------------------

procedure checkEmpty(data : TestData); overload;
begin
  AssertNEQ(data, nil);
  AssertEQ(data.text, '');
  AssertEQ(data.value, 0.0);
end;

procedure checkEmpty(data : ChildData); overload;
var
  i : Integer;
begin
  Log('Checking empty object...');

  //BaseData
  //   std::string baseText;
  AssertEQ(data.baseText, '', 'baseText');

  //   std::vector<std::string> stringOpenArr;
  AssertEQ(Length(data.stringOpenArr), 0, 'stringOpenArr');

  //   std::string stringFixArr[5];
  for i := 0 to 4 do AssertEQ(data.stringFixArr[i], '', 'stringFixArr');

  //   ops::fixed_string<23> fixLengthString;
  AssertEQ(data.fixLengthString, '', 'fixLengthString');

  //   std::vector<ops::fixed_string<16>> fixLengthStringOpenArr;
  AssertEQ(Length(data.fixLengthStringOpenArr), 0, 'fixLengthStringOpenArr');

  //   ops::fixed_string<16> fixLengthStringFixArr[10];
  for i := 0 to 9 do AssertEQ(data.fixLengthStringFixArr[i], '', 'fixLengthStringFixArr');

  // ChildData
	//  enums
	AssertEQ(data.enu1, ChildData.Order.ABC);
	AssertEQ(Length(data.enuVec), 0);
	for i := 0 to 5 do AssertEQ(data.enuFixArr[i], ChildData.Order.ABC);
	AssertEQ(data.cmd, Definitions.Command.START);
	for i := 0 to 1 do AssertEQ(data.cmds[i], Definitions.Command.START);

	//  core types
  AssertEQ(data.bo, false, 'data.bo');
  AssertEQ(data.b, 0, 'data.b');
  AssertEQ(data.sh, 0);
  AssertEQ(data.i, 0);
  AssertEQ(data.l, 0);
  AssertEQ(data.f, 0.0);
  AssertEQ(data.d, 0.0);
  AssertEQ(data.s, '');

  checkEmpty(data.test2);

  checkEmpty(data.testPointer);

  AssertEQ(Int64(data.fruit.value), Int64(TestAll.Fruit.APPLE));

  AssertEQ(Length(data.bos), 0);
  //	bool fbos[11];
  for i := 0 to 10 do AssertEQ(data.fbos[i], false);

  AssertEQ(Length(data.bs), 0);
  //	char fbs[256];
  for i := 0 to 255 do AssertEQ(data.fbs[i], 0);

  AssertEQ(Length(data.shs), 0);
  //	short fshs[4];
  for i := 0 to 3 do AssertEQ(data.fshs[i], 0);

  AssertEQ(Length(data.is_), 0);
  //	int fis_[3];
  for i := 0 to 2 do AssertEQ(data.fis_[i], 0);

  AssertEQ(Length(data.ls), 0);
  //	__int64 fls[6];
  for i := 0 to 5 do AssertEQ(data.fls[i], 0);

  AssertEQ(Length(data.fs), 0);
  //    float ffs[77];
  for i := 0 to 76 do AssertEQ(data.ffs[i], 0.0);

  AssertEQ(Length(data.ds), 0);
  //    double fds[5];
  for i := 0 to 4 do AssertEQ(data.fds[i], 0.0);

  AssertEQ(Length(data.ss), 0);
  //    std::string fss[10];
  for i := 0 to 9 do AssertEQ(data.fss[i], '');

  AssertEQ(Length(data.test2s), 0);
  //    TestData* ftest2s[5];
  for i := 0 to 4 do checkEmpty(data.ftest2s[i]);

  AssertEQ(Length(data.secondVirtArray), 0);
  //    TestData* fsecondVirtArray[7];
  for i := 0 to 6 do checkEmpty(data.fsecondVirtArray[i]);

  AssertEQ(Length(data.test2s2), 0);
  //    TestData ftest2s2[4];
  for i := 0 to 3 do checkEmpty(data.ftest2s2[i]);

  AssertEQ(Length(data.fruitarr), 0);
  //    Fruit ffruitarr[15];
  for i := 0 to 14 do AssertEQ(Int64(data.ffruitarr[i].value), Int64(TestAll.Fruit.APPLE));
end;

// -----------------------------------------------------------------------------

procedure checkObjects(data : TestData; exp : TestData; str : string); overload;
begin
  AssertNEQ(data, nil, str);
  AssertNEQ(exp, nil, str);
  AssertNEQ(data, exp, str);
  AssertEQ(data.text, exp.text);
  AssertEQ(data.value, exp.value);
end;

procedure checkObjects(data : ChildData; exp : ChildData); overload;
var
  i : Integer;
begin
  Log('Comparing objects...');

  //BaseData
  //   std::string baseText;
  AssertEQ(data.baseText, exp.baseText, 'baseText');

  //   std::vector<std::string> stringOpenArr;
  AssertEQ(Length(data.stringOpenArr), Length(exp.stringOpenArr), 'stringOpenArr');
  for i := 0 to Length(data.stringOpenArr) - 1 do AssertEQ(data.stringOpenArr[i], exp.stringOpenArr[i], 'stringOpenArr');

  //   std::string stringFixArr[5];
  for i := 0 to 4 do AssertEQ(data.stringFixArr[i], exp.stringFixArr[i], 'stringFixArr');

  //   ops::fixed_string<23> fixLengthString;
  AssertEQ(data.fixLengthString, exp.fixLengthString, 'fixLengthString');

  //   std::vector<ops::fixed_string<16>> fixLengthStringOpenArr;
  AssertEQ(Length(data.fixLengthStringOpenArr), Length(exp.fixLengthStringOpenArr), 'fixLengthStringOpenArr');
  for i := 0 to Length(data.fixLengthStringOpenArr) - 1 do AssertEQ(data.fixLengthStringOpenArr[i], exp.fixLengthStringOpenArr[i], 'fixLengthStringOpenArr');

  //   ops::fixed_string<16> fixLengthStringFixArr[10];
  for i := 0 to 9 do AssertEQ(data.fixLengthStringFixArr[i], exp.fixLengthStringFixArr[i], 'fixLengthStringFixArr');

  // ChildData
	//  enums
	AssertEQ(data.enu1, exp.enu1);
	AssertEQ(Length(data.enuVec), Length(exp.enuVec));
	for i := 0 to Length(data.enuVec)-1 do AssertEQ(data.enuVec[i], exp.enuVec[i]);
	for i := 0 to 5 do AssertEQ(data.enuFixArr[i], exp.enuFixArr[i]);
	AssertEQ(data.cmd, exp.cmd);
	for i := 0 to 1 do AssertEQ(data.cmds[i], exp.cmds[i]);

	//  core types
  AssertEQ(data.bo, exp.bo, 'data.bo');
  AssertEQ(data.b, exp.b, 'data.b');
  AssertEQ(data.sh, exp.sh);
  AssertEQ(data.i, exp.i);
  AssertEQ(data.l, exp.l);
  AssertEQ(data.f, exp.f);
  AssertEQ(data.d, exp.d);
  AssertEQ(data.s, exp.s);

  checkObjects(data.test2, exp.test2, 'test2');

  AssertEQ(data.testPointer is TestData, 'testPointer');
  AssertEQ(exp.testPointer is TestData, 'testPointer');

  checkObjects(data.testPointer, exp.testPointer, 'testPointer');

  AssertEQ(Int64(data.Fruit.value), Int64(exp.Fruit.value));

  AssertEQ(Length(data.bos), Length(exp.bos));
  // bool fbos[11];
  for i := 0 to 10 do AssertEQ(data.fbos[i], exp.fbos[i]);

  AssertEQ(Length(data.bs), Length(exp.bs));
  // char fbs[256];
  for i := 0 to 255 do AssertEQ(data.fbs[i], exp.fbs[i]);

  AssertEQ(Length(data.shs), Length(exp.shs));
  // short fshs[4];
  for i := 0 to 3 do AssertEQ(data.fshs[i], exp.fshs[i]);

  AssertEQ(Length(data.is_), Length(exp.is_));
  // int fis_[3];
  for i := 0 to 2 do AssertEQ(data.fis_[i], exp.fis_[i]);

  AssertEQ(Length(data.ls), Length(exp.ls));
  // __int64 fls[6];
  for i := 0 to 5 do AssertEQ(data.fls[i], exp.fls[i]);

  AssertEQ(Length(data.fs), Length(exp.fs));
  // float ffs[77];
  for i := 0 to 76 do AssertEQ(data.ffs[i], exp.ffs[i]);

  AssertEQ(Length(data.ds), Length(exp.ds));
  // double fds[5];
  for i := 0 to 4 do AssertEQ(data.fds[i], exp.fds[i]);

  AssertEQ(Length(data.ss), Length(exp.ss));
  // std::string fss[10];
  for i := 0 to 9 do AssertEQ(data.fss[i], exp.fss[i]);

  AssertEQ(Length(data.test2s), Length(exp.test2s));
  // TestData* ftest2s[5];
  for i := 0 to 4 do checkObjects(data.ftest2s[i], exp.ftest2s[i], 'ftest2s');

  AssertEQ(Length(data.secondVirtArray), Length(exp.secondVirtArray));
  // TestData* fsecondVirtArray[7];
  for i := 0 to 6 do checkObjects(data.fsecondVirtArray[i], exp.fsecondVirtArray[i], 'fsecondVirtArray');

  AssertEQ(Length(data.test2s2), Length(exp.test2s2));
  // TestData ftest2s2[4];
  for i := 0 to 3 do checkObjects(data.ftest2s2[i], exp.ftest2s2[i], 'ftest2s2');

  AssertEQ(Length(data.fruitarr), Length(exp.fruitarr));
  // Fruit ffruitarr[15];
  for i := 0 to 14 do AssertEQ(Int64(data.ffruitarr[i].value), Int64(exp.ffruitarr[i].value));
end;

// -----------------------------------------------------------------------------

// Fill ChildData with fixed values used for tests between languages
procedure fillChildData(data: TestAll.ChildData.ChildData);
var
  i : Integer;
begin
  // BasedData
  //   std::string baseText;
  //   baseText : AnsiString;
  data.baseText := 'dynamic string';
  //   std::vector<std::string> stringOpenArr;
  //   stringOpenArr : TDynAnsiStringArray;
  SetLength(data.stringOpenArr, 2);
  data.stringOpenArr[0] := 'dyn str 1';
  data.stringOpenArr[1] := 'dyn str 2';
  //   std::string stringFixArr[5];
  //   stringFixArr : array [0..4] of AnsiString;
  data.stringFixArr[0] := 'dsf 0';
  data.stringFixArr[1] := 'dsf 1';
  data.stringFixArr[2] := 'dsf 2';
  data.stringFixArr[3] := 'dsf 3';
  data.stringFixArr[4] := 'dsf 4';
  //   ops::fixed_string<23> fixLengthString;
  //   fixLengthString : AnsiString;
  data.fixLengthString := 'fixed length string';
  //   std::vector<ops::fixed_string<16>> fixLengthStringOpenArr;
  //   fixLengthStringOpenArr : TDynAnsiStringArray;
  SetLength(data.fixLengthStringOpenArr, 2);
  data.fixLengthStringOpenArr[0] := 'fix len str 1';
  data.fixLengthStringOpenArr[1] := 'fix len str 2';
  //   ops::fixed_string<16> fixLengthStringFixArr[10];
  //   fixLengthStringFixArr : array [0..9] of AnsiString;
  data.fixLengthStringFixArr[0] := 'fsf 0';
  data.fixLengthStringFixArr[1] := 'fsf 1';
  data.fixLengthStringFixArr[2] := 'fsf 2';
  data.fixLengthStringFixArr[3] := 'fsf 3';
  data.fixLengthStringFixArr[4] := 'fsf 4';
  data.fixLengthStringFixArr[5] := 'fsf 5';
  data.fixLengthStringFixArr[6] := 'fsf 6';
  data.fixLengthStringFixArr[7] := 'fsf 7';
  data.fixLengthStringFixArr[8] := 'fsf 8';
  data.fixLengthStringFixArr[9] := 'fsf 9';

  // ChildData
	//  enums
	data.enu1 := ChildData.Order.GHI;

  SetLength(data.enuVec, 5);
	data.enuVec[0] := ChildData.Order.GHI;
	data.enuVec[1] := ChildData.Order.JKL;
	data.enuVec[2] := ChildData.Order.JKL;
	data.enuVec[3] := ChildData.Order.ABC;
	data.enuVec[4] := ChildData.Order.DEF;

	data.enuFixArr[0] := ChildData.Order.DEF;
	data.enuFixArr[4] := ChildData.Order.JKL;
	data.enuFixArr[5] := ChildData.Order.DEF;

  data.cmd := Definitions.Command.CONTINUE;
	data.cmds[0] := Definitions.Command.PAUSE;
	data.cmds[1] := Definitions.Command.STOP;

	//  core types
  data.bo := true;
  data.b := 7;
  data.sh := -99;
  data.i := 19;
  data.l := 3456789;
  data.f := 123.4567;
  data.d := 987.12345678901;
  data.s := 'Test of [de]serializing';

  data.test2.text := 'TestData';
  data.test2.value := 555.5;

  data.testPointer.text := 'TestPtr';
  data.testPointer.value := 777.7;

  data.fruit.value := TestAll.Fruit.Fruit.enum.PEAR;

  SetLength(data.bos, 3);
  data.bos[0] := false;
  data.bos[1] := true;
  data.bos[2] := false;

  // bool fbos[11];
  FillChar(data.fbos, SizeOf(data.fbos), 0);
  data.fbos[5] := true;
  data.fbos[10] := true;

  SetLength(data.bs, 3);
  data.bs[0] := 10;
  data.bs[1] := 20;
  data.bs[2] := 30;

  // char fbs[256];
  for i := 0 to 255 do data.fbs[i] := i;

  SetLength(data.shs, 2);
  data.shs[0] := 1111;
  data.shs[1] := 2222;

  // short fshs[4];
  data.fshs[0] := 21;
  data.fshs[1] := 121;
  data.fshs[2] := 221;
  data.fshs[3] := 321;

  SetLength(data.is_, 4);
  data.is_[0] := 100000;
  data.is_[1] := 101010;
  data.is_[2] := 110101;
  data.is_[3] := 111111;

  // int fis_[3];
  data.fis_[0] := -1;
  data.fis_[1] := -2;
  data.fis_[2] := -3;

  SetLength(data.ls, 4);
  data.ls[0] := 9;
  data.ls[1] := 8;
  data.ls[2] := 7;
  data.ls[3] := 6;

  // __int64 fls[6];
  data.fls[0] := 9999;
  data.fls[1] := 9998;
  data.fls[2] := 9997;
  data.fls[3] := 9996;
  data.fls[4] := 9995;
  data.fls[5] := 9994;

  SetLength(data.fs, 4);
  data.fs[0] := 3.1;
  data.fs[1] := 31.14;
  data.fs[2] := 4.56;
  data.fs[3] := 987.0;

  // float ffs[77];
  for i := 0 to 76 do data.ffs[i] := 0.0;
  data.ffs[21] := 3.1415;

  SetLength(data.ds, 2);
  data.ds[0] := 1.987654321;
  data.ds[1] := 2.3456789;

  // double fds[5];
  data.fds[0] := 1.1;
  data.fds[1] := 2.1;
  data.fds[2] := 3.1;
  data.fds[3] := 4.1;
  data.fds[4] := 5.1;

  SetLength(data.ss, 3);
  data.ss[0] := 'Index 0';
  data.ss[1] := 'Index 1';
  data.ss[2] := 'Index 2';

  // std::string fss[10];
  data.fss[4] := '4 string';
  data.fss[7] := '7 string';
  data.fss[9] := '9 string';

  // std::vector<TestData*> test2s;
  SetLength(data.test2s, 4);
  data.test2s[0] := TestData.Create;
  data.test2s[1] := TestData.Create;
  data.test2s[2] := TestData.Create;
  data.test2s[3] := TestData.Create;
  // TestData* ftest2s[5];
  data.ftest2s[2].text := 'Index 2';
  data.ftest2s[2].value := 7.7;

  // std::vector<TestData*> secondVirtArray;
  SetLength(data.secondVirtArray, 2);
  data.secondVirtArray[0] := TestAll.TestData.TestData.Create;
  data.secondVirtArray[1] := TestAll.TestData.TestData.Create;
  // TestData* fsecondVirtArray[7];
  data.fsecondVirtArray[5].text := 'Index 5';
  data.fsecondVirtArray[5].value := -9.99;

  // std::vector<TestData> test2s2;
  SetLength(data.test2s2, 11);
  for i := 0 to 10 do data.test2s2[i] := TestAll.TestData.TestData.Create;
  // TestData ftest2s2[4];
  data.ftest2s2[3].text := '';
  data.ftest2s2[1].value := 710.6;

  SetLength(data.fruitarr, 2);
  for i := 0 to 1 do data.fruitarr[i] := TestAll.Fruit.Fruit.Create;
  data.fruitarr[0].value := TestAll.Fruit.Fruit.enum.PEAR;
  data.fruitarr[1].value := TestAll.Fruit.Fruit.enum.BANANA;

  // Fruit ffruitarr[15];
  data.ffruitarr[0].value := TestAll.Fruit.Fruit.enum.PEAR;
  data.ffruitarr[14].value := TestAll.Fruit.Fruit.enum.PEAR;
end;

// -----------------------------------------------------------------------------

type
  TStdOutLogger = class(TObject)
  public
    procedure OnErrorReport(Sender : TObject; Error : TError);
  end;

procedure TStdOutLogger.OnErrorReport(Sender : TObject; Error : TError);
begin
  Writeln(Error.getMessage);
end;


procedure Verify(Log : TLogger);
var
  pubs : TDictionary<string, Uint32>;
  OpsLogger : TStdOutLogger;
  cd1 : TestAll.ChildData.ChildData;
  cd2 : TestAll.ChildData.ChildData;
  cd3 : TestAll.ChildData.ChildData;
  participant : TParticipant;
  topic : TTopic;
  sub : ChildDataSubscriber;
  pub : ChildDataPublisher;
  Flag : Boolean;
  Limit : UInt64;
  PubTime : UInt64;
  map : TMemoryMap;
  buf : TByteBuffer;
  ao : TOPSArchiverOut;
  chksum : TChecksumArchiver;
  chkcalc : TCalculator_8bit_xor;
  cwd : string;
  idx : Integer;
  pubName : string;
  spareSize : NativeInt;
  msgCount : Uint32;

  procedure LogPubs;
  var
    pubnames : TStringList;
  begin
    pubnames := TStringList.Create;
    try
      Log('');
      Log('-----------------------------');

      for var Enum in pubs do begin
        pubnames.Add(Enum.Key);
      end;
      pubnames.Sort;

      for var str in pubnames do begin
	  	  Log( IntToStr(pubs.items[str]) + ' messages from ' + str );
      end;

      Log('-----------------------------');
    finally
      FreeAndNil(pubnames);
    end;
  end;

begin
  DetectedError := False;

  pubs := TDictionary<string, Uint32>.Create;

  OpsLogger := TStdOutLogger.Create;
  Logger := Log;

  cd1 := TestAll.ChildData.ChildData.Create;
  cd2 := TestAll.ChildData.ChildData.Create;
  cd3 := TestAll.ChildData.ChildData.Create;

	Log('Test initial state...');
  checkEmpty(cd1);
  checkEmpty(cd2);
  checkEmpty(cd3);

  checkObjects(cd1, cd2);
  Log('Finished');


	Log('Test cloning...');
	fillChildData(cd1);
	cd1.fillClone(TOPSObject(cd2));

	checkObjects(cd1, cd2);

  // Delete first object, recreate it and compare again
  // This to check that cd2 is really a deep clone (no common object)
  FreeAndNil(cd1);
  cd1 := TestAll.ChildData.ChildData.Create;
	fillChildData(cd1);

	checkObjects(cd1, cd2);

	Log('Finished');


	Log('Test Checksum...');
  chkcalc := TCalculator_8bit_xor.Create;
  chksum := TChecksumArchiver.Create(chkcalc);
  cd1.Serialize(chksum);
  AssertEQ(chkcalc.Sum, 140, 'Checksum error');

  Log('Checksum Archiver # fields = ' + IntToStr(chkcalc.TotalFields));
  Log('Checksum Archiver # bytes = ' + IntToStr(chkcalc.TotalBytes));
  Log('Checksum Archiver 8-bit XOR = ' + IntToStr(chkcalc.Sum));

  FreeAndNil(chksum);
  FreeAndNil(chkcalc);
	Log('Finished');


	Log('Serialize filled object');
	map := TMemoryMap.Create(1, 65536);
	buf := TByteBuffer.Create(map);
	ao := TOPSArchiverOut.Create(buf, False);

	Log('  GetSize()= ' + IntToStr(buf.GetSize()));
  ao.inout('data', TSerializable(cd1));
	Log('  optNonVirt = false, GetSize()= ' + IntToStr(buf.GetSize()));
  AssertEQ(buf.GetSize(), 3150, 'Serialized size error');

	FreeAndNil(map);
	FreeAndNil(buf);
	FreeAndNil(ao);

	map := TMemoryMap.Create(1, 65536);
	buf := TByteBuffer.Create(map);
	ao := TOPSArchiverOut.Create(buf, True);

	Log('  GetSize()= ' + IntToStr(buf.GetSize()));
  ao.inout('data', TSerializable(cd1));
	Log('  optNonVirt = true,  GetSize()= ' + IntToStr(buf.GetSize()));
  AssertEQ(buf.GetSize(), 2591, 'Serialized size error');
	Log('Serialize finished');

  Log('Test publish/subscribe');

  // Setup the OPS static error service (common for all participants, reports
  // errors during participant creation)
  uOps.Error.gStaticErrorService.addListener(OpsLogger.OnErrorReport);

  if FileExists('ops_config.xml') then begin
    Log('Using config file in CWD');
  end else begin
    cwd := GetCurrentDir;
    idx := Pos('Example', cwd);
    if Idx > 0 then begin
      cwd := Copy(cwd, 0, idx-1) + 'Examples/OPSIdls/TestAll/ops_config.xml';
      uOps.OPSConfigRepository.TOPSConfigRepository.Instance.Add(cwd);
      Log('Using config file: ' + cwd);
    end;
  end;

  participant := TParticipant.getInstance('TestAllDomain');
  if not Assigned(participant) then begin
    LogError('Create participant failed. do you have ops_config.xml on your rundirectory?');
    Exit;
  end;
  participant.addTypeSupport(TestAllTypeFactory.Create);
	// Setup the participant errorService to catch ev. internal OPS errors
	participant.getErrorService.addListener(OpsLogger.OnErrorReport);

  // Setup & start subscriber w polling
  topic := participant.getTopic('ChildTopic');
  sub := ChildDataSubscriber.Create(topic);
  sub.Start;

  // Setup & start publisher
  pub := ChildDataPublisher.Create(topic);
  pub.Name := 'Delphi';
  pub.Start;

  // Publish data
  pub.write(cd1);

  // Check that sent data isn't affected by publish
  checkObjects(cd1, cd2);

  // Check received values against sent values
  AssertEQ(sub.waitForNewData(100), true, 'No data received');
  flag := AssertEQ(sub.getData(cd3), true, 'No received data');

  sub.aquireMessageLock;
  AssertEQ(Length(sub.getMessage.spareBytes), 0, 'spareBytes');
  sub.releaseMessageLock;

  if flag then checkObjects(cd1, cd3);

  Log('Finished');

  Log('Waiting for more data... (Press Ctrl-C to terminate or wait 60 seconds)');
  Limit := GetTickCount + 60000;
  PubTime := GetTickCount + 5000;
  while True do begin
    if (sub.waitForNewData(100)) then begin
      sub.aquireMessageLock;
      pubName := string(sub.getMessage().PublisherName);
      spareSize := Length(sub.getMessage.spareBytes);
      sub.releaseMessageLock;
      sub.getData(cd3);

      Log('Received new data from ' + pubName + '. Checking...');
      AssertEQ(spareSize, 0, 'spareBytes');
      checkObjects(cd3, cd1);
      Log('Data check done');

      if pubs.TryGetValue(pubName, msgCount) then begin
        Inc(msgCount);
      end else begin
        msgCount := 1;
      end;
      pubs.AddOrSetValue(pubName, msgCount);
    end;
    if PubTime < GetTickCount then begin
      PubTime := GetTickCount + 5000;
      pub.write(cd1);
    end;
    if Limit < GetTickCount then Break;
  end;

  Logpubs;

  Log('');
  if DetectedError then begin
    Log('T E S T   F A I L E D');
  end else begin
    Log('T E S T   O K');
  end;
  Log('');

  Log('Sleeping for 10 seconds...');
  Sleep(10000);

  FreeAndNil(pub);
  FreeAndNil(sub);
  FreeAndNil(participant);

	FreeAndNil(map);
	FreeAndNil(buf);
	FreeAndNil(ao);

  FreeAndNil(cd3);
  FreeAndNil(cd2);
  FreeAndNil(cd1);

  FreeAndNil(OpsLogger);
  FreeAndNil(pubs);
end;

end.

