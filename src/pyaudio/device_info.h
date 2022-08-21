// PyAudioDeviceInfo: Python object wrapper for PortAudio's PaDeviceInfo struct.

#ifndef PYAUDIO_DEVICE_INFO_H_
#define PYAUDIO_DEVICE_INFO_H_

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

typedef struct {
  // clang-format off
  PyObject_HEAD
  PaDeviceInfo *devInfo;
  // clang-format on
} PyAudioDeviceInfo;

PyAudioDeviceInfo *CreatePyAudioDeviceInfo(void);

extern PyTypeObject PyAudioDeviceInfoType;

#endif  // PYAUDIO_DEVICE_INFO_H_
