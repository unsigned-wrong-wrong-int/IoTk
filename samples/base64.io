tk := IoTk clone

base64 := Object clone do(
   encode := method(src, dest,
      len := tk eval("llength [set io_base64.l [#{src} dump -text 1.0 end]]" interpolate) asNumber
      s := Sequence clone
      for(i, 1, len - 2, 3,
         s appendSeq(tk eval("lindex ${io_base64.l} " .. i))
      )
      writeln(s encoding)
      writeln(s)
      t := s asBase64
      tk eval("#{dest} delete 1.0 end; #{dest} insert end \"#{t}\"" interpolate)
      tk eval("unset io_base64.l")
   )
)

tk define("io_base64", base64)

tk do(
   eval("label .l1 -text \"Plain Text\"")
   eval("label .l2 -text \"Base64 Encoded\"")

   eval("button .enc -text \">>\" -command {io_base64 encode .plain .base64}")

   eval("text .plain -wrap word")
   eval("text .base64 -wrap char")

   eval("grid .l1 x .l2 -sticky nw")
   eval("grid .plain x .base64 -rowspan 3 -sticky nsew")
   eval("grid .enc -row 2 -column 1")
)

tk mainloop
