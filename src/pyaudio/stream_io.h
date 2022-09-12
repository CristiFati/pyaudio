#ifndef STREAM_IO_H_
#define STREAM_IO_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

int PyAudioStream_CallbackCFunc(const void *input, void *output,
                                unsigned long frameCount,
                                const PaStreamCallbackTimeInfo *timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData);

PyObject *PyAudio_WriteStream(PyObject *self, PyObject *args);
PyObject *PyAudio_ReadStream(PyObject *self, PyObject *args);
PyObject *PyAudio_GetStreamWriteAvailable(PyObject *self, PyObject *args);
PyObject *PyAudio_GetStreamReadAvailable(PyObject *self, PyObject *args);

#endif  // STREAM_IO_H
