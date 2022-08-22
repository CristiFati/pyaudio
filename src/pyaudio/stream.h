#ifndef STREAM_H_
#define STREAM_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

typedef struct {
  PyObject *callback;
  long main_thread_id;
  unsigned int frame_size;
} PyAudioCallbackContext;

typedef struct {
  // clang-format off
  PyObject_HEAD
  // clang-format on
  PaStream *stream;
  PaStreamParameters *inputParameters;
  PaStreamParameters *outputParameters;
  PyAudioCallbackContext *callbackContext;
} PyAudioStream;

extern PyTypeObject PyAudioStreamType;

// "Internal" utilities for other stream_*.c modules.

int is_stream_open(PyAudioStream *obj);
void cleanup_stream(PyAudioStream *streamObject);

// Public exported functions.

PyObject *pa_get_stream_time(PyObject *self, PyObject *args);
PyObject *pa_get_stream_cpu_load(PyObject *self, PyObject *args);

#endif  // STREAM_H_
