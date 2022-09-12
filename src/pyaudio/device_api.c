#include "device_api.h"

#include <stdio.h>

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

// Wrapper object for the PaDeviceInfo struct.

typedef struct {
  // clang-format off
  PyObject_HEAD
  const PaDeviceInfo *device_info;
  // clang-format on
} PyAudioDeviceInfo;

// Property getters for PyAudioDeviceInfo:

static PyObject *get_structVersion(PyAudioDeviceInfo *self, void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->device_info->structVersion);
}

static PyObject *get_name(PyAudioDeviceInfo *self, void *closure) {
  if ((!self->device_info) || (self->device_info->name == NULL)) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyBytes_FromString(self->device_info->name);
}

static PyObject *get_hostApi(PyAudioDeviceInfo *self, void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->device_info->hostApi);
}

static PyObject *get_maxInputChannels(PyAudioDeviceInfo *self, void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->device_info->maxInputChannels);
}

static PyObject *get_maxOutputChannels(PyAudioDeviceInfo *self, void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyLong_FromLong(self->device_info->maxOutputChannels);
}

static PyObject *get_defaultLowInputLatency(PyAudioDeviceInfo *self,
                                            void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->device_info->defaultLowInputLatency);
}

static PyObject *get_defaultLowOutputLatency(PyAudioDeviceInfo *self,
                                             void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->device_info->defaultLowOutputLatency);
}

static PyObject *get_defaultHighInputLatency(PyAudioDeviceInfo *self,
                                             void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->device_info->defaultHighInputLatency);
}

static PyObject *get_defaultHighOutputLatency(PyAudioDeviceInfo *self,
                                              void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->device_info->defaultHighOutputLatency);
}

static PyObject *get_defaultSampleRate(PyAudioDeviceInfo *self, void *closure) {
  if (!self->device_info) {
    PyErr_SetString(PyExc_AttributeError, "No Device Info available");
    return NULL;
  }
  return PyFloat_FromDouble(self->device_info->defaultSampleRate);
}

static int antiset(PyAudioDeviceInfo *self, PyObject *value, void *closure) {
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError,
                  "Fields read-only: cannot modify values");
  return -1;
}

static void dealloc(PyAudioDeviceInfo *self) {
  self->device_info = NULL;
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

// Public Device API functions

// Creates and returns a PyAudioDeviceInfo (PyAudioDeviceInfoType).
PyObject *PyAudio_GetDeviceInfo(PyObject *self, PyObject *args) {
  PaDeviceIndex index;
  if (!PyArg_ParseTuple(args, "i", &index)) {
    return NULL;
  }

  const PaDeviceInfo *pa_device_info = Pa_GetDeviceInfo(index);
  if (!pa_device_info) {
    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", paInvalidDevice,
                                                 "Invalid device info"));
    return NULL;
  }

  PyAudioDeviceInfo *py_device_info = (PyAudioDeviceInfo *)PyObject_New(
      PyAudioDeviceInfo, &PyAudioDeviceInfoType);
  py_device_info->device_info = pa_device_info;
  return (PyObject *)py_device_info;
}

PyObject *PyAudio_GetDeviceCount(PyObject *self, PyObject *args) {
  PaDeviceIndex count;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  count = Pa_GetDeviceCount();
  if (count < 0) {
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", count);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(count));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", count, Pa_GetErrorText(count)));
    return NULL;
  }

  return PyLong_FromLong(count);
}

PyObject *PyAudio_GetDefaultInputDevice(PyObject *self, PyObject *args) {
  PaDeviceIndex index;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  index = Pa_GetDefaultInputDevice();
  if (index == paNoDevice) {
    PyErr_SetString(PyExc_IOError, "No Default Input Device Available");
    return NULL;
  } else if (index < 0) {
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", index);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(index));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", index, Pa_GetErrorText(index)));
    return NULL;
  }

  return PyLong_FromLong(index);
}

PyObject *PyAudio_GetDefaultOutputDevice(PyObject *self, PyObject *args) {
  PaDeviceIndex index;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  index = Pa_GetDefaultOutputDevice();
  if (index == paNoDevice) {
    PyErr_SetString(PyExc_IOError, "No Default Output Device Available");
    return NULL;
  } else if (index < 0) {
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", index);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(index));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", index, Pa_GetErrorText(index)));
    return NULL;
  }

  return PyLong_FromLong(index);
}
