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

static PyMethodDef paMethods[] = {
    /* version */
    {"get_version", pa_get_version, METH_VARARGS, "get version"},
    {"get_version_text", pa_get_version_text, METH_VARARGS, "get version text"},

    /* inits */
    {"initialize", pa_initialize, METH_VARARGS, "initialize portaudio"},
    {"terminate", pa_terminate, METH_VARARGS, "terminate portaudio"},

    /* host api */
    {"get_host_api_count", pa_get_host_api_count, METH_VARARGS,
     "get host API count"},

    {"get_default_host_api", pa_get_default_host_api, METH_VARARGS,
     "get default host API index"},

    {"host_api_type_id_to_host_api_index",
     pa_host_api_type_id_to_host_api_index, METH_VARARGS,
     "get default host API index"},

    {"host_api_device_index_to_device_index",
     pa_host_api_device_index_to_device_index, METH_VARARGS,
     "get default host API index"},

    {"get_host_api_info", pa_get_host_api_info, METH_VARARGS,
     "get host api information"},

    /* device api */
    {"get_device_count", pa_get_device_count, METH_VARARGS,
     "get host API count"},

    {"get_default_input_device", pa_get_default_input_device, METH_VARARGS,
     "get default input device index"},

    {"get_default_output_device", pa_get_default_output_device, METH_VARARGS,
     "get default output device index"},

    {"get_device_info", pa_get_device_info, METH_VARARGS,
     "get device information"},

    /* stream open/close */
    {"open", (PyCFunction)pa_open, METH_VARARGS | METH_KEYWORDS,
     "open port audio stream"},
    {"close", pa_close, METH_VARARGS, "close port audio stream"},
    {"get_sample_size", pa_get_sample_size, METH_VARARGS,
     "get sample size of a format in bytes"},
    {"is_format_supported", (PyCFunction)pa_is_format_supported,
     METH_VARARGS | METH_KEYWORDS,
     "returns whether specified format is supported"},

    /* stream start/stop */
    {"start_stream", pa_start_stream, METH_VARARGS, "starts port audio stream"},
    {"stop_stream", pa_stop_stream, METH_VARARGS, "stops  port audio stream"},
    {"abort_stream", pa_abort_stream, METH_VARARGS, "aborts port audio stream"},
    {"is_stream_stopped", pa_is_stream_stopped, METH_VARARGS,
     "returns whether stream is stopped"},
    {"is_stream_active", pa_is_stream_active, METH_VARARGS,
     "returns whether stream is active"},
    {"get_stream_time", pa_get_stream_time, METH_VARARGS,
     "returns stream time"},
    {"get_stream_cpu_load", pa_get_stream_cpu_load, METH_VARARGS,
     "returns stream CPU load -- always 0 for blocking mode"},

    /* stream read/write */
    {"write_stream", pa_write_stream, METH_VARARGS, "write to stream"},
    {"read_stream", pa_read_stream, METH_VARARGS, "read from stream"},

    {"get_stream_write_available", pa_get_stream_write_available, METH_VARARGS,
     "get buffer available for writing"},

    {"get_stream_read_available", pa_get_stream_read_available, METH_VARARGS,
     "get buffer available for reading"},

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
    paMethods,
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

#ifdef MACOSX
  if (PyType_Ready(&PyAudioMacCoreStreamInfoType) < 0) {
    return ERROR_INIT;
  }
#endif

#if PY_MAJOR_VERSION >= 3
  m = PyModule_Create(&moduledef);
#else
  m = Py_InitModule("_portaudio", paMethods);
#endif

  Py_INCREF(&PyAudioStreamType);
  Py_INCREF(&PyAudioDeviceInfoType);
  Py_INCREF(&PyAudioHostApiInfoType);

#ifdef MACOSX
  Py_INCREF(&PyAudioMacCoreStreamInfoType);
  PyModule_AddObject(
      m, "paMacCoreStreamInfo",
      (PyObject *)&PyAudioMacCoreStreamInfoType);
#endif

  /* Add PortAudio constants */

  /* host apis */
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

  /* formats */
  PyModule_AddIntConstant(m, "paFloat32", paFloat32);
  PyModule_AddIntConstant(m, "paInt32", paInt32);
  PyModule_AddIntConstant(m, "paInt24", paInt24);
  PyModule_AddIntConstant(m, "paInt16", paInt16);
  PyModule_AddIntConstant(m, "paInt8", paInt8);
  PyModule_AddIntConstant(m, "paUInt8", paUInt8);
  PyModule_AddIntConstant(m, "paCustomFormat", paCustomFormat);

  /* error codes */
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

  /* callback constants */
  PyModule_AddIntConstant(m, "paContinue", paContinue);
  PyModule_AddIntConstant(m, "paComplete", paComplete);
  PyModule_AddIntConstant(m, "paAbort", paAbort);

  /* callback status flags */
  PyModule_AddIntConstant(m, "paInputUnderflow", paInputUnderflow);
  PyModule_AddIntConstant(m, "paInputOverflow", paInputOverflow);
  PyModule_AddIntConstant(m, "paOutputUnderflow", paOutputUnderflow);
  PyModule_AddIntConstant(m, "paOutputOverflow", paOutputOverflow);
  PyModule_AddIntConstant(m, "paPrimingOutput", paPrimingOutput);

  /* misc */
  PyModule_AddIntConstant(m, "paFramesPerBufferUnspecified",
                          paFramesPerBufferUnspecified);

#ifdef MACOSX
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
