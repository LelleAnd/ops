--
-- Copyright (C) 2016-2021 Lennart Andersson.
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

with Ops_Pa.Transport_Pa.SendDataHandler_Pa,
     Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa;

use Ops_Pa.Transport_Pa.SendDataHandler_Pa,
    Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa,
    Ops_Pa.Error_Pa,
    Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.Participant_Interface_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Participant_Interface is limited interface;
  type Participant_Interface_At is access all Participant_Interface'Class;

  -- Should only be used by Publishers
  function getSendDataHandler( Self: in out Participant_Interface; top : Topic_Class_At) return SendDataHandler_Class_At is abstract;
  procedure updateSendPartInfo( Self: in out Participant_Interface; top : Topic_Class_At ) is abstract;
  procedure releaseSendDataHandler( Self: in out Participant_Interface; top : Topic_Class_At ) is abstract;

  -- Should only be used by Subscribers
  function getReceiveDataHandler( Self: in out Participant_Interface; top : Topic_Class_At) return ReceiveDataHandler_Class_At is abstract;
  procedure releaseReceiveDataHandler( Self: in out Participant_Interface; top : Topic_Class_At ) is abstract;

  procedure ReportError( Self : in out Participant_Interface; Error : Error_Class_At ) is abstract;

end Ops_Pa.Participant_Interface_Pa;

