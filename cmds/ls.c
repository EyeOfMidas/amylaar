
int
main(string cdm, string args)
{
  if(!args)
    args = "/";
  if(args[strlen(args)-1] != '/')
    args += "/";
  printf("%-79#s\n", implode(get_dir(args), "\n"));
  return 1;
}
