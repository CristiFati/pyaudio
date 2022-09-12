#ifndef INIT_H_
#define INIT_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

PyObject *PyAudio_Initialize(PyObject *self, PyObject *args);
PyObject *PyAudio_Terminate(PyObject *self, PyObject *args);

#endif  // INIT_H
