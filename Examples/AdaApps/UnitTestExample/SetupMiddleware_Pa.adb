
with Text_IO; use Text_IO;

with Ops_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.Participant_Pa,
     Ops_Pa.SerializableFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.TestAll_TestAllTypeFactory;

use Ops_Pa,
    Ops_Pa.Error_Pa,
    Ops_Pa.Participant_Pa,
    Ops_Pa.SerializableFactory_Pa,
    Ops_Pa.SerializableFactory_Pa.TestAll_TestAllTypeFactory;

package body SetupMiddleware_Pa is
  
  procedure ErrorLog(Sender : in out Ops_Class_At; Item : Ops_Pa.Error_Pa.Error_Class_At; Arg : Ops_Class_At) is
  begin
    Put_Line("Error:: " & Item.GetMessage);
  end;

  participant : Participant_Class_At := null;

  function Setup return Ops_Pa.Participant_Pa.Participant_Class_At is
  begin
    -- Setup the OPS static error service (common for all participants, reports
    -- errors during participant creation)
    StaticErrorService.addListener(ErrorLog'Access, null);
    
    -- Create OPS participant to access a domain in the default configuration file
    -- "ops_config.xml" in current working directory. There are other overloads to
    -- create a participant for a specific configuration file.
    participant := getInstance("TestAllDomain");
    if participant = null then
      Put_Line("participant == NULL");
      return null;
    end if;

    -- Setup the participant errorService to catch ev. internal OPS errors
    participant.getErrorService.addListener(ErrorLog'Access, null);

    -- Add our generated factory so OPS can create our data objects
    declare
      fact : TestAllTypeFactory_Class_At := Create;
    begin
      participant.addTypeSupport( SerializableFactory_Class_At(fact) );
    end;

    return participant;
    
  exception
    when others =>
      Put_Line("Exception");
      return null;
  end;
  
  procedure Cleanup is
  begin
    Free(participant);
  end;
  
end;

