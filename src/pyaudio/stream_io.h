#ifndef STREAM_IO_H_
#define STREAM_IO_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

int stream_callback_cfunc(const void *input, void *output,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags, void *userData);

PyObject *pa_write_stream(PyObject *self, PyObject *args);
PyObject *pa_read_stream(PyObject *self, PyObject *args);
PyObject *pa_get_stream_write_available(PyObject *self, PyObject *args);
PyObject *pa_get_stream_read_available(PyObject *self, PyObject *args);

#endif  // STREAM_IO_H
