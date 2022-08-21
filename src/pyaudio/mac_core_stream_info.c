#ifdef MACOSX

#include "mac_core_stream_info.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"
#include "pa_mac_core.h"

typedef _pyAudio_MacOSX_hostApiSpecificStreamInfo _pyAudio_Mac_HASSI;

static void _pyAudio_MacOSX_hostApiSpecificStreamInfo_cleanup(
    _pyAudio_Mac_HASSI *self) {
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

static void _pyAudio_MacOSX_hostApiSpecificStreamInfo_dealloc(
    _pyAudio_Mac_HASSI *self) {
  _pyAudio_MacOSX_hostApiSpecificStreamInfo_cleanup(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static int _pyAudio_MacOSX_hostApiSpecificStreamInfo_init(PyObject *_self,
                                                          PyObject *args,
                                                          PyObject *kwargs) {
  _pyAudio_Mac_HASSI *self = (_pyAudio_Mac_HASSI *)_self;
  PyObject *channel_map = NULL;
  int flags = paMacCorePlayNice;

  static char *kwlist[] = {"flags", "channel_map", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|iO", kwlist, &flags,
                                   &channel_map)) {
    return -1;
  }

  _pyAudio_MacOSX_hostApiSpecificStreamInfo_cleanup(self);

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
      _pyAudio_MacOSX_hostApiSpecificStreamInfo_cleanup(self);
      return -1;
    }

    PyObject *element;
    int i;
    for (i = 0; i < self->channelMapSize; ++i) {
      element = PyTuple_GetItem(channel_map, i);
      if (element == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "Internal error: out of bounds index");
        _pyAudio_MacOSX_hostApiSpecificStreamInfo_cleanup(self);
        return -1;
      }

      if (!PyNumber_Check(element)) {
        PyErr_SetString(PyExc_ValueError,
                        "Channel Map must consist of integer elements");
        _pyAudio_MacOSX_hostApiSpecificStreamInfo_cleanup(self);
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
    _pyAudio_MacOSX_hostApiSpecificStreamInfo_cleanup(self);
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

static PyObject *_pyAudio_MacOSX_hostApiSpecificStreamInfo_get_flags(
    _pyAudio_Mac_HASSI *self, void *closure) {
  return PyLong_FromLong(self->flags);
}

static PyObject *_pyAudio_MacOSX_hostApiSpecificStreamInfo_get_channel_map(
    _pyAudio_Mac_HASSI *self, void *closure) {
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

static int _pyAudio_MacOSX_hostApiSpecificStreamInfo_antiset(
    _pyAudio_Mac_HASSI *self, PyObject *value, void *closure) {
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError,
                  "Fields read-only: cannot modify values");
  return -1;
}

static PyGetSetDef _pyAudio_MacOSX_hostApiSpecificStreamInfo_getseters[] = {
    {"flags", (getter)_pyAudio_MacOSX_hostApiSpecificStreamInfo_get_flags,
     (setter)_pyAudio_MacOSX_hostApiSpecificStreamInfo_antiset, "flags", NULL},

    {"channel_map",
     (getter)_pyAudio_MacOSX_hostApiSpecificStreamInfo_get_channel_map,
     (setter)_pyAudio_MacOSX_hostApiSpecificStreamInfo_antiset, "channel map",
     NULL},

    {NULL}};

PyTypeObject _pyAudio_MacOSX_hostApiSpecificStreamInfoType = {
  // clang-format off
  PyVarObject_HEAD_INIT(NULL, 0)
  // clang-format on
    "_portaudio.PaMacCoreStreamInfo",                  /*tp_name*/
    sizeof(_pyAudio_MacOSX_hostApiSpecificStreamInfo), /*tp_basicsize*/
    0,                                                 /*tp_itemsize*/
    /*tp_dealloc*/
    (destructor)_pyAudio_MacOSX_hostApiSpecificStreamInfo_dealloc,
    0,                                                   /*tp_print*/
    0,                                                   /*tp_getattr*/
    0,                                                   /*tp_setattr*/
    0,                                                   /*tp_compare*/
    0,                                                   /*tp_repr*/
    0,                                                   /*tp_as_number*/
    0,                                                   /*tp_as_sequence*/
    0,                                                   /*tp_as_mapping*/
    0,                                                   /*tp_hash */
    0,                                                   /*tp_call*/
    0,                                                   /*tp_str*/
    0,                                                   /*tp_getattro*/
    0,                                                   /*tp_setattro*/
    0,                                                   /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                                  /*tp_flags*/
    "Mac OS X Specific HostAPI configuration",           /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    0,                                                   /* tp_methods */
    0,                                                   /* tp_members */
    _pyAudio_MacOSX_hostApiSpecificStreamInfo_getseters, /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (int (*)(PyObject *, PyObject *, PyObject *))
        _pyAudio_MacOSX_hostApiSpecificStreamInfo_init, /* tp_init */
    0,                                                  /* tp_alloc */
    0,                                                  /* tp_new */
};

#endif  // MACOSX
