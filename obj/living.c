inherit "obj/object";

#include <config.h>

string real_name;

string
query_real_name()
{
  return real_name;
}

int
move_player(mixed dest)
{
  move(dest);
  return 1;
}
