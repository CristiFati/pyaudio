#include "host_api.h"

#include <stdio.h>

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

// Wrapper object for the PaHostApiInfo struct.

typedef struct {
  // clang-format off
  PyObject_HEAD
  // clang-format on
  const PaHostApiInfo *api_info;
} PyAudioHostApiInfo;

// Property getters for PyAudioHostApiInfo:

static PyObject *get_structVersion(PyAudioHostApiInfo *self, void *closure) {
  if ((!self->api_info)) {
    PyErr_SetString(PyExc_AttributeError, "No HostApi Info available");
    return NULL;
  }
  return PyLong_FromLong(self->api_info->structVersion);
}

static PyObject *get_type(PyAudioHostApiInfo *self, void *closure) {
  if ((!self->api_info)) {
    PyErr_SetString(PyExc_AttributeError, "No HostApi Info available");
    return NULL;
  }
  return PyLong_FromLong((long)self->api_info->type);
}

static PyObject *get_name(PyAudioHostApiInfo *self, void *closure) {
  if ((!self->api_info) || (self->api_info->name == NULL)) {
    PyErr_SetString(PyExc_AttributeError, "No HostApi Info available");
    return NULL;
  }
  return PyUnicode_FromString(self->api_info->name);
}

static PyObject *get_deviceCount(PyAudioHostApiInfo *self, void *closure) {
  if ((!self->api_info)) {
    PyErr_SetString(PyExc_AttributeError, "No HostApi Info available");
    return NULL;
  }
  return PyLong_FromLong(self->api_info->deviceCount);
}

static PyObject *get_defaultInputDevice(PyAudioHostApiInfo *self,
                                        void *closure) {
  if ((!self->api_info)) {
    PyErr_SetString(PyExc_AttributeError, "No HostApi Info available");
    return NULL;
  }
  return PyLong_FromLong(self->api_info->defaultInputDevice);
}

static PyObject *get_defaultOutputDevice(PyAudioHostApiInfo *self,
                                         void *closure) {
  if ((!self->api_info)) {
    PyErr_SetString(PyExc_AttributeError, "No HostApi Info available");
    return NULL;
  }
  return PyLong_FromLong(self->api_info->defaultOutputDevice);
}

static int antiset(PyAudioHostApiInfo *self, PyObject *value, void *closure) {
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError,
                  "Fields read-only: cannot modify values");
  return -1;
}

static void dealloc(PyAudioHostApiInfo *self) {
  self->api_info = NULL;
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyGetSetDef get_setters[] = {
    {"name", (getter)get_name, (setter)antiset, "host api name", NULL},

    {"structVersion", (getter)get_structVersion, (setter)antiset,
     "struct version", NULL},

    {"type", (getter)get_type, (setter)antiset, "host api type", NULL},

    {"deviceCount", (getter)get_deviceCount, (setter)antiset,
     "number of devices", NULL},

    {"defaultInputDevice", (getter)get_defaultInputDevice, (setter)antiset,
     "default input device index", NULL},

    {"defaultOutputDevice", (getter)get_defaultOutputDevice, (setter)antiset,
     "default output device index", NULL},

    {NULL}};

PyTypeObject PyAudioHostApiInfoType = {
    // clang-format off
    PyVarObject_HEAD_INIT(NULL, 0)
    // clang-format on
    .tp_name = "_portaudio.paHostApiInfo",
    .tp_basicsize = sizeof(PyAudioHostApiInfo),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("PortAudio PaHostApiInfo"),
    .tp_getset = get_setters,
    .tp_new = PyType_GenericNew,
};

// Public Functions

// Creates and returns a PyAudioHostApiInfo (PyAudioHostApiInfoType).
PyObject *PyAudio_GetHostApiInfo(PyObject *self, PyObject *args) {
  PaHostApiIndex index;
  if (!PyArg_ParseTuple(args, "i", &index)) {
    return NULL;
  }

  const PaHostApiInfo *pa_hostapi_info = Pa_GetHostApiInfo(index);
  if (!pa_hostapi_info) {
    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", paInvalidHostApi,
                                                 "Invalid host api info"));
    return NULL;
  }

  PyAudioHostApiInfo *py_hostapi_info = (PyAudioHostApiInfo *)PyObject_New(
      PyAudioHostApiInfo, &PyAudioHostApiInfoType);
  py_hostapi_info->api_info = pa_hostapi_info;
  return (PyObject *)py_hostapi_info;
}

PyObject *PyAudio_GetHostApiCount(PyObject *self, PyObject *args) {
  PaHostApiIndex count;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  count = Pa_GetHostApiCount();

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

PyObject *PyAudio_GetDefaultHostApi(PyObject *self, PyObject *args) {
  PaHostApiIndex index;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  index = Pa_GetDefaultHostApi();

  if (index < 0) {
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

PyObject *PyAudio_HostApiTypeIdToHostApiIndex(PyObject *self, PyObject *args) {
  PaHostApiTypeId typeid;
  PaHostApiIndex index;

  if (!PyArg_ParseTuple(args, "i", &typeid)) {
    return NULL;
  }

  index = Pa_HostApiTypeIdToHostApiIndex(typeid);

  if (index < 0) {
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

PyObject *PyAudio_HostApiDeviceIndexToDeviceIndex(PyObject *self,
                                                  PyObject *args) {
  PaHostApiIndex apiIndex;
  int hostApiDeviceindex;
  PaDeviceIndex devIndex;

  if (!PyArg_ParseTuple(args, "ii", &apiIndex, &hostApiDeviceindex)) {
    return NULL;
  }

  devIndex = Pa_HostApiDeviceIndexToDeviceIndex(apiIndex, hostApiDeviceindex);
  if (devIndex < 0) {
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", devIndex);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(devIndex));
#endif

    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", devIndex,
                                                 Pa_GetErrorText(devIndex)));
    return NULL;
  }

  return PyLong_FromLong(devIndex);
}
