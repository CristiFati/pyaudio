#ifndef PYAUDIO_DEVICE_API_H_
#define PYAUDIO_DEVICE_API_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

// Python object wrapper for PortAudio's PaDeviceInfo struct.
extern PyTypeObject PyAudioDeviceInfoType;

// Returns a PyAudioDeviceInfoType object
PyObject *PyAudio_GetDeviceInfo(PyObject *self, PyObject *args);
PyObject *PyAudio_GetDeviceCount(PyObject *self, PyObject *args);
PyObject *PyAudio_GetDefaultInputDevice(PyObject *self, PyObject *args);
PyObject *PyAudio_GetDefaultOutputDevice(PyObject *self, PyObject *args);

#endif  // PYAUDIO_DEVICE_API_H_
