#include "stream_lifecycle.h"

#include <stdio.h>

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

#include "mac_core_stream_info.h"
#include "stream.h"
#include "stream_io.h"

#define DEFAULT_FRAMES_PER_BUFFER paFramesPerBufferUnspecified

PyObject *PyAudio_OpenStream(PyObject *self, PyObject *args, PyObject *kwargs) {
  int rate, channels;
  int input_device_index = -1;
  int output_device_index = -1;
  PyObject *input_device_index_arg = NULL;
  PyObject *output_device_index_arg = NULL;
  PyObject *stream_callback = NULL;
  PaSampleFormat format;
  PaError err;
  PyObject *input_device_index_long;
  PyObject *output_device_index_long;

  static char *kwlist[] = {"rate",
                           "channels",
                           "format",
                           "input",
                           "output",
                           "input_device_index",
                           "output_device_index",
                           "frames_per_buffer",
                           "input_host_api_specific_stream_info",
                           "output_host_api_specific_stream_info",
                           "stream_callback",
                           NULL};

#ifdef MACOS
  PyAudioMacCoreStreamInfo *input_host_specific_stream_info = NULL;
  PyAudioMacCoreStreamInfo *output_host_specific_stream_info = NULL;
#else
  /* mostly ignored...*/
  PyObject *input_host_specific_stream_info = NULL;
  PyObject *output_host_specific_stream_info = NULL;
#endif

  /* default to neither output nor input */
  int input = 0;
  int output = 0;
  int frames_per_buffer = DEFAULT_FRAMES_PER_BUFFER;

  // clang-format off
  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
#ifdef MACOS
                                   "iik|iiOOiO!O!O",
#else
                                   "iik|iiOOiOOO",
#endif
                                   kwlist,
                                   &rate, &channels, &format,
                                   &input, &output,
                                   &input_device_index_arg,
                                   &output_device_index_arg,
                                   &frames_per_buffer,
#ifdef MACOS
                                   &PyAudioMacCoreStreamInfoType,
#endif
                                   &input_host_specific_stream_info,
#ifdef MACOS
                                   &PyAudioMacCoreStreamInfoType,
#endif
                                   &output_host_specific_stream_info,
                                   &stream_callback)) {

    return NULL;
  }
  // clang-format on

  if (stream_callback && (PyCallable_Check(stream_callback) == 0)) {
    PyErr_SetString(PyExc_TypeError, "stream_callback must be callable");
    return NULL;
  }

  if ((input_device_index_arg == NULL) || (input_device_index_arg == Py_None)) {
#ifdef VERBOSE
    printf("Using default input device\n");
#endif

    input_device_index = -1;
  } else {
    if (!PyNumber_Check(input_device_index_arg)) {
      PyErr_SetString(PyExc_ValueError,
                      "input_device_index must be integer (or None)");
      return NULL;
    }

    input_device_index_long = PyNumber_Long(input_device_index_arg);

    input_device_index = (int)PyLong_AsLong(input_device_index_long);
    Py_DECREF(input_device_index_long);

#ifdef VERBOSE
    printf("Using input device index number: %d\n", input_device_index);
#endif
  }

  if ((output_device_index_arg == NULL) ||
      (output_device_index_arg == Py_None)) {
#ifdef VERBOSE
    printf("Using default output device\n");
#endif

    output_device_index = -1;
  } else {
    if (!PyNumber_Check(output_device_index_arg)) {
      PyErr_SetString(PyExc_ValueError,
                      "output_device_index must be integer (or None)");
      return NULL;
    }

    output_device_index_long = PyNumber_Long(output_device_index_arg);
    output_device_index = (int)PyLong_AsLong(output_device_index_long);
    Py_DECREF(output_device_index_long);

#ifdef VERBOSE
    printf("Using output device index number: %d\n", output_device_index);
#endif
  }

  if (input == 0 && output == 0) {
    PyErr_SetString(PyExc_ValueError, "Must specify either input or output");
    return NULL;
  }

  if (channels < 1) {
    PyErr_SetString(PyExc_ValueError, "Invalid audio channels");
    return NULL;
  }

  PaStreamParameters output_parameters;
  if (output) {
    if (output_device_index < 0) {
      output_parameters.device = Pa_GetDefaultOutputDevice();
    } else {
      output_parameters.device = output_device_index;
    }

    /* final check -- ensure that there is a default device */
    if (output_parameters.device < 0 ||
        output_parameters.device >= Pa_GetDeviceCount()) {
      PyErr_SetObject(PyExc_IOError,
                      Py_BuildValue("(i,s)", paInvalidDevice,
                                    "Invalid output device "
                                    "(no default output device)"));
      return NULL;
    }

    output_parameters.channelCount = channels;
    output_parameters.sampleFormat = format;
    output_parameters.suggestedLatency =
        Pa_GetDeviceInfo(output_parameters.device)->defaultLowOutputLatency;
    output_parameters.hostApiSpecificStreamInfo = NULL;
#ifdef MACOS
    if (output_host_specific_stream_info) {
      output_parameters.hostApiSpecificStreamInfo =
          &output_host_specific_stream_info->stream_info;
    }
#endif
  }

  PaStreamParameters input_parameters;
  if (input) {
    if (input_device_index < 0) {
      input_parameters.device = Pa_GetDefaultInputDevice();
    } else {
      input_parameters.device = input_device_index;
    }

    /* final check -- ensure that there is a default device */
    if (input_parameters.device < 0) {
      PyErr_SetObject(PyExc_IOError,
                      Py_BuildValue("(i,s)", paInvalidDevice,
                                    "Invalid input device "
                                    "(no default output device)"));
      return NULL;
    }

    input_parameters.channelCount = channels;
    input_parameters.sampleFormat = format;
    input_parameters.suggestedLatency =
        Pa_GetDeviceInfo(input_parameters.device)->defaultLowInputLatency;
    input_parameters.hostApiSpecificStreamInfo = NULL;
#ifdef MACOS
    if (input_host_specific_stream_info) {
      input_parameters.hostApiSpecificStreamInfo =
          &input_host_specific_stream_info->stream_info;
    }
#endif
  }

  PyAudioStream *stream = PyAudioStream_Create();
  if (!stream) {
    PyErr_SetString(PyExc_MemoryError, "Cannot allocate stream object");
    return NULL;
  }

  PaStream *pa_stream = NULL;
  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_OpenStream(&pa_stream,
                      /* input/output parameters */
                      /* NULL values are ignored */
                      input ? &input_parameters : NULL,
                      output ? &output_parameters : NULL,
                      /* samples per second */
                      rate,
                      /* frames in the buffer */
                      frames_per_buffer,
                      /* we won't output out of range samples
                         so don't bother clipping them */
                      paClipOff,
                      /* callback, if specified */
                      stream_callback ? PyAudioStream_CallbackCFunc : NULL,
                      /* callback userData, if applicable */
                      stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err != paNoError) {
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif
    // Decrement reference, which automatically cleanups & deallocates stream.
    Py_DECREF(stream);

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  stream->context.stream = pa_stream;
  stream->context.frame_size = Pa_GetSampleSize(format) * channels;
  stream->context.main_thread_id = PyThreadState_Get()->thread_id;
  stream->context.callback = NULL;
  if (stream_callback) {
    Py_INCREF(stream_callback);
    stream->context.callback = stream_callback;
  }

  return (PyObject *)stream;
}

PyObject *PyAudio_CloseStream(PyObject *self, PyObject *args) {
  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  // Closes the PortAudio stream and cleans up.
  PyAudioStream_Cleanup(stream);

  Py_INCREF(Py_None);
  return Py_None;
}

/*************************************************************
 * Stream Start / Stop / Info
 *************************************************************/

PyObject *PyAudio_StartStream(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_StartStream(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if ((err != paNoError) && (err != paStreamIsNotStopped)) {
    PyAudioStream_Cleanup(stream);

#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *PyAudio_StopStream(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_StopStream(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if ((err != paNoError) && (err != paStreamIsStopped)) {
    PyAudioStream_Cleanup(stream);

#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *PyAudio_AbortStream(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_AbortStream(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if ((err != paNoError) && (err != paStreamIsStopped)) {
    PyAudioStream_Cleanup(stream);

#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

PyObject *PyAudio_IsStreamStopped(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_IsStreamStopped(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err < 0) {
    PyAudioStream_Cleanup(stream);

#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  if (err) {
    Py_INCREF(Py_True);
    return Py_True;
  }

  Py_INCREF(Py_False);
  return Py_False;
}

PyObject *PyAudio_IsStreamActive(PyObject *self, PyObject *args) {
  int is_active;
  PyObject *stream_arg;
  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  is_active = Pa_IsStreamActive(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (is_active < 0) {
    PaError err = is_active;
    PyAudioStream_Cleanup(stream);

#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  if (is_active) {
    Py_INCREF(Py_True);
    return Py_True;
  }

  Py_INCREF(Py_False);
  return Py_False;
}
