
int
main(string com, string args)
{
  object ob;

  ob = find_object(args);
  if(!ob)
  {
    write("No such object\n");
    return 1;
  }
  destruct(ob);
  write("Ok\n");
  return 1;
}

