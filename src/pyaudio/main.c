/**
 * PyAudio: Python Bindings for PortAudio.
 *
 * Copyright (c) 2006 Hubert Pham
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

#include "device_api.h"
#include "host_api.h"
#include "init.h"
#include "mac_core_stream_info.h"
#include "misc.h"
#include "stream.h"
#include "stream_io.h"
#include "stream_lifecycle.h"

static PyMethodDef exported_functions[] = {
    // init.h
    {"initialize", PyAudio_Initialize, METH_VARARGS, "Initializes PortAudio"},

    {"terminate", PyAudio_Terminate, METH_VARARGS, "Terminates PortAudio"},

    // misc.h
    {"get_sample_size", PyAudio_GetSampleSize, METH_VARARGS,
     "Returns sample size of a format in bytes"},

    {"is_format_supported", (PyCFunction)PyAudio_IsFormatSupported,
     METH_VARARGS | METH_KEYWORDS, "Returns whether format is supported"},

    {"get_version", PyAudio_GetPortAudioVersion, METH_VARARGS,
     "PortAudio version"},

    {"get_version_text", PyAudio_GetPortAudioVersionText, METH_VARARGS,
     "PortAudio version text"},

    // host_api.h
    {"get_host_api_count", PyAudio_GetHostApiCount, METH_VARARGS,
     "Returns the number of Host APIs"},

    {"get_default_host_api", PyAudio_GetDefaultHostApi, METH_VARARGS,
     "Returns the default Host API index"},

    {"host_api_type_id_to_host_api_index", PyAudio_HostApiTypeIdToHostApiIndex,
     METH_VARARGS,
     "Returns the Host API index for given a PortAudio Host API Type ID"},

    {"host_api_device_index_to_device_index",
     PyAudio_HostApiDeviceIndexToDeviceIndex, METH_VARARGS,
     "Returns a Host API-specific device index to PortAudio device index"},

    {"get_host_api_info", PyAudio_GetHostApiInfo, METH_VARARGS,
     "Returns an object with information about the Host API"},

    // device_api.h
    {"get_device_count", PyAudio_GetDeviceCount, METH_VARARGS,
     "Returns the number of available devices"},

    {"get_default_input_device", PyAudio_GetDefaultInputDevice, METH_VARARGS,
     "Returns the default input device index"},

    {"get_default_output_device", PyAudio_GetDefaultOutputDevice, METH_VARARGS,
     "Returns the default output device index"},

    {"get_device_info", PyAudio_GetDeviceInfo, METH_VARARGS,
     "Returns an object with device properties"},

    // stream.h
    {"get_stream_time", PyAudio_GetStreamTime, METH_VARARGS,
     "Returns the number of seconds for the stream. See PortAudio docs for "
     "details."},

    {"get_stream_cpu_load", PyAudio_GetStreamCpuLoad, METH_VARARGS,
     "Returns the stream's CPU load (always 0 for blocking mode)"},

    // stream_lifecycle.h (and stream.h)
    {"open", (PyCFunction)PyAudio_OpenStream, METH_VARARGS | METH_KEYWORDS,
     "Opens a PortAudio stream"},

    {"close", PyAudio_CloseStream, METH_VARARGS, "Closes a PortAudio stream"},

    {"start_stream", PyAudio_StartStream, METH_VARARGS, "Starts the stream"},

    {"stop_stream", PyAudio_StopStream, METH_VARARGS,
     "Stops (pauses) the stream"},

    {"abort_stream", PyAudio_AbortStream, METH_VARARGS, "Aborts the stream"},

    {"is_stream_stopped", PyAudio_IsStreamStopped, METH_VARARGS,
     "Returns whether the stream is stopped"},

    {"is_stream_active", PyAudio_IsStreamActive, METH_VARARGS,
     "Returns whether the stream is active"},

    // stream_io.h (and stream.h)
    {"write_stream", PyAudio_WriteStream, METH_VARARGS,
     "Write samples to stream"},

    {"read_stream", PyAudio_ReadStream, METH_VARARGS,
     "Read samples from stream"},

    {"get_stream_write_available", PyAudio_GetStreamWriteAvailable,
     METH_VARARGS,
     "Returns the number of frames that can be written without waiting"},

    {"get_stream_read_available", PyAudio_GetStreamReadAvailable, METH_VARARGS,
     "Returns the number of frames that can be read without waiting"},

    {NULL, NULL, 0, NULL}};

#if PY_MAJOR_VERSION >= 3
#define ERROR_INIT NULL
#else
#define ERROR_INIT /**/
#endif

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {  //
    PyModuleDef_HEAD_INIT,
    "_portaudio",
    NULL,
    -1,
    exported_functions,
    NULL,
    NULL,
    NULL,
    NULL};
#endif

PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit__portaudio(void)
#else
init_portaudio(void)
#endif
{
  PyObject *m;

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION <= 6
  // Deprecated since Python 3.7; now called by Py_Initialize().
  PyEval_InitThreads();
#endif

  if (PyType_Ready(&PyAudioStreamType) < 0) {
    return ERROR_INIT;
  }

  if (PyType_Ready(&PyAudioDeviceInfoType) < 0) {
    return ERROR_INIT;
  }

  if (PyType_Ready(&PyAudioHostApiInfoType) < 0) {
    return ERROR_INIT;
  }

#ifdef MACOS
  if (PyType_Ready(&PyAudioMacCoreStreamInfoType) < 0) {
    return ERROR_INIT;
  }
#endif

#if PY_MAJOR_VERSION >= 3
  m = PyModule_Create(&moduledef);
#else
  m = Py_InitModule("_portaudio", exported_functions);
#endif

  Py_INCREF(&PyAudioStreamType);
  Py_INCREF(&PyAudioDeviceInfoType);
  Py_INCREF(&PyAudioHostApiInfoType);
#ifdef MACOS
  Py_INCREF(&PyAudioMacCoreStreamInfoType);
  PyModule_AddObject(m, "paMacCoreStreamInfo",
                     (PyObject *)&PyAudioMacCoreStreamInfoType);
#endif

  // Add PortAudio constants

  // Host APIs
  PyModule_AddIntConstant(m, "paInDevelopment", paInDevelopment);
  PyModule_AddIntConstant(m, "paDirectSound", paDirectSound);
  PyModule_AddIntConstant(m, "paMME", paMME);
  PyModule_AddIntConstant(m, "paASIO", paASIO);
  PyModule_AddIntConstant(m, "paSoundManager", paSoundManager);
  PyModule_AddIntConstant(m, "paCoreAudio", paCoreAudio);
  PyModule_AddIntConstant(m, "paOSS", paOSS);
  PyModule_AddIntConstant(m, "paALSA", paALSA);
  PyModule_AddIntConstant(m, "paAL", paAL);
  PyModule_AddIntConstant(m, "paBeOS", paBeOS);
  PyModule_AddIntConstant(m, "paWDMKS", paWDMKS);
  PyModule_AddIntConstant(m, "paJACK", paJACK);
  PyModule_AddIntConstant(m, "paWASAPI", paWASAPI);
  PyModule_AddIntConstant(m, "paNoDevice", paNoDevice);

  // Formats
  PyModule_AddIntConstant(m, "paFloat32", paFloat32);
  PyModule_AddIntConstant(m, "paInt32", paInt32);
  PyModule_AddIntConstant(m, "paInt24", paInt24);
  PyModule_AddIntConstant(m, "paInt16", paInt16);
  PyModule_AddIntConstant(m, "paInt8", paInt8);
  PyModule_AddIntConstant(m, "paUInt8", paUInt8);
  PyModule_AddIntConstant(m, "paCustomFormat", paCustomFormat);

  // Error codes
  PyModule_AddIntConstant(m, "paNoError", paNoError);
  PyModule_AddIntConstant(m, "paNotInitialized", paNotInitialized);
  PyModule_AddIntConstant(m, "paUnanticipatedHostError",
                          paUnanticipatedHostError);
  PyModule_AddIntConstant(m, "paInvalidChannelCount", paInvalidChannelCount);
  PyModule_AddIntConstant(m, "paInvalidSampleRate", paInvalidSampleRate);
  PyModule_AddIntConstant(m, "paInvalidDevice", paInvalidDevice);
  PyModule_AddIntConstant(m, "paInvalidFlag", paInvalidFlag);
  PyModule_AddIntConstant(m, "paSampleFormatNotSupported",
                          paSampleFormatNotSupported);
  PyModule_AddIntConstant(m, "paBadIODeviceCombination",
                          paBadIODeviceCombination);
  PyModule_AddIntConstant(m, "paInsufficientMemory", paInsufficientMemory);
  PyModule_AddIntConstant(m, "paBufferTooBig", paBufferTooBig);
  PyModule_AddIntConstant(m, "paBufferTooSmall", paBufferTooSmall);
  PyModule_AddIntConstant(m, "paNullCallback", paNullCallback);
  PyModule_AddIntConstant(m, "paBadStreamPtr", paBadStreamPtr);
  PyModule_AddIntConstant(m, "paTimedOut", paTimedOut);
  PyModule_AddIntConstant(m, "paInternalError", paInternalError);
  PyModule_AddIntConstant(m, "paDeviceUnavailable", paDeviceUnavailable);
  PyModule_AddIntConstant(m, "paIncompatibleHostApiSpecificStreamInfo",
                          paIncompatibleHostApiSpecificStreamInfo);
  PyModule_AddIntConstant(m, "paStreamIsStopped", paStreamIsStopped);
  PyModule_AddIntConstant(m, "paStreamIsNotStopped", paStreamIsNotStopped);
  PyModule_AddIntConstant(m, "paInputOverflowed", paInputOverflowed);
  PyModule_AddIntConstant(m, "paOutputUnderflowed", paOutputUnderflowed);
  PyModule_AddIntConstant(m, "paHostApiNotFound", paHostApiNotFound);
  PyModule_AddIntConstant(m, "paInvalidHostApi", paInvalidHostApi);
  PyModule_AddIntConstant(m, "paCanNotReadFromACallbackStream",
                          paCanNotReadFromACallbackStream);
  PyModule_AddIntConstant(m, "paCanNotWriteToACallbackStream",
                          paCanNotWriteToACallbackStream);
  PyModule_AddIntConstant(m, "paCanNotReadFromAnOutputOnlyStream",
                          paCanNotReadFromAnOutputOnlyStream);
  PyModule_AddIntConstant(m, "paCanNotWriteToAnInputOnlyStream",
                          paCanNotWriteToAnInputOnlyStream);
  PyModule_AddIntConstant(m, "paIncompatibleStreamHostApi",
                          paIncompatibleStreamHostApi);

  // Callback constants
  PyModule_AddIntConstant(m, "paContinue", paContinue);
  PyModule_AddIntConstant(m, "paComplete", paComplete);
  PyModule_AddIntConstant(m, "paAbort", paAbort);

  // Callback status flags
  PyModule_AddIntConstant(m, "paInputUnderflow", paInputUnderflow);
  PyModule_AddIntConstant(m, "paInputOverflow", paInputOverflow);
  PyModule_AddIntConstant(m, "paOutputUnderflow", paOutputUnderflow);
  PyModule_AddIntConstant(m, "paOutputOverflow", paOutputOverflow);
  PyModule_AddIntConstant(m, "paPrimingOutput", paPrimingOutput);

  // Misc
  PyModule_AddIntConstant(m, "paFramesPerBufferUnspecified",
                          paFramesPerBufferUnspecified);

#ifdef MACOS
  PyModule_AddIntConstant(m, "paMacCoreChangeDeviceParameters",
                          paMacCoreChangeDeviceParameters);
  PyModule_AddIntConstant(m, "paMacCoreFailIfConversionRequired",
                          paMacCoreFailIfConversionRequired);
  PyModule_AddIntConstant(m, "paMacCoreConversionQualityMin",
                          paMacCoreConversionQualityMin);
  PyModule_AddIntConstant(m, "paMacCoreConversionQualityMedium",
                          paMacCoreConversionQualityMedium);
  PyModule_AddIntConstant(m, "paMacCoreConversionQualityLow",
                          paMacCoreConversionQualityLow);
  PyModule_AddIntConstant(m, "paMacCoreConversionQualityHigh",
                          paMacCoreConversionQualityHigh);
  PyModule_AddIntConstant(m, "paMacCoreConversionQualityMax",
                          paMacCoreConversionQualityMax);
  PyModule_AddIntConstant(m, "paMacCorePlayNice", paMacCorePlayNice);
  PyModule_AddIntConstant(m, "paMacCorePro", paMacCorePro);
  PyModule_AddIntConstant(m, "paMacCoreMinimizeCPUButPlayNice",
                          paMacCoreMinimizeCPUButPlayNice);
  PyModule_AddIntConstant(m, "paMacCoreMinimizeCPU", paMacCoreMinimizeCPU);
#endif

#if PY_MAJOR_VERSION >= 3
  return m;
#endif
}
