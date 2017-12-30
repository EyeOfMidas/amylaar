/*  obj/master.c     (compat)
**  secure/master.c  (native)
**
** The master is the gateway between the gamedriver and the mudlib to perform
** actions with mudlib specific effects.
** Calls to the master by the gamedriver have an automatic catch() in effect.
**
** This skeleton names all functions called by the gamedriver and gives
** suggestions on how to program then (a few by code).
** Functions which are specific to compat or native mode are tagged as such:
**   '// !compat' means: only when not running in compat mode.
**   '// native'  means: only when running in native mode.
** and similar for every other combination.
**
** Note that the master is loaded first of all objects. Thus you shouldn't
** inherit an other object, nor is the compiler able to search include files
** (read: they must be specified with full path).
**
** Amylaar: actually, you can inherit, but the file will be loaded then before
** the master, which isn't good for most files.
**
**   31-Aug-93  [Mateese]  Written for LPMud 3.2.
**    5-Sep-93  [Amylaar]  Fixed some obvious bugs.
**    6-Sep-93  [Mateese]  Nicened Amylaars additions.
**    9-Sep-93  [Amylaar]  A clarification on compat valid_read()/valid_write()
**   20-Sep-93  [Mateese]  Added quota_demon().
**   18-Nov-93  [Mateese]  Added fourth argument to privilege_violation().
*/


// A short reference to all functions...
//---------------------------------------------------------------------------
//     Initialisation
//
// void create ()  // !compat
//   Initialize the object. Not very useful, though.
//
// void reset (int flag)  // !native
// void reset ()          // native
//   Initialize (compat only) or reset the object.
//
// void inaugurate_master ()
//   Perform mudlib specific setup of the master.
//
// void flag (string arg)
//   Evaluate an argument given as option '-f' to the driver.
//
// string *define_include_dirs ()
//   Define where the include files are searched.
//
// void    epilog ()           // compat
// string *epilog (int eflag)  // !compat
//   Perform final actions before opening the game to players.
//   The semantics of this function differ for compat and !compat mode.
//
// void preload (string file)  // !compat
//   Preload a given object.
//
// void external_master_reload ()
//   Called after a reload of the master on external request.
//
// void reactivate_destructed_master (int removed)
//   Reactivate a formerly destructed master.
//
// string|string * get_simul_efun ()
//   Load the simul_efun object and return one or more paths of it.
//
//---------------------------------------------------------------------------
//     Handling of player connections
//
// object connect ()
//   Handle the request for a new connection.
//
// void disconnect (object obj)
//   Handle the loss of an IP connection.
//
// void remove_player (object player)
//   Remove a player object from the game.
//
//---------------------------------------------------------------------------
//     Runtime Support
//
// object compile_object (string filename)
//   Compile an virtual object.
//
// string get_wiz_name (string file)
//   Return the author of a file.
//
// string object_name (object obj)
//   Return a printable name for an object.
//
// mixed prepare_destruct (object obj)
//   Prepare the destruction of the given object.
//
// void quota_demon (void)
//   Handle quotas in times of memory shortage.
//
// void receive_imp (string host, string msg)
//   Handle a received IMP message.
//
// void slow_shut_down (int minutes)
//   Schedule a shutdown for the near future.
//
//---------------------------------------------------------------------------
//     Error Handling
//
// void dangling_lfun_closure ()
//   Handle a dangling lfun-closure.
//
// void log_error (string file, string err)
//   Announce a compiler-time error.
//
// mixed heart_beat_error (object culprit, string err,
//                         string prg, string curobj, int line)
//   Announce an error in the heart_beat() function.
//
// void runtime_error (string err, string prg, string curobj, int line)
//   Announce a runtime error.
//
//---------------------------------------------------------------------------
//     Security and Permissions
//
// int privilege_violation (string op, mixed who, mixed arg, mixed arg2)
//   Validate the execution of a privileged operation.
//
// int query_allow_shadow (object victim)
//   Validate a shadowing.
//
// int query_player_level (string what)
//   Check if the player is of high enough level for several things.
//
// int valid_exec (string name)
//   Validate the rebinding of an IP connection by usage of efun exec().
//
// int valid_query_snoop (object obj)
//   Validate if the snoopers of an object may be revealed by usage of the
//   efun query_snoop().
//
// int valid_snoop (object snoopee, object snooper)
//   Validate the start/stop of a snoop.
//
//---------------------------------------------------------------------------
//     Userids and depending Security
//
// string creator_file (mixed obj)
//   Return the name of the creator of an object.
//   This is called in every mode!
//
// string get_root_uid ()  // !compat
//   Return the string to be used as root-uid.
//
// string get_bb_uid()  // !compat
//   Return the string to be used as root-uid.
//
// int valid_seteuid (object obj, string neweuid)  // !compat
//   Validate the change of an objects euid by efun seteuid().
//
// int|string valid_read  (string path, string euid, string fun, object caller)
// int|string valid_write (string path, string euid, string fun, object caller)
//   Validate a reading/writing file operation.
//
//---------------------------------------------------------------------------
//     ed() Support
//
// string make_path_absolute (string str)
//   Absolutize a relative filename given to the editor.
//
// int save_ed_setup (object who, int code)
//   Save individual settings of ed for a wizard.
//
// int retrieve_ed_setup (object who)
//   Retrieve individual settings of ed for a wizard.
//
// string get_ed_buffer_save_file_name (string file)
//   Return a filename for the ed buffer to be save into.
//
//---------------------------------------------------------------------------
//     parse_command() Support  (!compat)
//
// string *parse_command_id_list ()
//   Return generic singular ids.
//
// string *parse_command_plural_id_list ()
//   Return generic plural ids.
//
// string *parse_command_adjectiv_id_list ()
//   Return generic adjective ids.
//
// string *parse_command_prepos_list ()
//   Return common prepositions.
//
// string parse_command_all_word()
//   Return the one(!) 'all' word.
//
//---------------------------------------------------------------------------


//===========================================================================
//  Initialisation
//
// These functions are called after (re)loading the master to establish the
// most basic operation parameters.
//
// The initialisation of LPMud on startup follows this schedule:
//   - The gamedriver evaluates the commandline options and initializes
//     itself.
//   - The master is loaded, and thus it's create() (!compat only) and
//     its reset() is called.
//   - get_root_uid() is called. If the result is valid, it becomes the
//     masters uid and euid.
//   - get_bb_uid() is called.
//   - inaugurate_master() is called.
//   - flag() is called for each given '-f' commandline option.
//   - get_simul_efun() is called.
//   - define_include_dirs() is called.
//   - Preloading is done:
//       compat : The filenames of the objects are read from INIT_FILE
//                and the objects are loaded. Then epilog() is called.
//       !compat: epilog() is called. If it returns an array of strings,
//                it is considered holding the filenames of the objects
//                to preload. They are then given one at a time as
//                argument to preload() which does the actual preloading.
//   - The gamedriver sets up the IP communication and enters the backend
//     loop.
//
// If the master is reloaded during the game, this actions are taken:
//   - The master is loaded, and thus it's create() (!compat only) and
//     its reset() is called.
//   - Any auto-include string is cleared.
//   - get_root_uid() is called. If the result is valid, it becomes the
//     masters uid and euid.
//   - inaugurate_master() is called.
//
// If the master was destructed, but couldn't be reloaded, the old
// master object could be reactivated. In that case:
//   - reactivate_destructed_master() is called.
//   - inaugurate_master() is called.
//===========================================================================

//---------------------------------------------------------------------------
void create ()  // !compat

// Initialize the master.
//
// This is the first function called, and since the master is not recognized
// as such at this time, a number of (important) things will not work here.
// Better omit this function and wait until inaugurate_master() is called,
// then the master will have it's full power.


//---------------------------------------------------------------------------
void reset (int flag)  // !native
void reset ()          // native

// Initialize (compat only) or reset the master.
//
// Argument (!native only):
//   flag: 0 if the object was just loaded (same as create() under !compat)
//         1 if the object is just reset.
//
// For the initial call after load (!native: flag = 0) the same considerations
// as to create() apply.


//---------------------------------------------------------------------------
void inaugurate_master ()

// Perform mudlib specific setup of the master.
//
// This function is called whenever the master becomes fully operational
// after (re)loading (it is now recognized as _the_ master).
// This doesn't imply that the game is up and running.
//
// Do whatever you feel you need to do, e.g. set_auto_include_string(),
// or give the master a decent euid.


//---------------------------------------------------------------------------
void flag (string arg)

// Evaluate an argument given as option '-f' to the driver.
//
// Arguments:
//   arg: The argument string from the option text '-f<arg>'.
//        If several '-f' options are given, this function
//        will be called sequentially with all given arguments.
//
// This function can be used to pass the master commands via arguments to
// the driver. This is useful when building a new mudlib from scratch.
// It is called only when the game is started.
//
// The code given implements these commands:
//   '-fcall <ob> <fun> <arg>': call function <fun> in object <ob> with
//                              argument <arg>.
//   '-fshutdown': shutdown the game immediately.
// Thus, starting the game as 'parse "-fcall foo bar Yow!" -fshutdown' would
// first do foo->bar("Yow!") and then shutdown the game.

{
  string obj, fun, rest;

  if (arg == "shutdown")
  {
    shutdown();
    return;
  }
  if (sscanf(arg, "call %s %s %s", obj, fun, rest) >= 2)
  {
    write(obj+"->"+fun+"(\""+rest+"\") = ");
    write(call_other(obj, fun, rest));
    write("\n");
    return;
  }
  write("master: Unknown flag "+arg+"\n");
}


//---------------------------------------------------------------------------
string *define_include_dirs ()

// Define where the include files are searched.
//
// Result:
//   An array of string patterns giving the absolut paths where to search an
//   include file. The patterns have to have a %s at the place where the
//   name given in the include statement has to be inserted.
//
// The example code defines the directories /sys/ and /lib/ as default dirs.

{
  return ({ "sys/%s", "lib/%s" });
}


//---------------------------------------------------------------------------
void    epilog ()           // compat
string *epilog (int eflag)  // !compat

// Perform final actions before opening the game to players.
// The semantics of this function differ for compat and !compat mode.
//
// compat:
//   Under compat, the objects from the INIT_FILE are already loaded at this
//   time. Normally there is nothing left to do for this function.
//
// native:
//   Arguments:
//     eflag: This is the number of '-e' options given to the parser.
//            Normally it is just 0 or 1.
//
//   Result:
//     An array of strings, which designate the objects to be
//     preloaded with preload().
//     Any other result is interpreted as 'no object to preload'.
//     The resulting strings will be passed one at the time as
//     arguments to preload().


//---------------------------------------------------------------------------
void preload (string file)  // !compat

// Preload a given object.
//
// Arguments:
//   file: The filename of the object to preload, as returned by epilog().
//
// It is task of the epilog()/preload() pair to ensure the validity of
// the given strings (e.g. filtering out comments and blank lines).
// For preload itself a 'call_other(file, "???")' is sufficient, but it
// should be guarded by a catch() to avoid premature blockings.
// Also it is wise to change the master's euid from root_uid to something
// less privileged for the time of the preload.


//---------------------------------------------------------------------------
void external_master_reload ()

// Master was reloaded on external request by SIGUSR1.
//
// If the gamedriver destruct and reloads the master on external request
// via SIGUSR1, it does this by a call to this function.
// It will be called after inaugurate_master() of course.
// If you plan to do additional magic here, you're welcome.


//---------------------------------------------------------------------------
void reactivate_destructed_master (int removed)

// Reactivate a formerly destructed master.
//
// Arguments:
//   removed: True if the master was already on the list of destructed
//            objects.
//
// This function is called in an formerly destructed master since a new master
// couldn't be loaded.
// This function has to reinitialize all variables at least to continue
// operation.


//---------------------------------------------------------------------------
mixed get_simul_efun ()

// Load the simul_efun object and return one or more paths of it.
//
// Result:
//   Either a single string with the file_name() of the simul_efun object,
//   or an array of strings which has to start with that file_name().
//   Return 0 if this feature isn't wanted.
//
// Note that the object must be loaded by this function!
//
// When you return an array of strings, the first string is taken as path
// to the simul_efun object, and all other paths are used for backup
// simul_efun objects to call simul_efuns that are not present in the
// main simul_efun object. This allows to remove simul_efuns without getting
// errors from old compiled programs that still use the obsolete simul_efuns.
//
// It is useful to have one 'spare' simul_efun object in reserve in case the
// normal one fails to load.
// If the game depends on the simul_efun object, and none could be loaded,
// an immediate shutdown should occur.


//===========================================================================
//  Handling of player connections
//
// See also valid_exec().
//===========================================================================

//---------------------------------------------------------------------------
object connect ()

// Handle the request for a new connection.
//
// Result:
//   An login object the requested connection should be bound to.
//
// Note that the connection is not bound yet!
//
// The gamedriver will call the lfun 'logon()' in the login object after
// binding the connection to it. That lfun has to return !=0 to succeed.


//---------------------------------------------------------------------------
void disconnect (object obj)

// Handle the loss of an IP connection.
//
// Argument:
//   obj: The (formerly) interactive object (player).
//
// This called by the gamedriver to handle the removal of an IP connection,
// either because the connection is already lost ('netdeath') or due to
// calls to exec() or remove_interactive().
// The connection will be unbound upon return from this call.


//---------------------------------------------------------------------------
void remove_player (object player)

// Remove a player object from the game.
//
// Argument:
//   player: The player object to be removed.
//
// This function is called by the gamedriver to expell remaining players
// from the game on shutdown in a polite way.
// If this functions fails to quit/destruct the player, it will be
// destructed the hard way by the gamedriver.
//
// Note: This function must not cause runtime errors.


//===========================================================================
//  Runtime Support
//
// Various functions used to implement advanced runtime features.
//===========================================================================

//---------------------------------------------------------------------------
object compile_object (string filename)

// Compile an virtual object.
//
// Arguments:
//   previous_object(): The object requesting the virtual object.
//   filename         : The requested filename for the virtual object.
//
// Result:
//   The object to serve as the requested virtual object, or 0.
//
// This function is called if the compiler can't find the filename for an
// object to compile. The master has now the opportunity to return an other
// which will then serve as if it was compiled from <filename>.
// If the master returns 0, the usual 'Could not load'-error will occur.


//---------------------------------------------------------------------------
string get_wiz_name (string file)

// Return the author of a file.
//
// Arguments:
//   file: The name of the file in question.
//
// Result:
//   The name of the file's author (or 0 if there is none).
//
// This function is called for maintenance of the wiz-list, to assign scores
// to the right wizard.


//---------------------------------------------------------------------------
string object_name (object obj)

// Return a printable name for an object.
//
// Arguments:
//   obj: The object which name is of interest.
//
// Result:
//   A string with the object name, or 0.
//
// This function is called by sprintf() to print a meaningful name
// in addition to the normal file_name().


//---------------------------------------------------------------------------
mixed prepare_destruct (object obj)

// Prepare the destruction of the given object.
//
// Argument:
//   obj : The object to destruct.
//
// Result:
//   Return 0 if the object is ready for destruction, any other value
//   will abort the attempt.
//   If a string is returned, an error with the string as message will
//   be issued.
//
// The gamedriver calls this function whenever an object shall be destructed.
// It expects, that this function cleans the inventory of the object, or
// the destruct will fail.
// Furthermore, the function could notify the former inventory objects that
// their holder is under destruction (useful to move players out of rooms which
// are updated); and it could announce mudwide the destruction(quitting) of
// players.


//---------------------------------------------------------------------------
void quota_demon (void)

// Handle quotas in times of memory shortage.
//
// This function is called during the final phase of a garbage collection if
// the reserved user area couldn't be reallocated. This function (or a called
// demon) has now the opportunity to remove some (still active) objects from
// the game. If this does not free enough memory to reallocate the user
// reserve, slow_shut_down() will be called to start Armageddon.
//
// Note: Up to now, the wizlist lacks various informations needed to detect
//   the memory-hungriest wizards.


//---------------------------------------------------------------------------
void receive_imp (string host, string msg)

// Handle a received IMP message.
//
// Arguments:
//   host: Name of the host the message comes from.
//   msg : The received message.
//
// This function is called for every message received on the IMP port.


//---------------------------------------------------------------------------
void slow_shut_down (int minutes)

// Schedule a shutdown for the near future.
//
// Argument:
//   minutes: The desired time in minutes till the shutdown:
//             six, if just the user reserve has been put intp use;
//             one, if the (smaller) master reserve has been put into use
//                  as well.
//
// The gamedriver calls this function when it runs low on memory.
// At this time, it has freed its reserve, but since it won't last long,
// the games needs to be shut down.
// The delay is to give the players the opportunity to finish quests,
// sell their stuff, etc.
//
// For eaxmple: this function might load an 'Armageddon' object and tells
// it what to do. It is the Armageddon then which performs the shutdown.
//
// Technical:
//   The memory handling of the gamedriver includes three reserved areas:
//   user, system and master. All three are there to insure that the game
//   shuts down gracefully when the memory runs out: the user area to give
//   the players time to quit normally, the others to enable emergency-logouts
//   when the user reserve is used up as well.
//   The areas are allocated at start of the gamedriver, and released when
//   no more memory could be obtained from the host. In such a case, one
//   of the remaining areas is freed (so the game can continue a short
//   while) and a garbagecollection is initiated.
//   If the garbagecollection recycles enough memory (either true garbage
//   or by the aid of the quota_demon) to reallocate the areas, all is
//   fine, else the game is shut down by a call to this function.


//===========================================================================
//  Error Handling
//
//===========================================================================

//---------------------------------------------------------------------------
void dangling_lfun_closure ()

// Handle a dangling lfun-closure.
//
// This is called when the gamedriver detects a closure using a vanished lfun.
// A proper handling is to raise a runtime error.

{
  raise_error("dangling lfun closure\n");
}

//---------------------------------------------------------------------------
void log_error (string file, string err)

// Announce a compiler-time error.
//
// Arguments:
//   file: The name of file containing the error (it needn't be an object
//         file!).
//   err : The error message.
//
// Whenever the LPC compiler detects an error, this function is called.
// It should at least log the error in a file, and also announce it
// to the active player if it is an wizard.


//---------------------------------------------------------------------------
mixed heart_beat_error (object culprit, string err,
                        string prg, string curobj, int line)

// Announce an error in the heart_beat() function.
//
// Arguments:
//   culprit: The object which lost the heart_beat.
//   err    : The error message.
//   prg    : The executed program (might be 0).
//   curobj : The object causing the error (might be 0).
//   line   : The line number where the error occured (might be 0).
//
// Result:
//   Return anything != 0 to restart the heart_beat in culprit.
//
// This function has to announce an error in the heart_beat() function
// of culprit.
// At time of call, the heart_beat has been turned off.
// A player should at least get a "You have no heartbeat!" message, a more
// advanced handling would destruct the offending object and allow the
// heartbeat to restart.
//
// Note that <prg> denotes the program actually executed (which might be
// inherited one) whereas <curobj> is just the offending object.


//---------------------------------------------------------------------------
void runtime_error (string err, string prg, string curobj, int line)

// Announce a runtime error.
//
// Arguments:
//   err    : The error message.
//   prg    : The executed program.
//   curobj : The object causing the error.
//   line   : The line number where the error occured.
//
// This function has to announce a runtime error to the active player.
// If it is a wizard, it might give him the full error message together
// with the source line; if it is a player, it should issue a decent
// message ("Your sensitive mind notices a wrongness in the fabric of space")
// and could also announce the error to the wizards online.
//
// Note that <prg> denotes the program actually executed (which might be
// inherited one) whereas <curobj> is just the offending object.


//===========================================================================
//  Security and Permissions
//
// Most of these functions guard critical efuns. A good approach to deal 
// with them is to redefine the efuns by simul_efuns (which can then avoid
// trouble prematurely) and give root objects only the permission to
// execute the real efuns.
//
// See also creator_file(), valid_read() and valid_write().
//===========================================================================

//---------------------------------------------------------------------------
int privilege_violation (string op, mixed who, mixed arg, mixed arg2)

// Validate the execution of a privileged operation.
//
// Arguments:
//   op   : the requestion operation (see below)
//   who  : the object requesting the operation (filename or object pointer)
//   arg  : additional argument, depending on <op>.
//   arg2 : additional argument, depending on <op>.
//
// Result:
//     >0: The caller is allowed for this operation.
//      0: The caller was probably misleaded; try to fix the error
//   else: A real privilege violation; handle it as error.
//
// Privileged operations are:
//   bind_lambda       : Bind a lambda-closure to object <arg>.
//   call_out_info     : Return an array with all call_out informations.
//   nomask simul_efun : Attempt to get an efun via efun:: when it is shadowed
//                       by a 'nomask'-type simul_efun.
//   rename_object     : The current object <who> renames object <arg>
//                       to name <arg2>.
//   send_imp          : Send UDP-data to host <arg>.
//   set_auto_include_string : Set the string automatically included by
//                       the compiler.
//   get_extra_wizinfo : Get the additional wiz-list info for wizard <arg>.
//   set_extra_wizinfo : Set the additional wiz-list info for wizard <arg>.
//   set_extra_wizinfo_size : Set the size of the additional wizard info
//                       in the wiz-list to <arg>.
//   set_this_object   : Set this_object() to <arg>.
//   shadow_add_action : Add an action to function <arg> from a shadow.
//   wizlist_info      : Return an array with all wiz-list information.
//
// call_out_info can return the arguments to functions and lambda closures
// to be called by call_out(); you should consider that read access to
// closures, mappings and pointers means write access and/or other privileges.
// wizlist_info() will return an array which holds, among others, the extra
// wizlist field. While a toplevel array, if found, will be copied, this does
// not apply to nested arrays or to any mappings. You might also have some
// sensitive closures there.
// send_imp() should be watched as it could be abused to mess up the IMP.
// The xxx_extra_wizinfo operations are necessary for a proper wizlist and
// should therefore be restricted to admins.
// All other operations are potential sources for direct security breaches -
// any use of them should be scrutinized closely.

//---------------------------------------------------------------------------
int query_allow_shadow (object victim)

// Validate a shadowing.
//
// Arguments:
//   previous_object(): the wannabe shadow
//   victim: the object to be shadowed.
//
// Result:
//   Return 0 to disallow the shadowing, any other value to allow it.
//   Destructing the shadow or the victim is another way of disallowing.
//
// The function should deny shadowing on all root objects, else it might
// query the victim for clearance.


//---------------------------------------------------------------------------
int query_player_level (string what)

// Check if the player is of high enough level for several things.
//
// Argument:
//   what: The 'thing' type (see below).
//
// Result:
//   Return 0 to disallow, any other value to allow it.
//
// Types asked for so far are:
//   "trace"         : Is the player is allowed to use tracing?
//   "showsmallnewmalloced": Is the player allowed to issue this special
//                     command?
//   "wizard"        : Is the player considered at least a "minimal" wizard ?


//---------------------------------------------------------------------------
int valid_exec (string name)

// Validate the rebinding of an IP connection by usage of efun exec().
//
// Arguments:
//    name: The name of the _program_ attempting to rebind the connection.
//          This is not the file_name() of the object, and has no leading
//          slash.
//
// Result:
//   Return 0 to disallow the action, any other value to allow it.


//---------------------------------------------------------------------------
int valid_query_snoop (object obj)

// Validate if the snoopers of an object may be revealed by usage of the
// efun query_snoop().
//
// Arguments:
//   previous_object(): the asking object.
//   obj              : the object which snoopers are to be revealed.
//
// Result:
//   Return 0 to disallow the action, any other value to allow it.


//---------------------------------------------------------------------------
int valid_snoop (object snoopee, object snooper)

// Validate the start/stop of a snoop.
//
// Arguments:
//   snoopee: The victim of the snoop.
//   snooper: The wannabe snooper, or 0 when stopping a snoop.
//
// Result:
//   Return 0 to disallow the action, any other value to allow it.


//===========================================================================
//  Userids and depending Security
//
// Userids are a new feature of !compat drivers to allow the implementation
// of more secure mudlibs (though some experts think that this attempt has
// failed (Heya Macbeth!)).
//
// Userids are unique strings (or 0 for 'no uid').
// The two most important uids are the root-uid (this is the one the master
// gets automatically), and the backbone-uid.
// Objects whose cuid (uid returned from creator_file()) is the backbone-uid 
// don't get that cuid as own uid, but instead the uid of the cloning
// or loading object.
//
// The main use for uids are for determination of file access rights, so
// valid_read() and valid_write() are under !compat no longer in the player
// object, but instead in the master.
// You can of course use the uids for other identification purposes as well.
//
// Note that creator_file(), valid_read() and valid_write() are in use
// even when not using uids themselves!
//===========================================================================

//---------------------------------------------------------------------------
string creator_file (mixed obj)

// Return the name of the creator of an object.
//
// Arguments:
//   obj: The object, given by pointer or by file_name().
//
// Result:
//   A string holding the name of the creator, or 0.
//
// This function is called for newly created objects.
// If the function returns 0, the object can't be loaded and is destructed
// again.
// In !compat mode, the returned string serves as the initial uid ('cuid')
// of the object.
// Under !compat, the returned name serves as uid for the obj (with the
// exception of the backbone uid).
//
// Note that this function is called in compat mode as well!


//---------------------------------------------------------------------------
string get_root_uid ()  // !compat

// Return the string to be used as root-uid.
// Under !native, the function is expendable.


//---------------------------------------------------------------------------
string get_bb_uid()  // !compat

// Return the string to be used as root-uid.
// Under !native, the function is expendable.


//---------------------------------------------------------------------------
int valid_seteuid (object obj, string neweuid)  // !compat

// Validate the change of an objects euid by efun seteuid().
//
// Arguments:
//   obj    : The object requesting the new euid.
//   neweuid: The new euid requested.
//
// Result:
//   Return 0 to disallow the change, any other value to allow it.


//---------------------------------------------------------------------------
mixed valid_read  (string path, string euid, string fun, object caller)
mixed valid_write (string path, string euid, string fun, object caller)

// Validate a reading/writing file operation.
//
// Arguments:
//   path   : The (possibly partial) filename given to the operation.
//   euid   : The euid of the caller (might be 0).
//   fun    : The name of the operation requested (see below).
//   caller : The calling object.
//
// Result:
//   The full pathname of the file to operate on, or 0 if the action is not
//   allowed.
//   You can also return 1 to indicate that the path can be used unchanged.
//
// These are the central functions establishing the various file access
// rights.
//
// Note that this function is called in compat mode as well!
// If you need to be compatible with the old 2.4.5-mudlib, redirect these
// calls to the valid_read/valid_write in the player object.
//
// valid_read() is called for these operations:
//   ed_start        (when reading a file)
//   file_size
//   get_dir
//   print_file
//   read_bytes
//   read_file
//   restore_object
//   tail
//
// valid_write() is called for these operations:
//   cindent
//   ed_start     (when writing a file)
//   do_rename    (twice for each the old and new name)
//   mkdir
//   save_object
//   remove_file
//   rmdir
//   write_bytes
//   write_file


//===========================================================================
//  ed() Support
//
//===========================================================================

//---------------------------------------------------------------------------
string make_path_absolute (string str)

// Absolutize a relative filename given to the editor.
//
// Argument:
//   str : The relative filename (without leading slash).
//
// Result:
//   The full pathname of the file to use.
//   Any non-string result will act as 'bad file name'.


//---------------------------------------------------------------------------
int save_ed_setup (object who, int code)

// Save individual settings of ed for a wizard.
//
// Arguments:
//   who : The wizard using the editor.
//   code: The encoded options to be saved.
//
// Result:
//   Return 0 on failure, any other value for success.
//
// This function has to save the given integer into a safe place in the
// realm of the given wizard, either a file, or in the wizard itself.
//
// Be aware of possible security breaches: under !compat, a write_file()
// should be surrounded by a temporary setting of the masters euid to
// that of the wizard.


//---------------------------------------------------------------------------
int retrieve_ed_setup (object who)

// Retrieve individual settings of ed for a wizard.
//
// Arguments:
//   who : The wizard using the editor.
//
// Result:
//   The encoded options retrieved (0 if there are none).


//---------------------------------------------------------------------------
string get_ed_buffer_save_file_name (string file)

// Return a filename for the ed buffer to be saved into.
//
// Arguments:
//   this_player(): The wizard using the editor.
//   file         : The name of the file currently in the buffer.
//
// Result:
//   The name of the file to save the buffer into, or 0.
//
// This function is called whenever a wizard is destructed/goes netdeath
// while editing. Using this function, his editing is not done in vain.


//===========================================================================
//  parse_command() Support  (!compat)
//
// LPMud has a builtin support for parsing complex commands.
// It does this by requestion several types of ids from the objects.
// The same queried functions are also in the master to provide decent
// defaults, especially for generic ids like 'all the blue ones'.
//
// Each of the functions has to return an array of strings (with the exception
// of parse_command_all_word), each string being one of the ids for that type
// of id.
//
// The whole parsing has a preferrence for the english language, so the
// the code for parsing english constructs is given as well.
//===========================================================================

//---------------------------------------------------------------------------
string *parse_command_id_list ()

// Return generic singular ids.

{
  return ({ "one", "thing" });
}


//---------------------------------------------------------------------------
string *parse_command_plural_id_list ()

// Return generic plural ids.

{
  return ({ "ones", "things", "them" });
}


//---------------------------------------------------------------------------
string *parse_command_adjectiv_id_list ()

// Return generic adjective ids.
// If there are none (like here), return some junk which is likely never
// typed.

{
  return ({ "iffish" });
}


//---------------------------------------------------------------------------
string *parse_command_prepos_list ()

// Return common prepositions.

{
    return ({ "in", "on", "under", "behind", "beside" });
}


//---------------------------------------------------------------------------
string parse_command_all_word()

// Return the one(!) 'all' word.

{
  return "all";
}


/****************************************************************************/
