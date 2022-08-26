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
PyObject *GetHostApiInfo(PyObject *self, PyObject *args);
PyObject *GetHostApiCount(PyObject *self, PyObject *args);
PyObject *GetDefaultHostApi(PyObject *self, PyObject *args);
PyObject *HostApiTypeIdToHostApiIndex(PyObject *self, PyObject *args);
PyObject *HostApiDeviceIndexToDeviceIndex(PyObject *self, PyObject *args);

#endif  // PYAUDIO_HOST_API_H_
