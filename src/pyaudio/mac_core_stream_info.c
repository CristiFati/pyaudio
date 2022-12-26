#ifdef MACOS

#include "mac_core_stream_info.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"
#include "pa_mac_core.h"

static void cleanup(PyAudioMacCoreStreamInfo *self) {
  if (self->channel_map != NULL) {
    free(self->channel_map);
    self->channel_map = NULL;
  }
  self->flags = paMacCorePlayNice;
  self->channel_map_size = 0;
}

static void dealloc(PyAudioMacCoreStreamInfo *self) {
  cleanup(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static int init(PyObject *_self, PyObject *args, PyObject *kwargs) {
  PyAudioMacCoreStreamInfo *self = (PyAudioMacCoreStreamInfo *)_self;
  // Init struct with default values.
  self->flags = paMacCorePlayNice;
  self->channel_map = NULL;
  self->channel_map_size = 0;
  PaMacCore_SetupStreamInfo(&self->stream_info, self->flags);

  PyObject *channel_map = NULL;
  static char *kwlist[] = {"flags", "channel_map", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|iO", kwlist, &self->flags,
                                   &channel_map)) {
    return -1;
  }

  if (channel_map != NULL) {
    if (!PyTuple_Check(channel_map)) {
      PyErr_SetString(PyExc_ValueError, "Channel map must be a tuple");
      return -1;
    }

    // generate SInt32 channel_map
    self->channel_map_size = (int)PyTuple_Size(channel_map);
    self->channel_map =
        (SInt32 *)malloc(sizeof(SInt32) * self->channel_map_size);

    if (self->channel_map == NULL) {
      PyErr_SetString(PyExc_SystemError, "Out of memory");
      cleanup(self);
      return -1;
    }

    for (int i = 0; i < self->channel_map_size; ++i) {
      PyObject *element = PyTuple_GetItem(channel_map, i);
      if (element == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "Internal error: out of bounds index");
        cleanup(self);
        return -1;
      }

      if (!PyNumber_Check(element)) {
        PyErr_SetString(PyExc_ValueError,
                        "Channel Map must consist of integer elements");
        cleanup(self);
        return -1;
      }

      PyObject *long_element = PyNumber_Long(element);
      self->channel_map[i] = (SInt32)PyLong_AsLong(long_element);
      Py_DECREF(long_element);
    }

    PaMacCore_SetupChannelMap(&self->stream_info, self->channel_map,
                              self->channel_map_size);
  }

  return 0;
}

static PyObject *get_flags(PyAudioMacCoreStreamInfo *self, void *closure) {
  return PyLong_FromLong(self->flags);
}

static PyObject *get_channel_map(PyAudioMacCoreStreamInfo *self,
                                 void *closure) {
  if (self->channel_map == NULL || self->channel_map_size == 0) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject *channel_map_tuple = PyTuple_New(self->channel_map_size);
  for (int i = 0; i < self->channel_map_size; ++i) {
    PyObject *element = PyLong_FromLong(self->channel_map[i]);
    if (!element) {
      PyErr_SetString(PyExc_SystemError, "Invalid channel map");
      return NULL;
    }

    if (PyTuple_SetItem(channel_map_tuple, i,
                        PyLong_FromLong(self->channel_map[i]))) {
      // non-zero on error
      PyErr_SetString(PyExc_SystemError, "Can't create channel map.");
      return NULL;
    }
  }
  return channel_map_tuple;
}

static int antiset(PyAudioMacCoreStreamInfo *self, PyObject *value,
                   void *closure) {
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError,
                  "Fields read-only: cannot modify values");
  return -1;
}

static PyGetSetDef get_setters[] = {
    {"flags", (getter)get_flags, (setter)antiset, "flags", NULL},
    {"channel_map", (getter)get_channel_map, (setter)antiset, "channel map",
     NULL},
    {NULL}};

PyTypeObject PyAudioMacCoreStreamInfoType = {
    // clang-format off
    PyVarObject_HEAD_INIT(NULL, 0)
    // clang-format on
    .tp_name = "_portaudio.PaMacCoreStreamInfo",
    .tp_basicsize = sizeof(PyAudioMacCoreStreamInfo),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = PyDoc_STR("macOS Specific HostAPI configuration"),
    .tp_getset = get_setters,
    .tp_init = (initproc)init,
    .tp_new = PyType_GenericNew,
};

#endif  // MACOS
