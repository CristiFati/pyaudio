#ifndef STREAM_H_
#define STREAM_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

typedef struct {
  // clang-format off
  PyObject_HEAD
  // clang-format on
  struct StreamContext {
    // PortAudio stream object. NULL when the stream is closed.
    PaStream *stream;
    // User audio callback routine, for when using callback mode.
    // NULL otherwise.
    PyObject *callback;
    // Frame size, in bytes, for input and output. Equal to
    // num channels x bytes per sample.
    unsigned int frame_size;
    // Main thread ID.
    long main_thread_id;
  } context;
} PyAudioStream;

extern PyTypeObject PyAudioStreamType;

// "Internal" utilities for other stream_*.c modules.

// Creates a PyAudioStream and zeros out the fields. Returns NULL if memory
// allocation fails.
PyAudioStream *create_stream(void);
// Closes the PortAudio stream (if open) and garbage collects the fields within
// a PyAudioStream. May be called multiple times on the same stream.
void cleanup_stream(PyAudioStream *stream);
// Returns whether the stream is open.
int is_stream_open(PyAudioStream *stream);

// Public exported functions.

PyObject *GetStreamTime(PyObject *self, PyObject *args);
PyObject *GetStreamCpuLoad(PyObject *self, PyObject *args);

#endif  // STREAM_H_
