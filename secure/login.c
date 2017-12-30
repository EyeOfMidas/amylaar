
#include <config.h>

void
create()
{
  seteuid(ROOT_EUID);
}

void
logon()
{
  cat(WELCOME);
  write("Login: ");
  input_to("get_name");
}

void
get_name(string name)
{
  object new_ob;

  if(!name)
  {
    write("Login: ");
    input_to("get_name");
  }
  name = lower_case(name);
  new_ob = clone_object(PLAYER_OBJ);
  exec(new_ob, this_object());
  new_ob->enter_game(name);
  destruct(this_object());
}
