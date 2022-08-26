#ifndef MISC_H_
#define MISC_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

PyObject *GetVersion(PyObject *self, PyObject *args);
PyObject *GetVersionText(PyObject *self, PyObject *args);
PyObject *GetSampleSize(PyObject *self, PyObject *args);
PyObject *IsFormatSupported(PyObject *self, PyObject *args, PyObject *kwargs);
#endif  // MISC_H_
