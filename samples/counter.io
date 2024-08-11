tk := IoTk clone

btn := Object clone do(
   cnt := 0

   onclick := method(
      self cnt := cnt + 1
      writeln("clicked! " .. cnt)
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

writeln("start " .. btn cnt)
tk mainloop
writeln("end " .. btn cnt)
