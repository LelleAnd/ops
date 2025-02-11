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

with Ops_Pa; use Ops_Pa;

package AssertGenerics_Pa is

  gErrorCount : Integer := 0;

  PointerIsNull : exception;

  -- Logging
  procedure ErrorLog(str : String);

  -- Ints
  generic
    type Item is (<>);
  procedure AssertIntEQ(val, exp : Item; str : String := "");

  -- Floats
  generic
    type Item is digits <>;
  procedure AssertFloatEQ(val, exp : Item; str : String := "");

  -- Array pointers
  generic
    type Item is private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
  procedure AssertAccessEQ0(val, exp : Item_Arr_At; str : String := "");

  generic
    type Item is private;
    type Item_Arr is array(Byte_Arr_Index_T range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
  procedure AssertAccessEQ10(val, exp : Item_Arr_At; str : String := "");

  -- Array content
  generic
    type Item is private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
    with procedure AssertItem(val, exp : Item; str : String);
  procedure AssertArrayEQ0(val, exp : Item_Arr_At; str : String := "");

  generic
    type Item is private;
    type Item_Arr is array(Byte_Arr_Index_T range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
    with procedure AssertItem(val, exp : Item; str : String);
  procedure AssertArrayEQ10(val, exp : Item_Arr_At; str : String := "");

  -- Class pointers
  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
  procedure ExceptionWhenNull(val : Item_At; str : String := "");

  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
  procedure AssertAccessEQ(val, exp : Item_At; str : String := "");

  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
  function AssertAccessNEQ(val, exp : Item_At; str : String := "") return Boolean;

  -- Class array pointers
  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
    type Item_At_Arr is array(Integer range <>) of Item_At;
    type Item_At_Arr_At is access all Item_At_Arr;
  procedure AssertAccessEQ3(val, exp : Item_At_Arr_At; str : String := "");

  generic
    type Item is tagged limited private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
  procedure AssertAccessEQ4(val, exp : Item_Arr_At; str : String := "");

  -- Class array content
  generic
    type Item is tagged limited private;
    type Item_At is access all Item'Class;
    type Item_At_Arr is array(Integer range <>) of Item_At;
    type Item_At_Arr_At is access all Item_At_Arr;
    with procedure AssertItem(val, exp : Item_At; str : String);
  procedure AssertArrayEQ3(val, exp : Item_At_Arr_At; str : String := "");

  generic
    type Item is tagged limited private;
    type Item_Arr is array(Integer range <>) of Item;
    type Item_Arr_At is access all Item_Arr;
    with procedure AssertItem(val, exp : Item; str : String);
  procedure AssertArrayEQ4(val, exp : Item_Arr_At; str : String := "");

end;

