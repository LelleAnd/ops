--
-- Copyright (C) 2017 Lennart Andersson.
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
with Ada.Text_Io;
with VerifySerDes_Pa;
with Interfaces;
with Ops_Pa;

use Ada.Text_IO;
use VerifySerDes_Pa;
use Interfaces;

procedure VerifySerDes_Main is
begin
  Put_Line("Debug: Count= " & System.Atomic_Counters.Atomic_Unsigned'Image(Ops_Pa.NumActiveObjects));
  VerifySerDes;
  Put_Line("Debug: Count= " & System.Atomic_Counters.Atomic_Unsigned'Image(Ops_Pa.NumActiveObjects));
end;

