int
main(string cmd, string arg)
{
  object *objs;

  objs = all_inventory(environment(this_player()));
  map_array(objs, "say_fun", this_object(), arg);

  return 1;
}

int
say_fun(object obj, string mess)
{
  if(obj == this_player())
  {
    obj->catch_message("say", "You say: " + mess + "\n");
  } 
  else if(interactive(obj))
  {
    obj->catch_message("say", capitalize(this_player()->query_real_name()) +
		       " says: " + mess + "\n");
  }
  return 0;
}
    
