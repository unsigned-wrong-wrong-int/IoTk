/*
   IoTk prototype and clone
*/
IoTk println
# ==>  IoTk_0x************:
#  define           = IoTk_define()
#  eval             = IoTk_eval()
#  mainloop         = IoTk_mainloop()
#  undef            = IoTk_undef()
#

tk := IoTk clone
tk println
# ==>  IoTk_0x************:
#

/*
   eval simple commands
*/
tk eval("puts \"Hello, world!\"")
# ==> Hello, world!

tk eval("puts [expr 1 + 2]")
# ==> 3

x := tk eval("expr 1 + 2 * 3")
x println
# ==> 7
x type println
# ==> Sequence

/*
   eval with variables
*/
tk eval("set a test")
x := tk eval("set a")
x println
# ==> test

tk eval("set b [list 1 2 3 $a]")
tk eval("puts $b")
# ==> 1 2 3 test

/*
   eval error
*/
try(
   tk eval("set")
) error println
# ==> Tcl/Tk error: wrong # args: should be "set varName ?newValue?"

/*
   Io object as command
*/
obj := Object clone do(
   foo := method("foo" println; 42)

   bar := method(x, y, x asNumber + y asNumber)

   baz := method(Exception raise("error from baz"))
)
tk define("io_obj", obj)

x := tk eval("io_obj foo")
# ==> foo
x println
# ==> 42

tk eval("set a 5")
tk eval("set b [io_obj bar 1 [expr $a * 10]]")
tk eval("puts $b")
# ==> 51

try(
   tk eval("io_obj baz")
) error println
# ==> Tcl/Tk error: error in Io method: error from baz

try(
   tk eval("io_obj")
) error println
# ==> Tcl/Tk error: wrong # args: should be "io_obj methodName ?arg ...?"

/*
   Io command undef
*/
tk undef("io_obj")
try(
   tk eval("io_obj foo")
) error println
# ==> Tcl/Tk error: invalid command name "io_obj"
