#include "IoState.h"
#include "IoSeq.h"
#include "IoNumber.h"

#include "IoIoTk.h"

#define DATA(self) ((IoIoTkData *)IoObject_dataPointer(self))

static const char *protoId = "IoTk";

IoTag *IoIoTk_newTag(void *state) {
   IoTag *tag = IoTag_newWithName_(protoId);
   IoTag_state_(tag, state);

   IoTag_cloneFunc_(tag, (IoTagCloneFunc *)IoIoTk_rawClone);
   IoTag_freeFunc_(tag, (IoTagFreeFunc *)IoIoTk_free);
   IoTag_markFunc_(tag, (IoTagMarkFunc *)IoIoTk_mark);

   return tag;
}

IoIoTk *IoIoTk_proto(void *state) {
   IoObject *self = IoObject_new(state);
   IoObject_tag_(self, IoIoTk_newTag(state));

   Tcl_FindExecutable(protoId);
   IoObject_setDataPointer_(self, calloc(1, sizeof(IoIoTkData)));
   DATA(self)->tcl = Tcl_CreateInterp();
   DATA(self)->isProto = true;
   Tcl_Init(DATA(self)->tcl);
   IoState_registerProtoWithId_((IoState *)state, self, protoId);

   IoMethodTable methodTable[] = {
      {"init", IoIoTk_init},
      {"mainloop", IoIoTk_mainloop},
      {"eval", IoIoTk_eval},
      {NULL, NULL}
   };
   IoObject_addMethodTable_(self, methodTable);

   return self;
}

IoIoTk *IoIoTk_rawClone(IoIoTk *proto) {
   IoIoTk *self = IoObject_rawClonePrimitive(proto);
   IoObject_setDataPointer_(self, calloc(1, sizeof(IoIoTkData)));
   DATA(self)->tcl = DATA(proto)->tcl;
   DATA(self)->isProto = false;
   return self;
}

void IoIoTk_free(IoIoTk *self) {
   if (DATA(self)->isProto) {
      Tcl_DeleteInterp(DATA(self)->tcl);
   }
   free(IoObject_dataPointer(self));
}

void IoIoTk_mark(IoIoTk *self) {

}

IoObject *IoIoTk_init(IoIoTk *self, IoObject *locals, IoMessage *m) {
   int r = Tk_Init(DATA(self)->tcl);
   return IOBOOL(self, r == 0);
}

IoObject *IoIoTk_mainloop(IoIoTk *self, IoObject *locals, IoMessage *m) {
   Tk_MainLoop();
   return IONIL(self);
}

IoObject *IoIoTk_eval(IoIoTk *self, IoObject *locals, IoMessage *m) {
   IoSymbol *cmd = IoMessage_locals_symbolArgAt_(m, locals, 0);
   char *str = CSTRING(cmd);
   size_t len = IoSeq_rawSizeInBytes(cmd);
   int r = Tcl_EvalEx(DATA(self)->tcl, str, len, 0);
   return IONUMBER(r);
}
