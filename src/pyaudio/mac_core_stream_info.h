// Python wrapper for PaMacCoreStreamInfo (macOS host-specific API).

#ifndef MAC_CORE_STREAM_INFO_H_
#define MAC_CORE_STREAM_INFO_H_

#ifdef MACOS

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"
#include "pa_mac_core.h"

typedef struct {
  // clang-format off
  PyObject_HEAD
  // clang-format on
  PaMacCoreStreamInfo stream_info;
  int flags;
  SInt32 *channel_map;
  int channel_map_size;
} PyAudioMacCoreStreamInfo;

extern PyTypeObject PyAudioMacCoreStreamInfoType;

#endif  // MACOS
#endif  // MAC_CORE_STREAM_INFO_H_
