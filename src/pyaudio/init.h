#ifndef INIT_H_
#define INIT_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

PyObject *pa_initialize(PyObject *self, PyObject *args);
PyObject *pa_terminate(PyObject *self, PyObject *args);

#endif  // INIT_H
