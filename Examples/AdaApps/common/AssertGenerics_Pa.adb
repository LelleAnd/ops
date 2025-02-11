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

with Text_IO; use Text_IO;

package body AssertGenerics_Pa is

  -- Access to operations on types
  use type Ops_Pa.UInt32;
  use type Ops_Pa.Int16;
  use type Ops_Pa.Int32;
  use type Ops_Pa.Float64;

  -- Logging
  procedure Log(str : string) is
  begin
    Put_Line(str);
  end;

  procedure ErrorLog(str : String) is
  begin
    Log("### Failed: " & str);
    gErrorCount := gErrorCount + 1;
  end;

  -- ---------------------

  procedure AssertIntEQ(val, exp : Item; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Item'Image(val) & ", expected= " & Item'Image(exp));
    end if;
  end;

  -- ---------------------

  procedure AssertFloatEQ(val, exp : Item; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Item'Image(val) & ", expected= " & Item'Image(exp));
    end if;
  end;

  -- ---------------------

  procedure ExceptionWhenNull(val : Item_At; str : String := "") is
  begin
    if val = null then
      ErrorLog(str & ", Pointer is null");
      raise PointerIsNull;
    end if;
  end;

  procedure AssertAccessEQ(val, exp : Item_At; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  function AssertAccessNEQ(val, exp : Item_At; str : String := "") return Boolean is
  begin
    if val = exp and val /= null then
      ErrorLog(str & ", Pointers are equal");
      return False;
    end if;
    return True;
  end;

  procedure AssertAccessEQ0(val, exp : Item_Arr_At; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertAccessEQ10(val, exp : Item_Arr_At; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertAccessEQ3(val, exp : Item_At_Arr_At; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  procedure AssertAccessEQ4(val, exp : Item_Arr_At; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", Pointers are different");
    end if;
  end;

  -- ---------------------

  procedure AssertArrayEQ0(val, exp : Item_Arr_At; str : String := "") is
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

  procedure AssertArrayEQ10(val, exp : Item_Arr_At; str : String := "") is
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

  procedure AssertArrayEQ3(val, exp : Item_At_Arr_At; str : String := "") is
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

  procedure AssertArrayEQ4(val, exp : Item_Arr_At; str : String := "") is
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

end;

