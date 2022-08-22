#include "stream.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

int _is_open(_pyAudio_Stream *obj) { return (obj) && (obj->is_open); }

void _cleanup_Stream_object(_pyAudio_Stream *streamObject) {
  if (streamObject->stream != NULL) {
    // clang-format off
    Py_BEGIN_ALLOW_THREADS
    Pa_CloseStream(streamObject->stream);
    Py_END_ALLOW_THREADS
    // clang-format on
    streamObject->stream = NULL;
  }

  if (streamObject->streamInfo) streamObject->streamInfo = NULL;

  if (streamObject->inputParameters != NULL) {
    free(streamObject->inputParameters);
    streamObject->inputParameters = NULL;
  }

  if (streamObject->outputParameters != NULL) {
    free(streamObject->outputParameters);
    streamObject->outputParameters = NULL;
  }

  if (streamObject->callbackContext != NULL) {
    Py_XDECREF(streamObject->callbackContext->callback);
    free(streamObject->callbackContext);
    streamObject->callbackContext = NULL;
  }

  streamObject->is_open = 0;
}

static void _pyAudio_Stream_dealloc(_pyAudio_Stream *self) {
  _cleanup_Stream_object(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *_pyAudio_Stream_get_structVersion(_pyAudio_Stream *self,
                                                   void *closure) {
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  if ((!self->streamInfo)) {
    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", paBadStreamPtr,
                                                 "No StreamInfo available"));
    return NULL;
  }

  return PyLong_FromLong(self->streamInfo->structVersion);
}

static PyObject *_pyAudio_Stream_get_inputLatency(_pyAudio_Stream *self,
                                                  void *closure) {
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  if ((!self->streamInfo)) {
    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", paBadStreamPtr,
                                                 "No StreamInfo available"));
    return NULL;
  }

  return PyFloat_FromDouble(self->streamInfo->inputLatency);
}

static PyObject *_pyAudio_Stream_get_outputLatency(_pyAudio_Stream *self,
                                                   void *closure) {
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  if ((!self->streamInfo)) {
    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", paBadStreamPtr,
                                                 "No StreamInfo available"));
    return NULL;
  }

  return PyFloat_FromDouble(self->streamInfo->outputLatency);
}

static PyObject *_pyAudio_Stream_get_sampleRate(_pyAudio_Stream *self,
                                                void *closure) {
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  if ((!self->streamInfo)) {
    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", paBadStreamPtr,
                                                 "No StreamInfo available"));
    return NULL;
  }

  return PyFloat_FromDouble(self->streamInfo->sampleRate);
}

static int _pyAudio_Stream_antiset(_pyAudio_Stream *self, PyObject *value,
                                   void *closure) {
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError,
                  "Fields read-only: cannot modify values");
  return -1;
}

static PyGetSetDef _pyAudio_Stream_getseters[] = {
    {"structVersion", (getter)_pyAudio_Stream_get_structVersion,
     (setter)_pyAudio_Stream_antiset, "struct version", NULL},

    {"inputLatency", (getter)_pyAudio_Stream_get_inputLatency,
     (setter)_pyAudio_Stream_antiset, "input latency", NULL},

    {"outputLatency", (getter)_pyAudio_Stream_get_outputLatency,
     (setter)_pyAudio_Stream_antiset, "output latency", NULL},

    {"sampleRate", (getter)_pyAudio_Stream_get_sampleRate,
     (setter)_pyAudio_Stream_antiset, "sample rate", NULL},

    {NULL}};

PyTypeObject _pyAudio_StreamType = {
    // clang-format off
  PyVarObject_HEAD_INIT(NULL, 0)
    // clang-format on
    "_portaudio.Stream",                 /*tp_name*/
    sizeof(_pyAudio_Stream),             /*tp_basicsize*/
    0,                                   /*tp_itemsize*/
    (destructor)_pyAudio_Stream_dealloc, /*tp_dealloc*/
    0,                                   /*tp_print*/
    0,                                   /*tp_getattr*/
    0,                                   /*tp_setattr*/
    0,                                   /*tp_compare*/
    0,                                   /*tp_repr*/
    0,                                   /*tp_as_number*/
    0,                                   /*tp_as_sequence*/
    0,                                   /*tp_as_mapping*/
    0,                                   /*tp_hash */
    0,                                   /*tp_call*/
    0,                                   /*tp_str*/
    0,                                   /*tp_getattro*/
    0,                                   /*tp_setattro*/
    0,                                   /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                  /*tp_flags*/
    "Port Audio Stream",                 /* tp_doc */
    0,                                   /* tp_traverse */
    0,                                   /* tp_clear */
    0,                                   /* tp_richcompare */
    0,                                   /* tp_weaklistoffset */
    0,                                   /* tp_iter */
    0,                                   /* tp_iternext */
    0,                                   /* tp_methods */
    0,                                   /* tp_members */
    _pyAudio_Stream_getseters,           /* tp_getset */
    0,                                   /* tp_base */
    0,                                   /* tp_dict */
    0,                                   /* tp_descr_get */
    0,                                   /* tp_descr_set */
    0,                                   /* tp_dictoffset */
    0,                                   /* tp_init */
    0,                                   /* tp_alloc */
    0,                                   /* tp_new */
};

_pyAudio_Stream *_create_Stream_object(void) {
  _pyAudio_Stream *obj;

  /* don't allow subclassing */
  obj = (_pyAudio_Stream *)PyObject_New(_pyAudio_Stream, &_pyAudio_StreamType);
  return obj;
}
