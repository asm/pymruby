#include <Python.h>


#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/hash.h"
#include "mruby/khash.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/data.h"
#include "mruby/proc.h"
#include "mruby/compile.h"
#include "mruby/opcode.h"
#include "mruby/error.h"

typedef struct {
  PyObject_HEAD
  mrb_state *mrb;
  uint16_t stack_keep;
  mrbc_context *ctx;
  int ai;
} PymrubyObject;

static PyObject *eval(PymrubyObject *self, PyObject *args);
static int Mruby_init(PymrubyObject *self, PyObject *args);
static PyObject *Mruby_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void Mruby_dealloc(PymrubyObject *self);

static char pymruby_docs[] = "eval(): what?\n";

static PyMethodDef mruby_functions[] = {
  {
    "eval",
    (PyCFunction)eval,
    METH_VARARGS,
    pymruby_docs,
  },
  {NULL}
};


static PyTypeObject Pymruby_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /* ob_size*/
    "pymruby.Pymruby",         /* tp_name*/
    sizeof(PymrubyObject),     /* tp_basicsize*/
    0,                         /* tp_itemsize*/
    (destructor)Mruby_dealloc, /* tp_dealloc*/
    0,                         /* tp_print*/
    0,                         /* tp_getattr*/
    0,                         /* tp_setattr*/
    0,                         /* tp_compare*/
    0,                         /* tp_repr*/
    0,                         /* tp_as_number*/
    0,                         /* tp_as_sequence*/
    0,                         /* tp_as_mapping*/
    0,                         /* tp_hash */
    0,                         /* tp_call*/
    0,                         /* tp_str*/
    0,                         /* tp_getattro*/
    0,                         /* tp_setattro*/
    0,                         /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /* tp_flags*/
    "Pymruby objects",         /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    mruby_functions,           /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Mruby_init,      /* tp_init */
    0,                         /* tp_alloc */
    Mruby_new,                 /* tp_new */
};

