

int
main(string cmd, string args)
{
  object *inv;

  inv = all_inventory(environment(this_player()));
  write(environment(this_player())->query_long());
  map_array(inv - ({ this_player() }), "map_fun", this_object());
  return 1;
}

void
map_fun(object ob)
{
  write(ob->query_short() + "\n");
}
