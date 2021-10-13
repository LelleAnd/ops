
with Text_IO; use Text_IO;
with Ada.Command_Line; use Ada.Command_Line;

with GenSubscriber_Pa; use GenSubscriber_Pa;

procedure GenSub is

  procedure Usage is
  begin
    Put_Line("");
    Put_Line("Usage: Gensub ops_config Domain Topic");
    Put_Line("");
  end;

begin
  begin
    SetUpErrorLog;

    --Listener("D:\OPS\ops4\Examples\OPSIdls\PizzaProject\ops_config.xml", "PizzaDomain", "PizzaTopic");

    if Argument_Count >= 3 then
      declare
        cfgfile : String := Argument(1);
        domain  : String := Argument(2);
        topic   : String := Argument(3);
      begin
        Listener(cfgfile, domain, topic);
      end;
    else
      Usage;
    end if;
  exception
    when others =>
      Put_Line("Exception");
  end;
end;
