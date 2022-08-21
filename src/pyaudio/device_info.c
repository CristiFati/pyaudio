// PyAudioDeviceInfo: Python object wrapper for PortAudio's PaDeviceInfo struct.

#include "device_info.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

static PyObject *get_structVersion(PyAudioDeviceInfo *self, void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->devInfo->structVersion);
}

static PyObject *get_name(PyAudioDeviceInfo *self, void *closure) {
  if ((!self->devInfo) || (self->devInfo->name == NULL)) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyBytes_FromString(self->devInfo->name);
}

static PyObject *get_hostApi(PyAudioDeviceInfo *self, void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->devInfo->hostApi);
}

static PyObject *get_maxInputChannels(PyAudioDeviceInfo *self, void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->devInfo->maxInputChannels);
}

static PyObject *get_maxOutputChannels(PyAudioDeviceInfo *self, void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->devInfo->maxOutputChannels);
}

static PyObject *get_defaultLowInputLatency(PyAudioDeviceInfo *self,
                                            void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->devInfo->defaultLowInputLatency);
}

static PyObject *get_defaultLowOutputLatency(PyAudioDeviceInfo *self,
                                             void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->devInfo->defaultLowOutputLatency);
}

static PyObject *get_defaultHighInputLatency(PyAudioDeviceInfo *self,
                                             void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->devInfo->defaultHighInputLatency);
}

static PyObject *get_defaultHighOutputLatency(PyAudioDeviceInfo *self,
                                              void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->devInfo->defaultHighOutputLatency);
}

static PyObject *get_defaultSampleRate(PyAudioDeviceInfo *self, void *closure) {
  if (!self->devInfo) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->devInfo->defaultSampleRate);
}

static int antiset(PyAudioDeviceInfo *self, PyObject *value, void *closure) {
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError,
                  "Fields read-only: cannot modify values");
  return -1;
}

static void dealloc(PyAudioDeviceInfo *self) {
  self->devInfo = NULL;
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyGetSetDef get_setters[] = {
    {"name", (getter)get_name, (setter)antiset, "device name", NULL},

    {"structVersion", (getter)get_structVersion, (setter)antiset,
     "struct version", NULL},

    {"hostApi", (getter)get_hostApi, (setter)antiset, "host api index", NULL},

    {"maxInputChannels", (getter)get_maxInputChannels, (setter)antiset,
     "max input channels", NULL},

    {"maxOutputChannels", (getter)get_maxOutputChannels, (setter)antiset,
     "max output channels", NULL},

    {"defaultLowInputLatency", (getter)get_defaultLowInputLatency,
     (setter)antiset, "default low input latency", NULL},

    {"defaultLowOutputLatency", (getter)get_defaultLowOutputLatency,
     (setter)antiset, "default low output latency", NULL},

    {"defaultHighInputLatency", (getter)get_defaultHighInputLatency,
     (setter)antiset, "default high input latency", NULL},

    {"defaultHighOutputLatency", (getter)get_defaultHighOutputLatency,
     (setter)antiset, "default high output latency", NULL},

    {"defaultSampleRate", (getter)get_defaultSampleRate, (setter)antiset,
     "default sample rate", NULL},

    {NULL}};

PyTypeObject PyAudioDeviceInfoType = {
    // clang-format off
    PyVarObject_HEAD_INIT(NULL, 0)
    // clang-format on
    .tp_name = "_portaudio.paDeviceInfo",
    .tp_basicsize = sizeof(PyAudioDeviceInfo),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("PortAudio PaDeviceInfo"),
    .tp_getset = get_setters,
    .tp_new = PyType_GenericNew,
};

PyAudioDeviceInfo *CreatePyAudioDeviceInfo(void) {
  PyAudioDeviceInfo *obj;
  /* Don't allow subclassing */
  obj = (PyAudioDeviceInfo *)PyObject_New(PyAudioDeviceInfo,
                                          &PyAudioDeviceInfoType);
  return obj;
}
