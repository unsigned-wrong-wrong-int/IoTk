tk := IoTk clone

btn := Object clone do(
   cnt := 1

   onclick := method(
      writeln("clicked! " .. cnt)
      self cnt := cnt + 1
      nil
   )
)

tk define("io_btn", btn)

tk do(
   eval("button .btn -text Click -command {io_btn onclick}")
   eval("pack .btn -side left -padx 10 -pady 10")
   eval("button .close -text Close -command {destroy .}")
   eval("pack .close -side right -padx 10 -pady 10")
)

writeln("start")
tk mainloop
writeln("end")
