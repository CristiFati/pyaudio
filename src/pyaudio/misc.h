#ifndef MISC_H_
#define MISC_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

PyObject *PyAudio_GetPortAudioVersion(PyObject *self, PyObject *args);
PyObject *PyAudio_GetPortAudioVersionText(PyObject *self, PyObject *args);
PyObject *PyAudio_GetSampleSize(PyObject *self, PyObject *args);
PyObject *PyAudio_IsFormatSupported(PyObject *self, PyObject *args,
                                    PyObject *kwargs);
#endif  // MISC_H_
