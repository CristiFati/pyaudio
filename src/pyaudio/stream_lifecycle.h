#ifndef STREAM_LIFECYCLE_H_
#define STREAM_LIFECYCLE_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

PyObject *pa_open(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *pa_close(PyObject *self, PyObject *args);
PyObject *pa_start_stream(PyObject *self, PyObject *args);
PyObject *pa_stop_stream(PyObject *self, PyObject *args);
PyObject *pa_abort_stream(PyObject *self, PyObject *args);
PyObject *pa_is_stream_stopped(PyObject *self, PyObject *args);
PyObject *pa_is_stream_active(PyObject *self, PyObject *args);

#endif  // STREAM_LIFECYCLE_H_
