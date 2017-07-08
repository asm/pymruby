#include "pymruby.h"

void mrb_exc_set(mrb_state *mrb, mrb_value exc);
PyMODINIT_FUNC initpymruby(void);

static mrb_value timeout_error;
static int borked = 0;
static uint32_t mem = 0;

static void timeout(int sig) {
  borked = 1;
}

void *counting_alloc(mrb_state *mrb, void *p, size_t size, void *ud) {
  mem += size;
  return mrb_default_allocf(mrb, p, size, ud);
}

static void hook(struct mrb_state* mrb, mrb_irep *irep, mrb_code *pc, mrb_value *regs) {
  if (borked) {
    if (borked == 1) {
      mrb_exc_set(mrb, timeout_error);
    }

    if (borked > 20) { 
      if ((*pc & 0x7f) == OP_JMPIF) {
        *pc = OP_STOP;
      }
    }
    //printf("op %li\n", (uint32_t)(*pc) & 0x7f);
    //printf("OP_NOP %li\n", (uint32_t)OP_JMPIF);
    borked++;
  }

  if (mem > 1000000) {
    mrb_exc_set(mrb, timeout_error);
  }
}

static PyObject *eval(PymrubyObject *self, PyObject *args) {
  char *code;

  if(!PyArg_ParseTuple(args, "s", &code))
    return Py_BuildValue("s", "Arg parse fail");

  struct mrb_parser_state *parser = mrb_parser_new(self->mrb);

  parser->s = code;
  parser->send = code + strlen(code);
  parser->lineno = 1;
  mrb_parser_parse(parser, self->ctx);

  if (parser->nerr > 0) {
    char error[1000];
    snprintf(error, 1000, "line %d: %s", parser->error_buffer[0].lineno, parser->error_buffer[0].message);
    return Py_BuildValue("s", error);
  }

  struct RProc *proc = mrb_generate_code(self->mrb, parser);


  signal(SIGALRM, timeout);
  alarm(1);

  mrb_value result = mrb_context_run(
    self->mrb,
    proc,
    mrb_top_self(self->mrb),
    self->stack_keep
  );

  borked = 0;
  mem = 0;

  signal(SIGALRM, SIG_IGN);
  alarm(0);

  self->stack_keep = proc->body.irep->nlocals;

  char *buff;
  PyObject *ret;

  // exception?
  if (self->mrb->exc) {
    mrb_value exc = mrb_funcall(self->mrb, mrb_obj_value(self->mrb->exc), "inspect", 0);

    buff = malloc(RSTRING_LEN(exc));
    memcpy(buff, RSTRING_PTR(exc), RSTRING_LEN(exc));
    ret = Py_BuildValue("s#", buff, RSTRING_LEN(exc));

    self->mrb->exc = 0;
  } else {
    enum mrb_vtype t = mrb_type(result);
    mrb_value val;

    switch(t) {
    case MRB_TT_STRING:
      buff = malloc(RSTRING_LEN(result));
      memcpy(buff, RSTRING_PTR(result), RSTRING_LEN(result));
      ret = Py_BuildValue("s#", buff, RSTRING_LEN(result));
      break;
    case MRB_TT_FIXNUM:
      ret = Py_BuildValue("n", mrb_fixnum(result));
      break;
    default:
      val = mrb_funcall(self->mrb, result, "inspect", 0);
      if (self->mrb->exc) {
        val = mrb_funcall(self->mrb, mrb_obj_value(self->mrb->exc), "inspect", 0);
      }
      if(!mrb_string_p(val)) {
        val = mrb_obj_as_string(self->mrb, result);
      }

      buff = malloc(RSTRING_LEN(val));
      memcpy(buff, RSTRING_PTR(val), RSTRING_LEN(val));
      ret = Py_BuildValue("s#", buff, RSTRING_LEN(val));

      break;
    }
  }

  mrb_gc_arena_restore(self->mrb, self->ai);
  mrb_parser_free(parser);

  return ret;
}

static void Mruby_dealloc(PymrubyObject *self) {
  mrbc_context_free(self->mrb, self->ctx);
  mrb_close(self->mrb);

  self->ob_type->tp_free((PyObject*)self);
}


static int Mruby_init(PymrubyObject *self, PyObject *args) {
  self->mrb = mrb_open();
  self->mrb->code_fetch_hook = hook;
  self->mrb->allocf = counting_alloc;
  self->stack_keep = 0;
  self->ctx = mrbc_context_new(self->mrb);
  self->ctx->lineno = 1;
  self->ctx->capture_errors = TRUE;
  mrbc_filename(self->mrb, self->ctx, "lol");

  mrb_value err_str;
  err_str = mrb_str_new_lit(self->mrb, "timeout!");
  timeout_error = mrb_exc_new_str(self->mrb, mrb_exc_get(self->mrb, "RuntimeError"), err_str);

  self->ai = mrb_gc_arena_save(self->mrb);

  return 0;
}

static PyObject *Mruby_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PymrubyObject *self;

  self = (PymrubyObject*)type->tp_alloc(type, 0);

  return (PyObject *)self;
}

PyMODINIT_FUNC initpymruby(void) {
  Pymruby_Type.tp_new = PyType_GenericNew;
  if(PyType_Ready(&Pymruby_Type) < 0)
    return;

  PyObject *mod = Py_InitModule3("pymruby", NULL, "Python MRuby Bindings");

  Py_INCREF(&Pymruby_Type);
  PyModule_AddObject(mod, "Pymruby", (PyObject *)&Pymruby_Type);
}
