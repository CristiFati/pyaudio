#include "stream.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

static void dealloc(PyAudioStream *self) {
  cleanup_stream(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *get_structVersion(PyAudioStream *self, void *closure) {
  if (!is_stream_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->stream);
  if (!stream_info) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  return PyLong_FromLong(stream_info->structVersion);
}

static PyObject *get_inputLatency(PyAudioStream *self, void *closure) {
  if (!is_stream_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->stream);
  if (!stream_info) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  return PyFloat_FromDouble(stream_info->inputLatency);
}

static PyObject *get_outputLatency(PyAudioStream *self, void *closure) {
  if (!is_stream_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->stream);
  if (!stream_info) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  return PyFloat_FromDouble(stream_info->outputLatency);
}

static PyObject *get_sampleRate(PyAudioStream *self, void *closure) {
  if (!is_stream_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  const PaStreamInfo *stream_info = Pa_GetStreamInfo(self->stream);
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

int is_stream_open(PyAudioStream *stream) {
  return (stream) && (stream->stream != NULL);
}

void cleanup_stream(PyAudioStream *stream) {
  if (stream->stream != NULL) {
    // clang-format off
    Py_BEGIN_ALLOW_THREADS
    Pa_CloseStream(stream->stream);
    Py_END_ALLOW_THREADS
    // clang-format on
    stream->stream = NULL;
  }

  if (stream->inputParameters != NULL) {
    free(stream->inputParameters);
    stream->inputParameters = NULL;
  }

  if (stream->outputParameters != NULL) {
    free(stream->outputParameters);
    stream->outputParameters = NULL;
  }

  if (stream->callbackContext != NULL) {
    Py_XDECREF(stream->callbackContext->callback);
    free(stream->callbackContext);
    stream->callbackContext = NULL;
  }
}

PyObject *pa_get_stream_time(PyObject *self, PyObject *args) {
  double time;
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  if (!is_stream_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  time = Pa_GetStreamTime(streamObject->stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (time == 0) {
    cleanup_stream(streamObject);
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError, "Internal Error"));
    return NULL;
  }

  return PyFloat_FromDouble(time);
}

PyObject *pa_get_stream_cpu_load(PyObject *self, PyObject *args) {
  double cpuload;
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  if (!is_stream_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  cpuload = Pa_GetStreamCpuLoad(streamObject->stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  return PyFloat_FromDouble(cpuload);
}
