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
PyObject *pa_get_host_api_info(PyObject *self, PyObject *args);
PyObject *pa_get_host_api_count(PyObject *self, PyObject *args);
PyObject *pa_get_default_host_api(PyObject *self, PyObject *args);
PyObject *pa_host_api_type_id_to_host_api_index(PyObject *self, PyObject *args);
PyObject *pa_host_api_device_index_to_device_index(PyObject *self,
                                                   PyObject *args);

#endif  // PYAUDIO_HOST_API_H_
