
int
main(string cmd, string arg)
{
  map_array(users(), "who_fun", this_object());
  return 1;
}

void
who_fun(object ob)
{
  write(capitalize(ob->query_real_name()) + "\n");
}
