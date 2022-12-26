#include "init.h"

#include <stdio.h>

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

PyObject *PyAudio_Initialize(PyObject *self, PyObject *args) {
  int err;

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_Initialize();
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err != paNoError) {
    // clang-format off
    Py_BEGIN_ALLOW_THREADS
    Pa_Terminate();
    Py_END_ALLOW_THREADS
    // clang-format on

#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *PyAudio_Terminate(PyObject *self, PyObject *args) {
  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  Pa_Terminate();
  Py_END_ALLOW_THREADS
  // clang-format on

  Py_INCREF(Py_None);
  return Py_None;
}
