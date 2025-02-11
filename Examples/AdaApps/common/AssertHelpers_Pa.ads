--
-- Copyright (C) 2017-2025 Lennart Andersson.
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

with Ops_Pa;
use Ops_Pa;

with AssertGenerics_Pa; use AssertGenerics_Pa;

package AssertHelpers_Pa is

  -- Result
  function GetNumErrors return Integer;

  -- Booleans
  function AssertEQ(val, exp : Boolean; str : String := "") return Boolean;
  procedure AssertEQ(val, exp : Boolean; str : String := "");

  -- Strings
  procedure AssertEQ(val, exp : String_At; str : String := "");

  -- Ints
  --  generic
  --    type Item is (<>);
  --  procedure AssertIntEQ(val, exp : Item; str : String := "");
  procedure AssertEQ is new AssertIntEQ(Byte);
  procedure AssertEQ is new AssertIntEQ(Int8);
  procedure AssertEQ is new AssertIntEQ(Int16);
  procedure AssertEQ is new AssertIntEQ(Int32);
  procedure AssertEQ is new AssertIntEQ(Int64);

  -- Enum types, example
  -- procedure AssertEQ is new AssertIntEQ(Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order);

  -- Floats
  --  generic
  --    type Item is digits <>;
  --  procedure AssertFloatEQ(val, exp : Item; str : String := "");
  procedure AssertEQ is new AssertFloatEQ(Float32);
  procedure AssertEQ is new AssertFloatEQ(Float64);

  -- Array pointers
  --  generic
  --    type Item is private;
  --    type Item_Arr is array(Integer range <>) of Item;
  --    type Item_Arr_At is access all Item_Arr;
  --  procedure AssertAccessEQ0(val, exp : Item_Arr_At; str : string := "");
  procedure AssertPtrEQ is new AssertAccessEQ0(Boolean, Boolean_Arr, Boolean_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Int16, Int16_Arr, Int16_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Int32, Int32_Arr, Int32_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Int64, Int64_Arr, Int64_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Float32, Float32_Arr, Float32_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(Float64, Float64_Arr, Float64_Arr_At);
  procedure AssertPtrEQ is new AssertAccessEQ0(String_At, String_Arr, String_Arr_At);

  -- Enum array pointers, example
  -- procedure AssertPtrEQ is new AssertAccessEQ0(Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order,
  --                                              Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr,
  --                                              Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr_At);

  -- Array pointers
  --  generic
  --    type Item is private;
  --    type Item_Arr is array(Byte_Arr_Index_T range <>) of Item;
  --    type Item_Arr_At is access all Item_Arr;
  --  procedure AssertAccessEQ10(val, exp : Item_Arr_At; str : string := "");
  procedure AssertPtrEQ is new AssertAccessEQ10(Byte,  Byte_Arr,  Byte_Arr_At);

  -- Class pointers
  --  generic
  --    type Item is tagged limited private;
  --    type Item_At is access all Item'Class;
  --  procedure ExceptionWhenNull(val : Item_At; str : String := "");

  --  generic
  --    type Item is tagged limited private;
  --    type Item_At is access all Item'Class;
  --  procedure AssertAccessEQ(val, exp : Item_At; str : String := "");

  --  generic
  --    type Item is tagged limited private;
  --    type Item_At is access all Item'Class;
  --  function AssertAccessNEQ(val, exp : Item_At; str : String := "") return Boolean;

  --  Example
  --    procedure AssertPtrEQ is new AssertAccessEQ(TestData_Class, TestData_Class_At);
  --    function AssertPtrNEQ is new AssertAccessNEQ(TestData_Class, TestData_Class_At);

  -- Class Array Pointers
  --  generic
  --    type Item is tagged limited private;
  --    type Item_At is access all Item'Class;
  --    type Item_At_Arr is array(Integer range <>) of Item_At;
  --    type Item_At_Arr_At is access all Item_At_Arr;
  --  procedure AssertAccessEQ3(val, exp : Item_At_Arr_At; str : String := "");

  -- Example
  --   procedure AssertPtrEQ is new AssertAccessEQ3(TestData_Class, TestData_Class_At, TestData_Class_At_Arr, TestData_Class_At_Arr_At);

  --  generic
  --    type Item is tagged limited private;
  --    type Item_Arr is array(Integer range <>) of Item;
  --    type Item_Arr_At is access all Item_Arr;
  --  procedure AssertAccessEQ4(val, exp : Item_Arr_At; str : String := "");

  -- Example
  --   procedure AssertPtrEQ is new AssertAccessEQ4(TestData_Class, TestData_Class_Arr, TestData_Class_Arr_At);

  -- Array content
  --  generic
  --    type Item is private;
  --    type Item_Arr is array(Integer range <>) of Item;
  --    type Item_Arr_At is access all Item_Arr;
  --    with procedure AssertItem(val, exp : Item; str : string);
  --  procedure AssertArrayEQ0(val, exp : Item_Arr_At; str : string := "");
  procedure AssertArrEQ is new AssertArrayEQ0(Boolean, Boolean_Arr, Boolean_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Int16, Int16_Arr, Int16_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Int32, Int32_Arr, Int32_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Int64, Int64_Arr, Int64_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Float32, Float32_Arr, Float32_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(Float64, Float64_Arr, Float64_Arr_At, AssertEQ);
  procedure AssertArrEQ is new AssertArrayEQ0(String_At, String_Arr, String_Arr_At, AssertEQ);

  -- Enum array content, example
  -- procedure AssertArrEQ is new AssertArrayEQ0(Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order,
  --                                             Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr,
  --                                             Ops_Pa.OpsObject_Pa.TestAll_ChildData.Order_Arr_At,
  --                                             AssertEQ);

  -- Array content
  --  generic
  --    type Item is private;
  --    type Item_Arr is array(Byte_Arr_Index_T range <>) of Item;
  --    type Item_Arr_At is access all Item_Arr;
  --    with procedure AssertItem(val, exp : Item; str : string);
  --  procedure AssertArrayEQ10(val, exp : Item_Arr_At; str : string := "");
  procedure AssertArrEQ is new AssertArrayEQ10(Byte,  Byte_Arr,  Byte_Arr_At, AssertEQ);

  -- Class array content
  --  generic
  --    type Item is tagged limited private;
  --    type Item_At is access all Item'Class;
  --    type Item_At_Arr is array(Integer range <>) of Item_At;
  --    type Item_At_Arr_At is access all Item_At_Arr;
  --    with procedure AssertItem(val, exp : Item_At; str : String);
  --  procedure AssertArrayEQ3(val, exp : Item_At_Arr_At; str : String := "");

  --  generic
  --    type Item is tagged limited private;
  --    type Item_Arr is array(Integer range <>) of Item;
  --    type Item_Arr_At is access all Item_Arr;
  --    with procedure AssertItem(val, exp : Item; str : String);
  --  procedure AssertArrayEQ4(val, exp : Item_Arr_At; str : String := "");

end;

