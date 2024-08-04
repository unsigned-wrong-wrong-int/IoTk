#include "IoState.h"

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
   Tk_Init(DATA(self)->tcl);
   IoState_registerProtoWithId_((IoState *)state, self, protoId);

   IoMethodTable methodTable[] = {
      {"test", IoIoTk_test},
      {NULL, NULL}
   };
   IoObject_addMethodTable_(self, methodTable);

   return self;
}

IoIoTk *IoIoTk_rawClone(IoIoTk *proto) {
   IoIoTk *self = IoObject_rawClonePrimitive(proto);
   DATA(self)->tcl = DATA(proto)->tcl;
   DATA(self)->isProto = false;
   Tk_Init(DATA(self)->tcl);
   return self;
}

void IoIoTk_free(IoIoTk *self) {
   if (DATA(self)->isProto) {
      Tcl_Finalize();
   }
   free(IoObject_dataPointer(self));
}

void IoIoTk_mark(IoIoTk *self) {

}

IoObject *IoIoTk_test(IoIoTk *self, IoObject *locals, IoMessage *m) {
   Tk_Window w = Tk_CreateWindow(DATA(self)->tcl, NULL, "test", "");
   Tk_ResizeWindow(w, 600, 400);
   Tk_Display(w);
   Tk_MainLoop();
   return IONIL(self);
}
