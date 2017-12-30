

int
main(string cmd, string arg)
{
  object ob; 

  seteuid(getuid());
  ob = find_object("/log/tmpfile");
  if(ob)
    destruct(ob);
  rm("/log/tmpfile.c");
  write_file("/log/tmpfile.c", "run() { " + arg + "}");
  "/log/tmpfile"->run();
  return 1;
}
