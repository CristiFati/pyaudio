#ifndef PYAUDIO_DEVICE_API_H_
#define PYAUDIO_DEVICE_API_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"

// Python object wrapper for PortAudio's PaDeviceInfo struct.
extern PyTypeObject PyAudioDeviceInfoType;

// Returns a PyAudioDeviceInfoType object
PyObject *pa_get_device_info(PyObject *self, PyObject *args);
PyObject *pa_get_device_count(PyObject *self, PyObject *args);
PyObject *pa_get_default_input_device(PyObject *self, PyObject *args);
PyObject *pa_get_default_output_device(PyObject *self, PyObject *args);

#endif  // PYAUDIO_DEVICE_API_H_
