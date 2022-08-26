#ifndef STREAM_LIFECYCLE_H_
#define STREAM_LIFECYCLE_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

PyObject *OpenStream(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *CloseStream(PyObject *self, PyObject *args);
PyObject *StartStream(PyObject *self, PyObject *args);
PyObject *StopStream(PyObject *self, PyObject *args);
PyObject *AbortStream(PyObject *self, PyObject *args);
PyObject *IsStreamStopped(PyObject *self, PyObject *args);
PyObject *IsStreamActive(PyObject *self, PyObject *args);

#endif  // STREAM_LIFECYCLE_H_
