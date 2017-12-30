
void
create()
{
  seteuid(getuid());
}

int
move(mixed dest)
{
  move_object(this_object(), dest);
  return 1;
}

