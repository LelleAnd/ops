--
-- Copyright (C) 2017-2020 Lennart Andersson.
--
-- This file is part of OPS (Open Publish Subscribe).
--
-- OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.

with System.Atomic_Counters;
with Ada.Text_IO; use Ada.Text_IO;
with Ada.Directories; use Ada.Directories;
with Ada.Strings.Fixed; use Ada.Strings.Fixed;
with Ops_Pa;

with Interfaces;
with GNAT.Ctrl_C;

with Ops_Pa.MemoryMap_Pa;
with Ops_Pa.ByteBuffer_Pa;
with Ops_Pa.ArchiverInOut_Pa;
with Ops_Pa.ArchiverInOut_Pa.ArchiverOut_Pa;
use Ops_Pa.MemoryMap_Pa;
use Ops_Pa.ByteBuffer_Pa;
use Ops_Pa.ArchiverInOut_Pa;
use Ops_Pa.ArchiverInOut_Pa.ArchiverOut_Pa;

with Ops_Pa.OpsObject_Pa;
with Ops_Pa.OpsObject_Pa.OPSConfig_Pa;
with Ops_Pa.OpsObject_Pa.TestAll_Definitions;
with Ops_Pa.OpsObject_Pa.TestAll_BaseData;
with Ops_Pa.OpsObject_Pa.TestAll_TestData;
with Ops_Pa.OpsObject_Pa.TestAll_ChildData;
with Ops_Pa.OpsObject_Pa.TestAll_Fruit;
with Ops_Pa.SerializableFactory_Pa.TestAll_TestAllTypeFactory;

use Ops_Pa.OpsObject_Pa;
use Ops_Pa.OpsObject_Pa.OPSConfig_Pa;
use Ops_Pa.OpsObject_Pa.TestAll_Definitions;
use Ops_Pa.OpsObject_Pa.TestAll_BaseData;
use Ops_Pa.OpsObject_Pa.TestAll_TestData;
use Ops_Pa.OpsObject_Pa.TestAll_ChildData;
use Ops_Pa.OpsObject_Pa.TestAll_Fruit;
use Ops_Pa.SerializableFactory_Pa.TestAll_TestAllTypeFactory;

with Ops_Pa.PublisherAbs_Pa.Publisher_Pa;
with Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData;
with Ops_Pa.OpsObject_Pa.Topic_Pa;
with Ops_Pa.Subscriber_Pa;
with Ops_Pa.Subscriber_Pa.TestAll_ChildData;
with Ops_Pa.OpsObject_Pa.OPSMessage_Pa;
use Ops_Pa.Subscriber_Pa;
use Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData;
use Ops_Pa.Subscriber_Pa.TestAll_ChildData;

with Ops_Pa.ArchiverInOut_Pa.PrintArchiverOut_Pa;
use Ops_Pa.ArchiverInOut_Pa.PrintArchiverOut_Pa;

with Ops_Pa.ArchiverInOut_Pa.ChecksumArchiver_Pa;
use Ops_Pa.ArchiverInOut_Pa.ChecksumArchiver_Pa;

with Ops_Pa; use Ops_Pa;
with Ops_Pa.Error_Pa;
with Ops_Pa.Participant_Pa;
with Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa;

package body VerifySerDes_Pa is

  -- Access to operations on types
  use type Ops_Pa.UInt32;
  use type Ops_Pa.Int16;
  use type Ops_Pa.Int32;
  use type Ops_Pa.Float64;

  use type Ops_Pa.CreateStatus_T;

  procedure MyTrace( Class : String;
             CreateStatus  : Ops_Pa.CreateStatus_T;
             TotalAllocObj : System.Atomic_Counters.Atomic_Unsigned) is
  begin
    if CreateStatus = Ops_Pa.Alloc then
      Put_Line("Debug: Alloc: " & Class & ", Total= " & System.Atomic_Counters.Atomic_Unsigned'Image(TotalAllocObj));
    else
      Put_Line("Debug: Dealloc: " & Class & ", Total= " & System.Atomic_Counters.Atomic_Unsigned'Image(TotalAllocObj));
    end if;
  end;

  procedure InstallMyTrace is
  begin
    Ops_Pa.InstallTrace(Routine => MyTrace'Access);
  end;

  procedure Log(str : string) is
  begin
    Put_Line(str);
  end;

  EnumTest : TestAll_Definitions.Command := TestAll_Definitions.PAUSE;


  -- ---------------------

  gErrorCount : Integer := 0;

  procedure ErrorLog(str : String) is
  begin
    Log("### Failed: " & str);
    gErrorCount := gErrorCount + 1;
  end;

  function AssertEQ(val, exp : Boolean; str : string := "") return Boolean is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Boolean'Image(val) & ", expected= " & Boolean'Image(exp));
    end if;
    return val;
  end;

  procedure AssertEQ(val, exp : Boolean; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Boolean'Image(val) & ", expected= " & Boolean'Image(exp));
    end if;
  end;

  procedure AssertEQ(val, exp : String_At; str : string := "") is
  begin
    -- NOTE that we consider null be the same as "", since we can't distinguish this when received
    if val /= exp then
      if val = null then
        if exp.all /= "" then
          ErrorLog(str & ", value= (null)" & ", expected= " & exp.all);
        end if;
      elsif exp = null then
        if val.all /= "" then
          ErrorLog(str & ", value= " & val.all & ", expected= (null)");
        end if;
      elsif val.all /= exp.all then
        ErrorLog(str & ", value= " & val.all & ", expected= " & exp.all);
      end if;
    end if;
  end;

  -- ---------------------

  generic
    type Item is (<>);
  procedure AssertIntEQ(val, exp : Item; str : string := "");

  procedure AssertIntEQ(val, exp : Item; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Item'Image(val) & ", expected= " & Item'Image(exp));
    end if;
  end;

  procedure AssertEQ is new AssertIntEQ(Byte);
  procedure AssertEQ is new AssertIntEQ(Int8);
  procedure AssertEQ is new AssertIntEQ(Int16);
  procedure AssertEQ is new AssertIntEQ(Int32);
  procedure AssertEQ is new AssertIntEQ(Int64);

  procedure AssertEQ is new AssertIntEQ(Ops_Pa.OpsObject_Pa.TestAll_Fruit.enum);
  procedure AssertEQ is new AssertIntEQ(Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order);
  procedure AssertEQ is new AssertIntEQ(Ops_Pa.OpsObject_Pa.TestAll_Definitions.Command);

  -- ---------------------

  generic
    type Item is digits <>;
  procedure AssertFloatEQ(val, exp : Item; str : string := "");

  procedure AssertFloatEQ(val, exp : Item; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Item'Image(val) & ", expected= " & Item'Image(exp));
    end if;
  end;

  procedure AssertEQ is new AssertFloatEQ(Float32);
  procedure AssertEQ is new AssertFloatEQ(Float64);

  -- ---------------------

  generic
    type Item is private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
  procedure AssertAccessEQ0(val, exp : Item_Arr_At; str : string := "");

  procedure AssertAccessEQ0(val, exp : Item_Arr_At; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertPtrEQ is new AssertAccessEQ0(Boolean, Boolean_Arr, Boolean_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Int16, Int16_Arr, Int16_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Int32, Int32_Arr, Int32_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Int64, Int64_Arr, Int64_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Float32, Float32_Arr, Float32_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Float64, Float64_Arr, Float64_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(String_At, String_Arr, String_Arr_At);

  procedure AssertPtrEQ is new AssertAccessEQ0(Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order,
                                               Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr,
                                               Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr_At);

  generic
    type Item is private;
    type Item_Arr is array(Byte_Arr_Index_T range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
  procedure AssertAccessEQ10(val, exp : Item_Arr_At; str : string := "");

  procedure AssertAccessEQ10(val, exp : Item_Arr_At; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertPtrEQ is new AssertAccessEQ10(Byte,  Byte_Arr,  Byte_Arr_At);

  generic
    type Item is private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
    with procedure AssertItem(val, exp : Item; str : string);
  procedure AssertArrayEQ0(val, exp : Item_Arr_At; str : string := "");

  procedure AssertArrayEQ0(val, exp : Item_Arr_At; str : string := "") is
  begin
    if val /= null or exp /= null then
      if val = null or exp = null then
        ErrorLog(str & ", A Pointer is null");
      else
        if val'length /= exp'length then
          ErrorLog(str & ", Arrays have different sizes");
        else
          for i in val'range loop
            AssertItem(val(i), exp(i), str);
          end loop;
        end if;
      end if;
    end if;
  end;

  procedure AssertArrEQ is new AssertArrayEQ0(Boolean, Boolean_Arr, Boolean_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Int16, Int16_Arr, Int16_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Int32, Int32_Arr, Int32_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Int64, Int64_Arr, Int64_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Float32, Float32_Arr, Float32_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Float64, Float64_Arr, Float64_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(String_At, String_Arr, String_Arr_At, AssertEQ);

  procedure AssertArrEQ is new AssertArrayEQ0(Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order,
                                              Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr,
                                              Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr_At,
                                              AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Ops_Pa.OpsObject_Pa.TestAll_Definitions.Command,
                                              Ops_Pa.OpsObject_Pa.TestAll_Definitions.Command_Arr,
                                              Ops_Pa.OpsObject_Pa.TestAll_Definitions.Command_Arr_At,
                                              AssertEQ);

  generic
    type Item is private;
    type Item_Arr is array(Byte_Arr_Index_T range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
    with procedure AssertItem(val, exp : Item; str : string);
  procedure AssertArrayEQ10(val, exp : Item_Arr_At; str : string := "");

  procedure AssertArrayEQ10(val, exp : Item_Arr_At; str : string := "") is
  begin
    if val /= null or exp /= null then
      if val = null or exp = null then
        ErrorLog(str & ", A Pointer is null");
      else
        if val'length /= exp'length then
          ErrorLog(str & ", Arrays have different sizes");
        else
          for i in val'range loop
            AssertItem(val(i), exp(i), str);
          end loop;
        end if;
      end if;
    end if;
  end;

  procedure AssertArrEQ is new AssertArrayEQ10(Byte,  Byte_Arr,  Byte_Arr_At, AssertEQ);

  -- ---------------------

  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
  procedure AssertAccessEQ(val, exp : Item_At; str : string := "");

  procedure AssertAccessEQ(val, exp : Item_At; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertPtrEQ is new AssertAccessEQ(TestData_Class, TestData_Class_At);
  procedure AssertPtrEQ is new AssertAccessEQ(Fruit_Class, Fruit_Class_At);

  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
  function AssertAccessNEQ(val, exp : Item_At; str : string := "") return Boolean;

  function AssertAccessNEQ(val, exp : Item_At; str : string := "") return Boolean is
  begin
    if val = exp and val /= null then
      ErrorLog(str & ", Pointers are equal");
      return False;
    end if;
    return True;
  end;

  function AssertPtrNEQ is new AssertAccessNEQ(TestData_Class, TestData_Class_At);
  function AssertPtrNEQ is new AssertAccessNEQ(Fruit_Class, Fruit_Class_At);

  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
    type Item_At_Arr is array(Integer range <>) of Item_At;
    type Item_At_Arr_At is access all Item_At_Arr;
  procedure AssertAccessEQ3(val, exp : Item_At_Arr_At; str : string := "");

  procedure AssertAccessEQ3(val, exp : Item_At_Arr_At; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertPtrEQ is new AssertAccessEQ3(TestData_Class, TestData_Class_At, TestData_Class_At_Arr, TestData_Class_At_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ3(Fruit_Class, Fruit_Class_At, Fruit_Class_At_Arr, Fruit_Class_At_Arr_At);

  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
    type Item_At_Arr is array(Integer range <>) of Item_At;
    type Item_At_Arr_At is access all Item_At_Arr;
    with procedure AssertItem(val, exp : Item_At; str : string);
  procedure AssertArrayEQ3(val, exp : Item_At_Arr_At; str : string := "");

  procedure AssertArrayEQ3(val, exp : Item_At_Arr_At; str : string := "") is
  begin
    if val /= null or exp /= null then
      if val = null or exp = null then
        ErrorLog(str & ", A Pointer is null");
      else
        if val.all'length /= exp.all'length then
          ErrorLog(str & ", Arrays have different sizes");
        else
          for i in val.all'range loop
            AssertItem(val.all(i), exp.all(i), str & "(" & Integer'Image(i) & ")");
          end loop;
        end if;
      end if;
    end if;
  end;

  generic
    type Item is tagged limited private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
  procedure AssertAccessEQ4(val, exp : Item_Arr_At; str : string := "");

  procedure AssertAccessEQ4(val, exp : Item_Arr_At; str : string := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertPtrEQ is new AssertAccessEQ4(TestData_Class, TestData_Class_Arr, TestData_Class_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ4(Fruit_Class, Fruit_Class_Arr, Fruit_Class_Arr_At);

  generic
    type Item is tagged limited private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
    with procedure AssertItem(val, exp : Item; str : string);
  procedure AssertArrayEQ4(val, exp : Item_Arr_At; str : string := "");

  procedure AssertArrayEQ4(val, exp : Item_Arr_At; str : string := "") is
  begin
    if val /= null or exp /= null then
      if val = null or exp = null then
        ErrorLog(str & ", A Pointer is null");
      else
        if val'length /= exp'length then
          ErrorLog(str & ", Arrays have different sizes");
        else
          for i in val'range loop
            AssertItem(val(i), exp(i), str);
          end loop;
        end if;
      end if;
    end if;
  end;

  -- ---------------------

  procedure AssertEQ(val : Boolean; str : string := "") is
  begin
    if not val then
      ErrorLog(str);
    end if;
  end;

  -- /////////////////////////////////////////////////////////////////////////

  procedure checkEmpty(data : TestData_Class; str : string := "") is
  begin
    AssertEQ(data.text, null, str);
    AssertEQ(data.value, 0.0, str);
  end;

  procedure checkEmpty(data : TestData_Class_At; str : string := "") is
  begin
    -- AssertPtrEQ(data, null, str);
    if data /= null then
      AssertEQ(data.text, null, str);
      AssertEQ(data.value, 0.0, str);
    end if;
  end;

  procedure checkObjects(data, exp : TestData_Class_At; str : string := "") is
  begin
    if data /= null or exp /= null then
      if AssertPtrNEQ(data, null, str) then
        if AssertPtrNEQ(exp, null, str) then
          if AssertPtrNEQ(data, exp, str) then
            AssertEQ(data.text, exp.text, str & ".text");
            AssertEQ(data.value, exp.value, str & ".value");
          end if;
        end if;
      end if;
    end if;
  end;

  procedure checkObjects(data, exp : TestData_Class; str : string := "") is
  begin
    AssertEQ(data.text, exp.text, str);
    AssertEQ(data.value, exp.value, str);
  end;

  procedure AssertArrEQ is new AssertArrayEQ4(TestData_Class, TestData_Class_Arr, TestData_Class_Arr_At, checkObjects);
  procedure AssertArrEQ is new AssertArrayEQ3(TestData_Class, TestData_Class_At, TestData_Class_At_Arr, TestData_Class_At_Arr_At, checkObjects);

  -- /////////////////////////////////////////////////////////////////////////

  procedure checkEmpty(data : Fruit_Class; str : string := "") is
  begin
    AssertEQ(data.value, Ops_Pa.OpsObject_Pa.TestAll_Fruit.APPLE, str);
  end;

  procedure checkEmpty(data : Fruit_Class_At; str : string := "") is
  begin
    -- AssertPtrEQ(data, null, str);
    if data /= null then
      AssertEQ(data.value, Ops_Pa.OpsObject_Pa.TestAll_Fruit.APPLE, str);
    end if;
  end;

  procedure checkObjects(data, exp : Fruit_Class_At; str : string := "") is
  begin
    if data /= null or exp /= null then
      if AssertPtrNEQ(data, null, str) then
        if AssertPtrNEQ(exp, null, str) then
          if AssertPtrNEQ(data, exp, str) then
            AssertEQ(data.value, exp.Value, str);
          end if;
        end if;
      end if;
    end if;
  end;

  procedure checkObjects(data, exp : Fruit_Class; str : string := "") is
  begin
    AssertEQ(data.value, exp.value, str);
  end;

  procedure AssertArrEQ is new AssertArrayEQ4(Fruit_Class, Fruit_Class_Arr, Fruit_Class_Arr_At, checkObjects);
  procedure AssertArrEQ is new AssertArrayEQ3(Fruit_Class, Fruit_Class_At, Fruit_Class_At_Arr, Fruit_Class_At_Arr_At, checkObjects);

  -- /////////////////////////////////////////////////////////////////////////

  procedure checkEmpty(data : ChildData_Class_At) is
    dummy : Boolean;
  begin
    Log("  Checking empty object...");

    -- BaseData
    --   std::string baseText;
    AssertEQ(data.baseText, null, "baseText");

    --   std::vector<std::string> stringOpenArr;
    AssertPtrEQ(data.stringOpenArr, null, "stringOpenArr");

    --   std::string stringFixArr[5];
    for i in data.stringFixArr'Range loop AssertEQ(data.stringFixArr(i), null, "stringFixArr"); end loop;

    --   ops::fixed_string<23> fixLengthString;
    AssertEQ(data.fixLengthString, null, "fixLengthString");

    --   std::vector<ops::fixed_string<16>> fixLengthStringOpenArr;
    AssertPtrEQ(data.fixLengthStringOpenArr, null, "fixLengthStringOpenArr");

    --   ops::fixed_string<16> fixLengthStringFixArr[10];
    for i in data.fixLengthStringFixArr'Range loop AssertEQ(data.fixLengthStringFixArr(i), null, "fixLengthStringFixArr"); end loop;


    -- ChildData
    -- enums
    AssertEQ(data.enu1, ABC);
    AssertPtrEQ(data.enuVec, null);
    for i in data.enuFixArr'Range loop AssertEQ(data.enuFixArr(i), ABC); end loop;
    AssertEQ(data.cmd, START);
    for i in data.cmds'Range loop AssertEQ(data.cmds(i), START); end loop;

    -- core types
    dummy := AssertEQ(data.bo, false, "data.bo");
    AssertEQ(data.b, 0, "data.b");
    AssertEQ(data.sh, 0);
    AssertEQ(data.i, 0);
    AssertEQ(data.l, 0);
    AssertEQ(data.f, 0.0);
    AssertEQ(data.d, 0.0);
    AssertEQ(data.s, null);

    checkEmpty(data.test2, "test2");

    checkEmpty(data.testPointer, "testPointer");

    checkEmpty(data.fruit, "fruit");

    AssertPtrEQ(data.bos, null);
    --AssertEQ(Int32(data.bos'Length), 0);
    --	bool fbos(11);
    for i in data.fbos'Range loop dummy := AssertEQ(data.fbos(i), false); end loop;

    AssertPtrEQ(data.bs, null);
    --AssertEQ(Int32(data.bs'Length), 0);
    --	char fbs(256);
    for i in data.fbs'Range loop AssertEQ(data.fbs(i), 0); end loop;

    AssertPtrEQ(data.shs, null);
    --AssertEQ(Int32(data.shs'Length), 0);
    --	short fshs(4);
    for i in data.fshs'Range loop AssertEQ(data.fshs(i), 0); end loop;

    AssertPtrEQ(data.is_a, null);
    --AssertEQ(Int32(data.is_a'Length), 0);
    --	int fis_(3);
    for i in data.fis_a'range loop AssertEQ(data.fis_a(i), 0); end loop;

    AssertPtrEQ(data.ls, null);
    --AssertEQ(Int32(data.ls'Length), 0);
    --	__int64 fls(6);
    for i in data.fls'Range loop AssertEQ(data.fls(i), 0); end loop;

    AssertPtrEQ(data.fs, null);
    --AssertEQ(Int32(data.fs'Length), 0);
    --    float ffs(77);
    for i in data.ffs'Range loop AssertEQ(data.ffs(i), 0.0); end loop;

    AssertPtrEQ(data.ds, null);
    --AssertEQ(Int32(data.ds'Length), 0);
    --    double fds(5);
    for i in data.fds'range loop AssertEQ(data.fds(i), 0.0); end loop;

    AssertPtrEQ(data.ss, null);
    --AssertEQ(Int32(data.ss'Length), 0);
    --    std::string fss(10);
    for i in data.fss'Range loop AssertEQ(data.fss(i), null); end loop;

    AssertPtrEQ(data.test2s, null);
    --AssertEQ(Int32(data.test2s'Length), 0);
    --    TestData* ftest2s(5);
    for i in data.ftest2s'range loop checkEmpty(data.ftest2s(i)); end loop;

    AssertPtrEQ(data.secondVirtArray, null);
    --AssertEQ(Int32(data.secondVirtArray'Length), 0);
    --    TestData* fsecondVirtArray(7);
    for i in data.fsecondVirtArray'Range loop checkEmpty(data.fsecondVirtArray(i)); end loop;

    AssertPtrEQ(data.test2s2, null);
    --AssertEQ(Int32(data.test2s2'Length), 0);
    --    TestData ftest2s2(4);
    for i in data.ftest2s2'range loop checkEmpty(data.ftest2s2(i)); end loop;

    AssertPtrEQ(data.fruitarr, null);
    --AssertEQ(Int32(data.fruitarr'Length), 0);
    --    Fruit ffruitarr(15);
    for i in data.ffruitarr'range loop checkEmpty(data.ffruitarr(i)); end loop;

    Log("  Checking empty object finished");
  end;

  -- -----------------------------------------------------------------------------

  procedure checkObjects(data, exp : ChildData_Class_At) is
    dummy : Boolean;
  begin
    Log("  Comparing objects...");

    -- BaseData
    --   std::string baseText;
    AssertEQ(data.baseText, exp.baseText, "baseText");

    --   std::vector<std::string> stringOpenArr;
    AssertArrEQ(data.stringOpenArr, exp.stringOpenArr, "stringOpenArr");

    --   std::string stringFixArr[5];
    AssertArrEQ(data.stringFixArr, exp.stringFixArr, "stringFixArr");

    --   ops::fixed_string<23> fixLengthString;
    AssertEQ(data.fixLengthString, exp.fixLengthString, "fixLengthString");

    --   std::vector<ops::fixed_string<16>> fixLengthStringOpenArr;
    AssertArrEQ(data.fixLengthStringOpenArr, exp.fixLengthStringOpenArr, "fixLengthStringOpenArr");

    --   ops::fixed_string<16> fixLengthStringFixArr[10];
    AssertArrEQ(data.fixLengthStringFixArr, exp.fixLengthStringFixArr, "fixLengthStringFixArr");


    -- ChildData
    -- enums
    AssertEQ(data.enu1, exp.enu1);
    AssertArrEQ(data.enuVec, exp.enuVec);
    AssertArrEQ(data.enuFixArr, exp.enuFixArr);
    AssertEQ(data.cmd, exp.cmd);
    AssertArrEQ(data.cmds, exp.cmds);

    -- core types
    AssertEQ(data.bo, exp.bo, "data.bo");
    AssertEQ(data.b, exp.b, "data.b");
    AssertEQ(data.sh, exp.sh);
    AssertEQ(data.i, exp.i);
    AssertEQ(data.l, exp.l);
    AssertEQ(data.f, exp.f);
    AssertEQ(data.d, exp.d);
    AssertEQ(data.s, exp.s);

    checkObjects(data.test2, exp.test2, "test2");

    checkObjects(data.testPointer, exp.testPointer, "testPointer");

    checkObjects(data.Fruit, exp.Fruit);

    AssertArrEQ(data.bos, exp.bos);
    --	bool fbos(11);
    --for i in data.fbos'Range loop dummy := AssertEQ(data.fbos(i), exp.fbos(i)); end loop;
    AssertArrEQ(data.fbos, exp.fbos);

    AssertArrEQ(data.bs, exp.bs);
    --	char fbs(256);
    --for i in data.fbs'Range loop AssertEQ(data.fbs(i), exp.fbs(i)); end loop;
    AssertArrEQ(data.fbs, exp.fbs);

    AssertArrEQ(data.shs, exp.shs);
    --	short fshs(4);
    --for i in data.fshs'Range loop AssertEQ(data.fshs(i), exp.fshs(i)); end loop;
    AssertArrEQ(data.fshs, exp.fshs);

    AssertArrEQ(data.is_a, exp.is_a);
    --	int fis_(3);
    --for i in data.fis_a'range loop AssertEQ(data.fis_a(i), exp.fis_a(i)); end loop;
    AssertArrEQ(data.fis_a, exp.fis_a);

    AssertArrEQ(data.ls, exp.ls);
    --	__int64 fls(6);
    --for i in data.fls'Range loop AssertEQ(data.fls(i), exp.fls(i)); end loop;
    AssertArrEQ(data.fls, exp.fls);

    AssertArrEQ(data.fs, exp.fs);
    --    float ffs(77);
    --for i in data.ffs'Range loop AssertEQ(data.ffs(i), exp.ffs(i)); end loop;
    AssertArrEQ(data.ffs, exp.ffs);

    AssertArrEQ(data.ds, exp.ds);
    --    double fds(5);
    --for i in data.fds'range loop AssertEQ(data.fds(i), exp.fds(i)); end loop;
    AssertArrEQ(data.fds, exp.fds);

    AssertArrEQ(data.ss, exp.ss);
    --    std::string fss(10);
    --for i in data.fss'Range loop AssertEQ(data.fss(i), exp.fss(i)); end loop;
    AssertArrEQ(data.fss, exp.fss);

    AssertArrEQ(data.test2s, exp.test2s);
    --    TestData* ftest2s(5);
    --for i in data.ftest2s'range loop checkEmpty(data.ftest2s(i)); end loop;
    --for i in data.ftest2s'range loop checkObjects(data.ftest2s(i), exp.ftest2s(i)); end loop;
    AssertArrEQ(data.ftest2s, exp.ftest2s);

    AssertArrEQ(data.secondVirtArray, exp.secondVirtArray);
    --    TestData* fsecondVirtArray(7);
    --for i in data.fsecondVirtArray'Range loop checkEmpty(data.fsecondVirtArray(i)); end loop;
    --for i in data.fsecondVirtArray'Range loop checkObjects(data.fsecondVirtArray(i), exp.fsecondVirtArray(i)); end loop;
    AssertArrEQ(data.fsecondVirtArray, exp.fsecondVirtArray);

    AssertArrEQ(data.test2s2, exp.test2s2, "test2s2");
    --    TestData ftest2s2(4);
    --for i in data.ftest2s2'range loop checkObjects(data.ftest2s2(i), exp.ftest2s2(i)); end loop;
    AssertArrEQ(data.ftest2s2, exp.ftest2s2, "ftest2s2");

    AssertArrEQ(data.fruitarr, exp.fruitarr, "fruitarr");
    --AssertEQ(Int32(data.fruitarr'Length), 0);
    --    Fruit ffruitarr(15);
    --for i in data.ffruitarr'range loop checkObjects(data.ffruitarr(i), exp.ffruitarr(i)); end loop;
    AssertArrEQ(data.ffruitarr, exp.ffruitarr, "ffruitarr");

    Log("  Comparing objects finished");
  end;

  -- -----------------------------------------------------------------------------
  -- Fill ChildData with fixed values used for tests between languages
  procedure fillChildData(data: in out ChildData_Class_At) is
  begin
    -- BasedData
    --   baseText : String_At := null;
    data.baseText := Copy("dynamic string");
    --   stringOpenArr : String_Arr_At := null;
    data.stringOpenArr := new String_Arr(0..1);
    data.stringOpenArr(0) := Copy("dyn str 1");
    data.stringOpenArr(1) := Copy("dyn str 2");
    --   stringFixArr : String_Arr_At := new String_Arr'(0..4 => null);
    data.stringFixArr(0) := Copy("dsf 0");
    data.stringFixArr(1) := Copy("dsf 1");
    data.stringFixArr(2) := Copy("dsf 2");
    data.stringFixArr(3) := Copy("dsf 3");
    data.stringFixArr(4) := Copy("dsf 4");
    --   fixLengthString : String_At := null;
    data.fixLengthString := Copy("fixed length string");
    --   fixLengthStringOpenArr : String_Arr_At := null;
    data.fixLengthStringOpenArr := new String_Arr(0..1);
    data.fixLengthStringOpenArr(0) := Copy("fix len str 1");
    data.fixLengthStringOpenArr(1) := Copy("fix len str 2");
    --   fixLengthStringFixArr : String_Arr_At := new String_Arr'(0..9 => null);
    data.fixLengthStringFixArr(0) := Copy("fsf 0");
    data.fixLengthStringFixArr(1) := Copy("fsf 1");
    data.fixLengthStringFixArr(2) := Copy("fsf 2");
    data.fixLengthStringFixArr(3) := Copy("fsf 3");
    data.fixLengthStringFixArr(4) := Copy("fsf 4");
    data.fixLengthStringFixArr(5) := Copy("fsf 5");
    data.fixLengthStringFixArr(6) := Copy("fsf 6");
    data.fixLengthStringFixArr(7) := Copy("fsf 7");
    data.fixLengthStringFixArr(8) := Copy("fsf 8");
    data.fixLengthStringFixArr(9) := Copy("fsf 9");

    -- ChildData
    -- enums
    data.enu1 := GHI;
    data.enuVec := new Order_Arr(0..4);
    data.enuVec(0) := GHI;
    data.enuVec(1) := JKL;
    data.enuVec(2) := JKL;
    data.enuVec(3) := ABC;
    data.enuVec(4) := DEF;

    data.enuFixArr(0) := DEF;
    data.enuFixArr(4) := JKL;
    data.enuFixArr(5) := DEF;

    data.cmd := CONTINUE;
    data.cmds(0) := PAUSE;
    data.cmds(1) := STOP;

    -- core types
    data.bo := true;
    data.b := 7;
    data.sh := -99;
    data.i := 19;
    data.l := 3456789;
    data.f := 123.4567;
    data.d := 987.12345678901;
    Replace(data.s, "Test of [de]serializing");

    if data.test2 /= null then
      Free(data.test2);
    end if;
    data.test2 := Create;
    Replace(data.test2.text, "TestData");
    data.test2.value := 555.5;

    if data.testPointer /= null then
      Free(data.testPointer);
    end if;
    data.testPointer := Create;
    data.testPointer.text := Copy("TestPtr");
    data.testPointer.value := 777.7;

    if data.fruit /= null then
      Free(data.fruit);
    end if;
    data.fruit := Create;
    data.fruit.value := Ops_Pa.OpsObject_Pa.TestAll_Fruit.PEAR;

    if data.bos /= null then
      Dispose(data.bos);
    end if;
    data.bos := new Boolean_Arr(0..2);
    data.bos(0) := false;
    data.bos(1) := true;
    data.bos(2) := false;

    -- bool fbos(11);
    for i in data.fbos'range loop data.fbos(i) := false; end loop;
    data.fbos(5) := true;
    data.fbos(10) := true;

    if data.bs /= null then
      Dispose(data.bs);
    end if;
    data.bs := new Byte_Arr(0..2);
    data.bs(0) := 10;
    data.bs(1) := 20;
    data.bs(2) := 30;

    -- char fbs(256);
    for i in data.fbs'range loop data.fbs(i) := Byte(i); end loop;

    if data.shs /= null then
      Dispose(data.shs);
    end if;
    data.shs := new Int16_Arr(0..1);
    data.shs(0) := 1111;
    data.shs(1) := 2222;

    -- short fshs(4);
    data.fshs(0) := 21;
    data.fshs(1) := 121;
    data.fshs(2) := 221;
    data.fshs(3) := 321;

    if data.is_a /= null then
      Dispose(data.is_a);
    end if;
    data.is_a := new Int32_Arr(0..3);
    data.is_a(0) := 100000;
    data.is_a(1) := 101010;
    data.is_a(2) := 110101;
    data.is_a(3) := 111111;

    -- int fis_(3);
    data.fis_a(0) := -1;
    data.fis_a(1) := -2;
    data.fis_a(2) := -3;

    if data.ls /= null then
      Dispose(data.ls);
    end if;
    data.ls := new Int64_Arr(0..3);
    data.ls(0) := 9;
    data.ls(1) := 8;
    data.ls(2) := 7;
    data.ls(3) := 6;

    -- __int64 fls(6);
    data.fls(0) := 9999;
    data.fls(1) := 9998;
    data.fls(2) := 9997;
    data.fls(3) := 9996;
    data.fls(4) := 9995;
    data.fls(5) := 9994;

    if data.fs /= null then
      Dispose(data.fs);
    end if;
    data.fs := new Float32_Arr(0..3);
    data.fs(0) := 3.1;
    data.fs(1) := 31.14;
    data.fs(2) := 4.56;
    data.fs(3) := 987.0;

    -- float ffs(77);
    for i in data.ffs'range loop data.ffs(i) := 0.0; end loop;
    data.ffs(21) := 3.1415;

    if data.ds /= null then
      Dispose(data.ds);
    end if;
    data.ds := new Float64_Arr(0..1);
    data.ds(0) := 1.987654321;
    data.ds(1) := 2.3456789;

    -- double fds(5);
    data.fds(0) := 1.1;
    data.fds(1) := 2.1;
    data.fds(2) := 3.1;
    data.fds(3) := 4.1;
    data.fds(4) := 5.1;

    if data.ss /= null then
      Clear(data.ss.all);
      Dispose(data.ss);
    end if;
    data.ss := new String_Arr(0..2);
    data.ss(0) := Copy("Index 0");
    data.ss(1) := Copy("Index 1");
    data.ss(2) := Copy("Index 2");

    -- std::string fss(10);
    Clear(data.fss.all);
    data.fss(4) := Copy("4 string");
    data.fss(7) := Copy("7 string");
    data.fss(9) := Copy("9 string");

    -- std::vector<TestData*> test2s;
    if data.test2s /= null then
      Clear(data.test2s.all);
      Dispose(data.test2s);
    end if;
    data.test2s := new TestData_Class_At_Arr(0..3);
    data.test2s(0) := Create;
    data.test2s(1) := Create;
    data.test2s(2) := Create;
    data.test2s(3) := Create;

    -- TestData* ftest2s(5);
    data.ftest2s(2).text := Copy("Index 2");
    data.ftest2s(2).value := 7.7;

    -- std::vector<TestData*> secondVirtArray;
    if data.secondVirtArray /= null then
      Clear(data.secondVirtArray.all);
      Dispose(data.secondVirtArray);
    end if;
    data.secondVirtArray := new TestData_Class_At_Arr(0..1);
    for i in data.secondVirtArray'Range loop
      data.secondVirtArray(i) := Create;
    end loop;

    -- TestData* fsecondVirtArray(7);
    data.fsecondVirtArray(5).text := Copy("Index 5");
    data.fsecondVirtArray(5).value := -9.99;

    -- std::vector<TestData> test2s2;
    if data.test2s2 /= null then
      Clear(data.test2s2.all);
      Dispose(data.test2s2);
    end if;
    data.test2s2 := new TestData_Class_At_Arr(0..10);
    for i in data.test2s2'Range loop
      data.test2s2(i) := Create;
    end loop;

    -- TestData ftest2s2(4);
    Replace(data.ftest2s2(3).text, "");
    data.ftest2s2(1).value := 710.6;

    if data.fruitarr /= null then
      Clear(data.fruitarr.all);
      Dispose(data.fruitarr);
    end if;
    data.fruitarr := new Fruit_Class_At_Arr(0..1);
    data.fruitarr(0) := Create;
    data.fruitarr(1) := Create;
    data.fruitarr(0).value := Ops_Pa.OpsObject_Pa.TestAll_Fruit.PEAR;
    data.fruitarr(1).value := Ops_Pa.OpsObject_Pa.TestAll_Fruit.BANANA;

    -- Fruit ffruitarr(15);
    data.ffruitarr(0).value := Ops_Pa.OpsObject_Pa.TestAll_Fruit.PEAR;
    data.ffruitarr(14).value := Ops_Pa.OpsObject_Pa.TestAll_Fruit.PEAR;
  end;

-- -----------------------------------------------------------------------------
  procedure ErrorLog(Sender : in out Ops_Class_At; Item : Ops_Pa.Error_Pa.Error_Class_At; Arg : Ops_Class_At) is
  begin
    Log("Error:: " & Item.GetMessage);
  end;

  gTerminate : Boolean := False;
  pragma Volatile(gTerminate);

  procedure My_Ctrl_C_Handler is
  begin
    gTerminate := True;
  end;

  procedure setup_alt_config(cfg_rel_ops4 : String) is
  begin
    -- Check if we have an ops_config.xml in CWD
    if Exists("ops_config.xml") then
      Put_Line("Using config file in CWD");

    else
      declare
        cwd : String := Current_Directory;
        pos : Natural := Index(cwd, "Examples", 1);
        dummy : Boolean;
      begin
        if pos > 1 then
          dummy := RepositoryInstance.add(Head(cwd, pos - 1) & cfg_rel_ops4);
          Put_Line("Using config file: " & Head(cwd, pos - 1) & cfg_rel_ops4);
        end if;
      end;
    end if;
  end;

  procedure VerifySerDes_Internal is
    cd1, cd2, cd3 : Ops_Pa.OpsObject_Pa.TestAll_ChildData.ChildData_Class_At;
    FMap : MemoryMap_Class_At := null;
    FBuf : ByteBuffer_Class_At := null;
    ao : ArchiverOut_Class_At := null;
  begin
    Log("Creating test object cd1 ...");
    cd1 := Create;
    Log("Creating test object cd2 ...");
    cd2 := Create;
    cd3 := null;

    Log("Test initial state...");
    checkEmpty(cd1);
    checkEmpty(cd2);

    checkObjects(cd1, cd2);
    Log("Initial state test finished");

    Log("Test cloning of empty object...");
    cd3 := ChildData_Class_At(cd1.Clone);
    Log("  Object cloned");
    checkEmpty(cd3);
    checkObjects(cd1, cd3);
    Log("Cloning test of empty object finished");

    Log("Serialize empty object");
    FMap := Create(1, 65536);
    FBuf := Create(FMap);
    ao := Create(FBuf, False);

    Put_line("  GetSize()= " & UInt32'Image(FBuf.GetSize));
    ao.inout("data", Serializable_Class_At(cd1));
    Put_Line("  GetSize()= " & UInt32'Image(FBuf.GetSize));

    Free(ao);
    Free(FBuf);
    Free(FMap);
    Log("Serialize finished");

    Log("Test cloning of filled object...");
    Log("  Fill cd1 ...");
    fillChildData(cd1);
    Log("  Object filled, Cloning...");

    cd1.fillClone(obj => Ops_Pa.OpsObject_Pa.OpsObject_Class_At(cd2));
    Log("  Object cloned");

    checkObjects(cd1, cd2);

    -- Delete first object, recreate it and compare again
    -- This to check that cd2 is really a deep clone (no common object)
    Log("  Free(cd1) ...");
    Free(cd1);
    Log("  cd1 := Create ...");
    cd1 := Create;
    Log("  Fill cd1 ...");
    fillChildData(cd1);
    Log("  cd1 filled");

    checkObjects(cd1, cd2);

    Log("Cloning test of filled object finished");

    Log("Serialize filled object");
    FMap := Create(1, 65536);
    FBuf := Create(FMap);
    ao := Create(FBuf, False);

    Put_line("  GetSize()= " & UInt32'Image(FBuf.GetSize));
    ao.inout("data", Serializable_Class_At(cd1));
    Put_Line("  NonVirtOpt = False, GetSize()= " & UInt32'Image(FBuf.GetSize));
    AssertEQ(Int32(FBuf.GetSize), Int32(3150), "Serialized size error");

    Free(ao);
    Free(FBuf);
    Free(FMap);

    FMap := Create(1, 65536);
    FBuf := Create(FMap);
    ao := Create(FBuf, True);

    Put_line("  GetSize()= " & UInt32'Image(FBuf.GetSize));
    ao.inout("data", Serializable_Class_At(cd1));
    Put_Line("  NonVirtOpt = True,  GetSize()= " & UInt32'Image(FBuf.GetSize));
    AssertEQ(Int32(FBuf.GetSize), Int32(2591), "Serialized size error");
    Log("Serialize finished");

--      declare
--        prt : PrintArchiverOut_Class_At := Create;
--      begin
--        prt.PrintObject("Kalle", Serializable_Class_At(cd1));
--        Free(prt);
--      end;

--    Put_Line("Debug: Count= " & Ctv.Integer32'Image(Ops_Pa.NumActiveObjects));

    Log("Test Checksum Archiver ...");
    declare
      calc : Checksum_Calc_8bit_xor_Class_At := Create;
      chksum : ChecksumArchiver_Class_At := Create( Checksum_Calculator_Class_At(calc) );
    begin
      cd1.Serialize( ArchiverInOut_Class_At(chksum) );
      Free(chksum);
      AssertEQ(calc.Sum, 140, "Wrong checksum");
      Log("Checksum # fields  = " & UInt32'Image(calc.TotalFields));
      Log("Checksum # bytes   = " & UInt32'Image(calc.TotalBytes));
      Log("Checksum 8-bit XOR = " & Byte'Image(calc.Sum));
      Free(calc);
    exception
      when others =>
        ErrorLog("Checksum exception !!!");
    end;
    Log("Checksum Archiver finished");

--    Put_Line("Debug: Count= " & Ctv.Integer32'Image(Ops_Pa.NumActiveObjects));

    Log("Test publish/subscribe ...");

    Ops_Pa.Error_Pa.StaticErrorService.addListener(Proc => ErrorLog'Access, Arg => null);

    setup_alt_config("Examples/OPSIdls/TestAll/ops_config.xml");

    declare
      part : Ops_Pa.Participant_Pa.Participant_Class_At := null;
      top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At := null;
      use type Ops_Pa.Participant_Pa.Participant_Class_At;
    begin
      part := Ops_Pa.Participant_Pa.getInstance("TestAllDomain");
      if part = null then
        ErrorLog("#### Failed to create Participant");
        return;
      end if;
      declare
        fact : TestAllTypeFactory_Class_At := Create;
      begin
        part.addTypeSupport( SerializableFactory_Pa.SerializableFactory_Class_At(fact) );
      end;

      -- Add an error listener to get ev. error reports
      part.getErrorService.addListener(Proc => ErrorLog'Access, Arg => null);

      top := part.getTopic("ChildTopic");

      declare
        sub : Ops_Pa.Subscriber_Pa.TestAll_ChildData.ChildDataSubscriber_Class_At := null;
        pub : Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData.ChildDataPublisher_Class_At := null;
        flag : Boolean;
        use type Ops_Pa.OpsObject_Pa.OPSMessage_Pa.OPSMessage_Class_At;
      begin
        -- Setup & start subscriber w polling
        sub := Create( top );
        sub.Start;

        -- Setup & start publisher
        pub := Create( top );
        pub.SetName("Ada");
        pub.Start;

        -- Publish data
        pub.write( cd1 );

        -- Check that sent data isn't affected by publish
        checkObjects( cd1, cd2 );

        -- Check received values against sent values
        AssertEQ(sub.waitForNewData(100), True, "No data received");
        flag := AssertEQ(sub.getData(cd3), True, "No received data");

        declare
          s : Ops_Pa.Subscriber_Pa.Scope_MessageLock( Subscriber_Class_At(sub) );
        begin
          if sub.getMessage /= null then
            AssertPtrEQ(sub.getMessage.SpareBytes, null, "spareBytes");
          else
            ErrorLog("### Failed: sub.getMessage() == NULL");
          end if;
        end;

        if flag then
          checkObjects(cd1, cd3);
        end if;

        Log("  Waiting for more data for 60 seconds ... (Press Ctrl-C to terminate)");
        declare
          stopTime : Ops_Pa.TimeMs_T := Ops_Pa.GetTimeInMs + 60000;
          pubTime : Ops_Pa.TimeMs_T := Ops_Pa.GetTimeInMs + 5000;
        begin
          while not gTerminate and Ops_Pa.GetTimeInMs < stopTime loop
            if (sub.waitForNewData(100)) then
              declare
                msg : Ops_Pa.OpsObject_Pa.OPSMessage_Pa.OPSMessage_Class_At;
              begin
                sub.acquireMessageLock;
                msg := sub.getMessage;
                Log("Received new data from " & msg.PublisherName & ". Checking...");
                sub.releaseMessageLock;
              end;
              flag := sub.getData(cd3);
              declare
                s : Ops_Pa.Subscriber_Pa.Scope_MessageLock( Subscriber_Class_At(sub) );
              begin
                AssertPtrEQ(sub.getMessage.SpareBytes, null, "spareBytes");
              end;
              checkObjects(cd3, cd1);
              Log("Data check done");

            end if;
            if Ops_Pa.GetTimeInMs >= pubTime then
              pubTime := Ops_Pa.GetTimeInMs + 5000;
              pub.Write( cd1 );
            end if;
          end loop;
        end;

        pub.Stop;
        sub.Stop;

        delay 1.0;
        Free(pub);
        Free(sub);
      end;

      Ops_Pa.Participant_Pa.releaseInstance(part);
    end;
    Log("Test publish/subscribe finished");

    Log("Freeing test objects...");
    Free(cd1);
    Free(cd2);
    Free(cd3);
    Free(ao);
    Free(FBuf);
    Free(FMap);
    Log("Freeing finished");

    -- Test when looking for memory leaks
    Ops_Pa.Error_Pa.Debug_TotalClear;
    Ops_Pa.OpsObject_Pa.OPSConfig_Pa.Debug_TotalClear;
    Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa.Debug_TotalClear;
  end;

  procedure VerifySerDes is
  begin
    GNAT.Ctrl_C.Install_Handler(My_Ctrl_C_Handler'Access);
    VerifySerDes_Internal;
    Log("");
    if gErrorCount > 0 then
      Log("  !!!! " & Integer'Image(gErrorCount) & " ERRORS occurred !!!! ");
    else
      Log("  VERFIFY == OK ");
    end if;
    Log("");
    Log("Sleeping for 10 seconds...");
    delay 10.0;
    GNAT.Ctrl_C.Uninstall_Handler;
  exception
    when others =>
      GNAT.Ctrl_C.Uninstall_Handler;
  end;

end;

