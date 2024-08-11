tk := IoTk clone

tk do(
   eval("label .lbl -text \"Hello, world!\"")
   eval("pack .lbl -padx 80 -pady 20")
) mainloop
