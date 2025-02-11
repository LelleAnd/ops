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

package body AssertHelpers_Pa is

  -- Access to operations on types
  use type Ops_Pa.UInt32;
  use type Ops_Pa.Int16;
  use type Ops_Pa.Int32;
  use type Ops_Pa.Float64;

  function GetNumErrors return Integer is
  begin
    return gErrorCount;
  end;

  -- Booleans
  function AssertEQ(val, exp : Boolean; str : String := "") return Boolean is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Boolean'Image(val) & ", expected= " & Boolean'Image(exp));
    end if;
    return val;
  end;

  procedure AssertEQ(val, exp : Boolean; str : String := "") is
  begin
    if val /= exp then
      ErrorLog(str & ", value= " & Boolean'Image(val) & ", expected= " & Boolean'Image(exp));
    end if;
  end;

  -- ---------------------

  procedure AssertEQ(val, exp : String_At; str : String := "") is
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


end;

