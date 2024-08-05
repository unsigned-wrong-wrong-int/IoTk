#ifndef IOIOTK_DEFINED
#define IOIOTK_DEFINED 1

#include <tcl8.6/tcl.h>
#include <tcl8.6/tk.h>

#include "IoObject.h"

#define ISIOTK(self) IoObject_hasCloneFunc_(self, (IoTagCloneFunc *)IoIoTk_rawClone)

typedef IoObject IoIoTk;

typedef struct {
   Tcl_Interp *tcl;
   bool isProto;
   List *cmdList;
} IoIoTkData;

IoIoTk *IoIoTk_proto(void *state);
IoIoTk *IoIoTk_rawClone(IoIoTk *proto);
void IoIoTk_free(IoIoTk *self);
void IoIoTk_mark(IoIoTk *self);

IoObject *IoIoTk_init(IoIoTk *self, IoObject *locals, IoMessage *m);
IoObject *IoIoTk_mainloop(IoIoTk *self, IoObject *locals, IoMessage *m);

IoObject *IoIoTk_eval(IoIoTk *self, IoObject *locals, IoMessage *m);

IoObject *IoIoTk_define(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoIoTk_undef(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoIoTk_varAt(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoIoTk_varAtPut(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoIoTk_varRemoveAt(IoObject *self, IoObject *locals, IoMessage *m);

#endif
