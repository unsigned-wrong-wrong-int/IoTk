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
      {"define", IoIoTk_define},
      {"undef", IoIoTk_undef},
      {"varAt", IoIoTk_varAt},
      {"varAtPut", IoIoTk_varAtPut},
      {"varRemoveAt", IoIoTk_varRemoveAt},
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
   if (r != TCL_OK) {
      return IONIL(self);
   }
   const char *result = Tcl_GetStringResult(DATA(self)->tcl);
   return IoSeq_newWithCString_(IOSTATE, result);
}

typedef struct {
   IoObject *self;
   IoSymbol *slotName;
} TkCmdData;

int TkCmdProc(ClientData data, Tcl_Interp *interp, int argc, const char *argv[]) {
   IoObject *self = ((TkCmdData *)data)->self;
   IoMessage *m = IoMessage_newWithName_(IOSTATE, ((TkCmdData *)data)->slotName);
   for (int i = 1; i < argc; ++i) {
      IoMessage_addCachedArg_(m, IoSeq_newWithCString_(IOSTATE, argv[i]));
   }
   IoObject *result = IoMessage_locals_performOn_(m, self, self);
   char *str = NULL;
   if (result != IONIL(self)) {
      IoObject *ioStr = IoMessage_locals_performOn_(IOSTATE->asStringMessage, result, result);
      if (ISSEQ(ioStr)) {
         str = CSTRING(ioStr);
      }
   }
   Tcl_FreeResult(interp);
   Tcl_SetResult(interp, str, TCL_VOLATILE);
   return TCL_OK;
}

void TkCmdDeleteProc(ClientData data) {
   free(data);
}

IoObject *IoIoTk_define(IoObject *self, IoObject *locals, IoMessage *m) {
   char *name = IoMessage_locals_cStringArgAt_(m, locals, 0);
   TkCmdData *data = (TkCmdData *)calloc(1, sizeof(TkCmdData));
   data->self = IoMessage_locals_valueArgAt_(m, locals, 1);
   data->slotName = IoMessage_locals_symbolArgAt_(m, locals, 2);
   Tcl_CreateCommand(DATA(self)->tcl, name, TkCmdProc, data, TkCmdDeleteProc);
   return IONIL(self);
}

IoObject *IoIoTk_undef(IoObject *self, IoObject *locals, IoMessage *m) {
   char *name = IoMessage_locals_cStringArgAt_(m, locals, 0);
   Tcl_DeleteCommand(DATA(self)->tcl, name);
   return IONIL(self);
}

IoObject *IoIoTk_varAt(IoObject *self, IoObject *locals, IoMessage *m) {
   char *name1 = IoMessage_locals_cStringArgAt_(m, locals, 0);
   char *name2 = NULL;
   if (IoMessage_argCount(m) >= 2) {
      name2 = IoMessage_locals_cStringArgAt_(m, locals, 1);
   }
   const char *value = Tcl_GetVar2(DATA(self)->tcl, name1, name2, 0);
   if (value == NULL) {
      return IONIL(self);
   }
   return IoSeq_newWithCString_(IOSTATE, value);
}

IoObject *IoIoTk_varAtPut(IoObject *self, IoObject *locals, IoMessage *m) {
   char *name1 = IoMessage_locals_cStringArgAt_(m, locals, 0);
   char *name2 = NULL;
   IoObject *value;
   if (IoMessage_argCount(m) >= 3) {
      name2 = IoMessage_locals_cStringArgAt_(m, locals, 1);
      value = IoMessage_locals_valueArgAt_(m, locals, 2);
   } else {
      value = IoMessage_locals_valueArgAt_(m, locals, 1);
   }
   const char *str = "";
   if (value != IONIL(self)) {
      IoObject *ioStr = IoMessage_locals_performOn_(IOSTATE->asStringMessage, value, value);
      if (ISSEQ(ioStr)) {
         str = CSTRING(ioStr);
      }
   }
   const char *result = Tcl_SetVar2(DATA(self)->tcl, name1, name2, str, 0);
   return IOBOOL(self, result != NULL);
}

IoObject *IoIoTk_varRemoveAt(IoObject *self, IoObject *locals, IoMessage *m) {
   char *name1 = IoMessage_locals_cStringArgAt_(m, locals, 0);
   char *name2 = NULL;
   if (IoMessage_argCount(m) >= 2) {
      name2 = IoMessage_locals_cStringArgAt_(m, locals, 1);
   }
   int r = Tcl_UnsetVar2(DATA(self)->tcl, name1, name2, 0);
   return IOBOOL(self, r == TCL_OK);
}
