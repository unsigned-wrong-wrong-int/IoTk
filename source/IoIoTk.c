#include "IoState.h"
#include "IoSeq.h"
#include "IoNumber.h"

#include "IoIoTk.h"

#define DATA(self) ((IoIoTkData *)IoObject_dataPointer(self))

#define ERROR_IF_PROTO(self, name, m) \
   if (DATA(self) == NULL) IoState_error_(IOSTATE, m, \
      "method '%s' called on IoTk prototype, use 'IoTk clone'", name);

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
   IoObject_setDataPointer_(self, NULL);
   IoState_registerProtoWithId_((IoState *)state, self, protoId);

   IoMethodTable methodTable[] = {
      {"mainloop", IoIoTk_mainloop},
      {"eval", IoIoTk_eval},
      {"define", IoIoTk_define},
      {"undef", IoIoTk_undef},
      {NULL, NULL}
   };
   IoObject_addMethodTable_(self, methodTable);

   return self;
}

IoIoTk *IoIoTk_rawClone(IoIoTk *proto) {
   IoIoTk *self = IoObject_rawClonePrimitive(proto);
   IoObject_setDataPointer_(self, calloc(1, sizeof(IoIoTkData)));
   DATA(self)->interp = Tcl_CreateInterp();
   Tcl_Init(DATA(self)->interp);
   Tk_Init(DATA(self)->interp);
   DATA(self)->commands = PHash_new();
   return self;
}

void IoIoTk_free(IoIoTk *self) {
   if (DATA(self) == NULL) return;
   if (!Tcl_InterpDeleted(DATA(self)->interp)) {
      Tcl_DeleteInterp(DATA(self)->interp);
   }
   PHash_free(DATA(self)->commands);
   free(IoObject_dataPointer(self));
}

void IoIoTk_mark(IoIoTk *self) {
   if (DATA(self) == NULL) return;
   PHash *commands = DATA(self)->commands;
   PHASH_FOREACH(commands, key, value,
      IoObject_shouldMark((IoObject *)key);
      IoObject_shouldMark((IoObject *)value));
}

IoObject *IoIoTk_mainloop(IoIoTk *self, IoObject *locals, IoMessage *m) {
   Tk_MainLoop();
   return IONIL(self);
}

IoObject *IoIoTk_eval(IoIoTk *self, IoObject *locals, IoMessage *m) {
   ERROR_IF_PROTO(self, "eval", m);
   IoSymbol *cmd = IoMessage_locals_symbolArgAt_(m, locals, 0);
   char *str = CSTRING(cmd);
   size_t len = IoSeq_rawSizeInBytes(cmd);
   int r = Tcl_EvalEx(DATA(self)->interp, str, len, 0);
   const char *result = Tcl_GetStringResult(DATA(self)->interp);
   if (r == TCL_ERROR) {
      IoState_error_(IOSTATE, m, "Tcl/Tk error: %s", result);
   }
   return IoSeq_newWithCString_(IOSTATE, result);
}

int TkCmdProc(ClientData data, Tcl_Interp *interp, int argc, const char *argv[]) {
   IoObject *self = (IoObject *)data;
   IoObject *result = NULL;
   char *errfmt = NULL, *str = NULL;
   if (argc <= 1) {
      errfmt = "wrong # args: should be \"%s methodName ?arg ...?\"";
      str = argv[0];
      goto error;
   }
   IoMessage *m = IoMessage_newWithName_(IOSTATE, IOSYMBOL(argv[1]));
   for (int i = 2; i < argc; ++i) {
      IoMessage_addCachedArg_(m, IoSeq_newWithCString_(IOSTATE, argv[i]));
   }
   IoCoroutine *coro = IoCoroutine_newWithTry(IOSTATE, self, self, m);
   IoObject *e = IoCoroutine_rawException(coro);
   if (!ISNIL(e)) {
      errfmt = "error in Io method: %s";
      result = IoObject_rawGetSlot_(e, IOSYMBOL("error"));
      str = ISSEQ(result) ? CSTRING(result) : "";
      goto error;
   }
   result = IoCoroutine_rawResult(coro);
   if (!ISNIL(result)) {
      IoCoroutine_try(coro, result, result, IOSTATE->asStringMessage);
      result = IoCoroutine_rawResult(coro);
      if (ISNIL(IoCoroutine_rawException(coro)) && ISSEQ(result)) {
         str = CSTRING(result);
      }
   }
   Tcl_FreeResult(interp);
   Tcl_SetResult(interp, str, TCL_VOLATILE);
   return TCL_OK;
error:
   UArray *ua = UArray_newWithFormat_(errfmt, str);
   Tcl_FreeResult(interp);
   Tcl_SetResult(interp, (char *)UArray_data(ua), TCL_VOLATILE);
   UArray_free(ua);
   return TCL_ERROR;
}

IoObject *IoIoTk_define(IoObject *self, IoObject *locals, IoMessage *m) {
   ERROR_IF_PROTO(self, "define", m);
   IoSymbol *key = IoMessage_locals_symbolArgAt_(m, locals, 0);
   IoObject *value = IoMessage_locals_valueArgAt_(m, locals, 1);
   PHash_at_put_(DATA(self)->commands, key, value);
   Tcl_CreateCommand(DATA(self)->interp, CSTRING(key), TkCmdProc, value, NULL);
   return IONIL(self);
}

IoObject *IoIoTk_undef(IoObject *self, IoObject *locals, IoMessage *m) {
   ERROR_IF_PROTO(self, "undef", m);
   IoSymbol *key = IoMessage_locals_symbolArgAt_(m, locals, 0);
   Tcl_DeleteCommand(DATA(self)->interp, CSTRING(key));
   PHash_removeKey_(DATA(self)->commands, key);
   return IONIL(self);
}
