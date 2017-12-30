
inherit "/obj/living";

#include <config.h>

static mapping commands = ([ ]);

void
restore_me()
{
  return 0;
}

void 
save_me()
{
  return 0;
}

int
move_player(mixed dest)
{
  ::move_player(dest);
}



void
add_commands()
{
  string *files;
  string  com;
  int     i;

  commands = ([ /* empty */ ]);
  files = get_dir(BIN_DIR + "/*.c");
  for(i = 0; i < sizeof(files); i++)
  {
    com = files[i];
    sscanf(com, "%s.c", com);
    commands[com] = BIN_DIR + "/" + files[i];
  }
  add_action("command_hook", "", 1);
}

int
command_hook(string arg)
{
  string cmd;

  cmd = commands[query_verb()];
  if(cmd)
  {
    return call_other(cmd, "main", query_verb(), arg);
  }
  return 0;
}

int
enter_game(string my_name)
{
  real_name = my_name;
  restore_me();
  add_commands();
  move_player(START);
}

string
query_short()
{
  return capitalize(real_name);
}

void
catch_message(string type, string mess)
{
  tell_object(this_object(), mess);
}
