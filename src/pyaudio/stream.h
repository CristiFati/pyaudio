#ifndef STREAM_H_
#define STREAM_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

typedef struct {
  PyObject *callback;
  long main_thread_id;
  unsigned int frame_size;
} PyAudioCallbackContext;

typedef struct {
  // clang-format off
  PyObject_HEAD
  // clang-format on
  PaStream *stream;
  PaStreamParameters *inputParameters;
  PaStreamParameters *outputParameters;
  PaStreamInfo *streamInfo;
  PyAudioCallbackContext *callbackContext;
  int is_open;
} _pyAudio_Stream;

extern PyTypeObject _pyAudio_StreamType;

_pyAudio_Stream *_create_Stream_object(void);
int _is_open(_pyAudio_Stream *obj);
void _cleanup_Stream_object(_pyAudio_Stream *streamObject);

#endif  // STREAM_H_
