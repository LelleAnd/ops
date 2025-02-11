
with Ops_Pa.Participant_Pa;

package SetupMiddleware_Pa is
  
  function Setup return Ops_Pa.Participant_Pa.Participant_Class_At;
  procedure Cleanup;
  
end;

