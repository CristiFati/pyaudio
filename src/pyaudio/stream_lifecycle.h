#ifndef STREAM_LIFECYCLE_H_
#define STREAM_LIFECYCLE_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

PyObject *PyAudio_OpenStream(PyObject *self, PyObject *args, PyObject *kwargs);
PyObject *PyAudio_CloseStream(PyObject *self, PyObject *args);
PyObject *PyAudio_StartStream(PyObject *self, PyObject *args);
PyObject *PyAudio_StopStream(PyObject *self, PyObject *args);
PyObject *PyAudio_AbortStream(PyObject *self, PyObject *args);
PyObject *PyAudio_IsStreamStopped(PyObject *self, PyObject *args);
PyObject *PyAudio_IsStreamActive(PyObject *self, PyObject *args);

#endif  // STREAM_LIFECYCLE_H_
