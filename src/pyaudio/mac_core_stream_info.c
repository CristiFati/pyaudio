#ifdef MACOSX

#include "mac_core_stream_info.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "pa_mac_core.h"
#include "portaudio.h"

static void cleanup(PyAudioMacCoreStreamInfo *self) {
  if (self->paMacCoreStreamInfo != NULL) {
    free(self->paMacCoreStreamInfo);
    self->paMacCoreStreamInfo = NULL;
  }

  if (self->channelMap != NULL) {
    free(self->channelMap);
    self->channelMap = NULL;
  }

  self->flags = paMacCorePlayNice;
  self->channelMapSize = 0;
}

static void dealloc(PyAudioMacCoreStreamInfo *self) {
  cleanup(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static int init(PyObject *_self, PyObject *args, PyObject *kwargs) {
  PyAudioMacCoreStreamInfo *self = (PyAudioMacCoreStreamInfo *)_self;
  PyObject *channel_map = NULL;
  int flags = paMacCorePlayNice;

  static char *kwlist[] = {"flags", "channel_map", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|iO", kwlist, &flags,
                                   &channel_map)) {
    return -1;
  }

  cleanup(self);

  if (channel_map != NULL) {
    if (!PyTuple_Check(channel_map)) {
      PyErr_SetString(PyExc_ValueError, "Channel map must be a tuple");
      return -1;
    }

    // generate SInt32 channelMap
    self->channelMapSize = (int)PyTuple_Size(channel_map);
    self->channelMap = (SInt32 *)malloc(sizeof(SInt32) * self->channelMapSize);

    if (self->channelMap == NULL) {
      PyErr_SetString(PyExc_SystemError, "Out of memory");
      cleanup(self);
      return -1;
    }

    PyObject *element;
    int i;
    for (i = 0; i < self->channelMapSize; ++i) {
      element = PyTuple_GetItem(channel_map, i);
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
      self->channelMap[i] = (SInt32)PyLong_AsLong(long_element);
      Py_DECREF(long_element);
    }
  }

  self->paMacCoreStreamInfo =
      (PaMacCoreStreamInfo *)malloc(sizeof(PaMacCoreStreamInfo));

  if (self->paMacCoreStreamInfo == NULL) {
    PyErr_SetString(PyExc_SystemError, "Out of memeory");
    cleanup(self);
    return -1;
  }

  PaMacCore_SetupStreamInfo(self->paMacCoreStreamInfo, flags);

  if (self->channelMap) {
    PaMacCore_SetupChannelMap(self->paMacCoreStreamInfo, self->channelMap,
                              self->channelMapSize);
  }

  self->flags = flags;
  return 0;
}

static PyObject *get_flags(PyAudioMacCoreStreamInfo *self, void *closure) {
  return PyLong_FromLong(self->flags);
}

static PyObject *get_channel_map(PyAudioMacCoreStreamInfo *self,
                                 void *closure) {
  if (self->channelMap == NULL || self->channelMapSize == 0) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  int i;
  PyObject *channelMapTuple = PyTuple_New(self->channelMapSize);
  for (i = 0; i < self->channelMapSize; ++i) {
    PyObject *element = PyLong_FromLong(self->channelMap[i]);
    if (!element) {
      PyErr_SetString(PyExc_SystemError, "Invalid channel map");
      return NULL;
    }

    if (PyTuple_SetItem(channelMapTuple, i,
                        PyLong_FromLong(self->channelMap[i]))) {
      // non-zero on error
      PyErr_SetString(PyExc_SystemError, "Can't create channel map.");
      return NULL;
    }
  }
  return channelMapTuple;
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
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("macOS Specific HostAPI configuration"),
    .tp_getset = get_setters,
    .tp_init = (int (*)(PyObject *, PyObject *, PyObject *))init,
    .tp_new = PyType_GenericNew,
};

#endif  // MACOSX
