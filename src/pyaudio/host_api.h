// HostAPI related functions.

#ifndef PYAUDIO_HOST_API_H_
#define PYAUDIO_HOST_API_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

// Python object wrapper for PortAudio's PaHostApi struct.
extern PyTypeObject PyAudioHostApiInfoType;

// Returns a PyAudioHostApiInfoType object
PyObject *PyAudio_GetHostApiInfo(PyObject *self, PyObject *args);
PyObject *PyAudio_GetHostApiCount(PyObject *self, PyObject *args);
PyObject *PyAudio_GetDefaultHostApi(PyObject *self, PyObject *args);
PyObject *PyAudio_HostApiTypeIdToHostApiIndex(PyObject *self, PyObject *args);
PyObject *PyAudio_HostApiDeviceIndexToDeviceIndex(PyObject *self,
                                                  PyObject *args);

#endif  // PYAUDIO_HOST_API_H_
