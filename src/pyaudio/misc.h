#ifndef MISC_H_
#define MISC_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

PyObject *pa_get_version(PyObject *self, PyObject *args);
PyObject *pa_get_version_text(PyObject *self, PyObject *args);
PyObject *pa_get_sample_size(PyObject *self, PyObject *args);
PyObject *pa_is_format_supported(PyObject *self, PyObject *args,
                                 PyObject *kwargs);
#endif  // MISC_H_
