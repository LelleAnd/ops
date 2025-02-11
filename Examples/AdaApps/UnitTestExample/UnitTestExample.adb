
with Text_IO; use Text_IO;
--with Ada.Command_Line; use Ada.Command_Line;
--with Ada.Directories; use Ada.Directories;
--with Ada.Strings.Fixed; use Ada.Strings.Fixed;

with OPs_Pa.Participant_Pa;

with SetupMiddleware_Pa;
with ClassUnderTest_Test_Pa;

procedure UnitTestExample is
  participant : Ops_Pa.Participant_Pa.Participant_Class_At := null;
begin
  participant := SetupMiddleware_Pa.Setup;
 
  ClassUnderTest_Test_Pa.PerformTest(participant);
    
  SetupMiddleware_Pa.Cleanup;
  
  Put_Line("Exit UnitTestExample");
  
exception
  when others =>
    Put_Line("Exception");
end;

