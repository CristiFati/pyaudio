// Python wrapper for PaMacCoreStreamInfo (macOS host-specific API).

#ifndef MAC_CORE_STREAM_INFO_H_
#define MAC_CORE_STREAM_INFO_H_

#ifdef MACOSX

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
      PaMacCoreStreamInfo *paMacCoreStreamInfo;
  int flags;
  SInt32 *channelMap;
  int channelMapSize;
} _pyAudio_MacOSX_hostApiSpecificStreamInfo;

// typedef _pyAudio_MacOSX_hostApiSpecificStreamInfo _pyAudio_Mac_HASSI;

extern PyTypeObject _pyAudio_MacOSX_hostApiSpecificStreamInfoType;

#endif  // MACOSX
#endif  // MAC_CORE_STREAM_INFO_H_
