#include "stream.h"

#include <string.h>

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

static void dealloc(PyAudioStream *self) {
  PyAudioStream_Cleanup(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *get_structVersion(PyAudioStream *self, void *closure) {
  if (!PyAudioStream_IsOpen(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->context.stream);
  if (!stream_info) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  return PyLong_FromLong(stream_info->structVersion);
}

static PyObject *get_inputLatency(PyAudioStream *self, void *closure) {
  if (!PyAudioStream_IsOpen(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->context.stream);
  if (!stream_info) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  return PyFloat_FromDouble(stream_info->inputLatency);
}

static PyObject *get_outputLatency(PyAudioStream *self, void *closure) {
  if (!PyAudioStream_IsOpen(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->context.stream);
  if (!stream_info) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  return PyFloat_FromDouble(stream_info->outputLatency);
}

static PyObject *get_sampleRate(PyAudioStream *self, void *closure) {
  if (!PyAudioStream_IsOpen(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->context.stream);
  if (!stream_info) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  return PyFloat_FromDouble(stream_info->sampleRate);
}

static int antiset(PyAudioStream *self, PyObject *value, void *closure) {
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError,
                  "Fields read-only: cannot modify values");
  return -1;
}

static PyGetSetDef get_setters[] = {{"structVersion", (getter)get_structVersion,
                                     (setter)antiset, "struct version", NULL},

                                    {"inputLatency", (getter)get_inputLatency,
                                     (setter)antiset, "input latency", NULL},

                                    {"outputLatency", (getter)get_outputLatency,
                                     (setter)antiset, "output latency", NULL},

                                    {"sampleRate", (getter)get_sampleRate,
                                     (setter)antiset, "sample rate", NULL},

                                    {NULL}};

PyTypeObject PyAudioStreamType = {
    // clang-format off
    PyVarObject_HEAD_INIT(NULL, 0)
    // clang-format on
    .tp_name = "_portaudio.Stream",
    .tp_basicsize = sizeof(PyAudioStream),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = PyDoc_STR("PyAudio Stream"),
    .tp_getset = get_setters,
    .tp_new = PyType_GenericNew,
};

int PyAudioStream_IsOpen(PyAudioStream *stream) {
  return (stream) && (stream->context.stream != NULL);
}

PyAudioStream *PyAudioStream_Create(void) {
  PyAudioStream *stream =
      (PyAudioStream *)PyObject_New(PyAudioStream, &PyAudioStreamType);
  if (!stream) {
    return NULL;
  }
  memset(&(stream->context), 0, sizeof(struct StreamContext));
  return stream;
}

void PyAudioStream_Cleanup(PyAudioStream *stream) {
  // Note that this function may be called multiple times on the same stream.
  // For example, stream_lifecycle.c may call this when the user closes the
  // stream, and Python may call it again during deallocation, i.e., when the
  // stream Python object's reference count reaches 0.
  if (stream->context.stream != NULL) {
    // clang-format off
    Py_BEGIN_ALLOW_THREADS
    Pa_CloseStream(stream->context.stream);
    Py_END_ALLOW_THREADS
    // clang-format on
    stream->context.stream = NULL;
  }

  if (stream->context.callback != NULL) {
    Py_XDECREF(stream->context.callback);
    stream->context.callback = NULL;
  }

  // Just in case, zero out the entire struct.
  memset(&(stream->context), 0, sizeof(struct StreamContext));
}

PyObject *PyAudio_GetStreamTime(PyObject *self, PyObject *args) {
  double time;

  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  time = Pa_GetStreamTime(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (time == 0) {
    PyAudioStream_Cleanup(stream);
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError, "Internal Error"));
    return NULL;
  }

  return PyFloat_FromDouble(time);
}

PyObject *PyAudio_GetStreamCpuLoad(PyObject *self, PyObject *args) {
  double cpuload;

  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  cpuload = Pa_GetStreamCpuLoad(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  return PyFloat_FromDouble(cpuload);
}
