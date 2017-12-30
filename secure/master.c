

#include "/include/config.h"
#include "simul_efun.c"

object connect() {
  return clone_object(LOGIN_OBJ);
}


void runtime_error (string err, string prg, string curobj, int line)
{
  string mess;

  mess = curobj + ":" + prg + ":" + line + "\n" + err;
  write(mess);
  log_file("runtime.err", mess);
}

void log_error (string file, string err)
{
  string mess;

  mess = file + "\n" + err;
  write(mess);
  log_file("compile.err", mess);
}

mixed heart_beat_error (object culprit, string err,
                        string prg, string curobj, int line)
{
  log_file("heart_beat", file_name(culprit) + "\n" + err  + "\n" + prg  + 
	   "\n" + curobj + "\n" +  line  + "\n");
  return 0;
}
  
void crash(string error)
{
  log_file("crashes", "CRASHED on: " + ctime(time()) +
	   " ERROR: "+error+"\n");
}


int valid_seteuid(object ob, string str) { return 1; }

nomask int valid_shadow(object ob) { return 1; }

string get_root_uid() { return ROOT_EUID; }

string get_bb_uid() { return "Backbone"; }

string *define_include_dirs() { return ({ "/include/%s" }); }

string get_simul_efun() 
{
  "/secure/simul_efun"->gurksallad();
  return "/secure/simul_efun";
}

int valid_socket(object calling_ob, string func, mixed *info) { return 1;  }

int valid_override(string file, string name) { return 1; }
int valid_read() { return 1; }
int valid_write() { return 1; }

int valid_exec (string name) { return 1; }

int
valid_hide(object who)
{ return 0; }

logon()
{
  cat(WELCOME);
}


string process_input(string str) 
{
  object ob;
  mixed ret;

  seteuid("Root");
  rm("/etc/fil.c");
  if(ob = find_object("/etc/fil.c"))
    destruct(ob);
  write_file("/etc/fil.c", "run() {  seteuid(\"Root\"); return " + str + ";}");
  ret = "/etc/fil"->run();
  printf("Ret: %O\n", ret);
  rm("/etc/fil.c");
  return "";
}
  
string domain_file() 
{
    return "foo";
}

string creator_file() 
{
    return "foo";
}

string author_file() 
{
    return "foo";
}

mixed 
prepare_destruct (object obj) { return 0; }

