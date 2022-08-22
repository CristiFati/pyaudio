#include "stream_lifecycle.h"

#include "mac_core_stream_info.h"
#include "stream.h"
#include "stream_io.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

#define DEFAULT_FRAMES_PER_BUFFER paFramesPerBufferUnspecified

PyObject *pa_open(PyObject *self, PyObject *args, PyObject *kwargs) {
  int rate, channels;
  int input, output, frames_per_buffer;
  int input_device_index = -1;
  int output_device_index = -1;
  PyObject *input_device_index_arg = NULL;
  PyObject *output_device_index_arg = NULL;
  PyObject *stream_callback = NULL;
  PaSampleFormat format;
  PaError err;
  PyObject *input_device_index_long;
  PyObject *output_device_index_long;
  PaStreamParameters *outputParameters = NULL;
  PaStreamParameters *inputParameters = NULL;
  PaStream *stream = NULL;
  PaStreamInfo *streamInfo = NULL;
  PyAudioCallbackContext *context = NULL;
  PyAudioStream *streamObject;

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

#ifdef MACOSX
  PyAudioMacCoreStreamInfo *inputHostSpecificStreamInfo = NULL;
  PyAudioMacCoreStreamInfo *outputHostSpecificStreamInfo = NULL;
#else
  /* mostly ignored...*/
  PyObject *inputHostSpecificStreamInfo = NULL;
  PyObject *outputHostSpecificStreamInfo = NULL;
#endif

  /* default to neither output nor input */
  input = 0;
  output = 0;
  frames_per_buffer = DEFAULT_FRAMES_PER_BUFFER;

  // clang-format off
  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
#ifdef MACOSX
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
#ifdef MACOSX
                                   &PyAudioMacCoreStreamInfoType,
#endif
                                   &inputHostSpecificStreamInfo,
#ifdef MACOSX
                                   &PyAudioMacCoreStreamInfoType,
#endif
                                   &outputHostSpecificStreamInfo,
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

  if (output) {
    outputParameters = (PaStreamParameters *)malloc(sizeof(PaStreamParameters));

    if (output_device_index < 0) {
      outputParameters->device = Pa_GetDefaultOutputDevice();
    } else {
      outputParameters->device = output_device_index;
    }

    /* final check -- ensure that there is a default device */
    if (outputParameters->device < 0 ||
        outputParameters->device >= Pa_GetDeviceCount()) {
      free(outputParameters);
      PyErr_SetObject(PyExc_IOError,
                      Py_BuildValue("(i,s)", paInvalidDevice,
                                    "Invalid output device "
                                    "(no default output device)"));
      return NULL;
    }

    outputParameters->channelCount = channels;
    outputParameters->sampleFormat = format;
    outputParameters->suggestedLatency =
        Pa_GetDeviceInfo(outputParameters->device)->defaultLowOutputLatency;
    outputParameters->hostApiSpecificStreamInfo = NULL;

#ifdef MACOSX
    if (outputHostSpecificStreamInfo) {
      outputParameters->hostApiSpecificStreamInfo =
          &outputHostSpecificStreamInfo->paMacCoreStreamInfo;
    }
#endif
  }

  if (input) {
    inputParameters = (PaStreamParameters *)malloc(sizeof(PaStreamParameters));

    if (input_device_index < 0) {
      inputParameters->device = Pa_GetDefaultInputDevice();
    } else {
      inputParameters->device = input_device_index;
    }

    /* final check -- ensure that there is a default device */
    if (inputParameters->device < 0) {
      free(inputParameters);
      PyErr_SetObject(PyExc_IOError,
                      Py_BuildValue("(i,s)", paInvalidDevice,
                                    "Invalid input device "
                                    "(no default output device)"));
      return NULL;
    }

    inputParameters->channelCount = channels;
    inputParameters->sampleFormat = format;
    inputParameters->suggestedLatency =
        Pa_GetDeviceInfo(inputParameters->device)->defaultLowInputLatency;
    inputParameters->hostApiSpecificStreamInfo = NULL;

#ifdef MACOSX
    if (inputHostSpecificStreamInfo) {
      inputParameters->hostApiSpecificStreamInfo =
          &inputHostSpecificStreamInfo->paMacCoreStreamInfo;
    }
#endif
  }

  if (stream_callback) {
    Py_INCREF(stream_callback);
    context = (PyAudioCallbackContext *)malloc(sizeof(PyAudioCallbackContext));
    context->callback = (PyObject *)stream_callback;
    context->main_thread_id = PyThreadState_Get()->thread_id;
    context->frame_size = Pa_GetSampleSize(format) * channels;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_OpenStream(&stream,
                      /* input/output parameters */
                      /* NULL values are ignored */
                      inputParameters, outputParameters,
                      /* samples per second */
                      rate,
                      /* frames in the buffer */
                      frames_per_buffer,
                      /* we won't output out of range samples
                         so don't bother clipping them */
                      paClipOff,
                      /* callback, if specified */
                      (stream_callback) ? (stream_callback_cfunc) : (NULL),
                      /* callback userData, if applicable */
                      context);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err != paNoError) {
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));
    return NULL;
  }

  streamInfo = (PaStreamInfo *)Pa_GetStreamInfo(stream);
  if (!streamInfo) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paInternalError,
                                  "Could not get stream information"));

    return NULL;
  }

  streamObject =
      (PyAudioStream *)PyObject_New(PyAudioStream, &PyAudioStreamType);
  streamObject->stream = stream;
  streamObject->inputParameters = inputParameters;
  streamObject->outputParameters = outputParameters;
  streamObject->is_open = 1;
  streamObject->streamInfo = streamInfo;
  streamObject->callbackContext = context;
  return (PyObject *)streamObject;
}

PyObject *pa_close(PyObject *self, PyObject *args) {
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  cleanup_stream(streamObject);

  Py_INCREF(Py_None);
  return Py_None;
}


/*************************************************************
 * Stream Start / Stop / Info
 *************************************************************/

PyObject *pa_start_stream(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  if (!is_stream_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_StartStream(streamObject->stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if ((err != paNoError) &&
      (err != paStreamIsNotStopped)) {
    cleanup_stream(streamObject);

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

PyObject *pa_stop_stream(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  if (!is_stream_open(streamObject)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_StopStream(streamObject->stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if ((err != paNoError) && (err != paStreamIsStopped)) {
    cleanup_stream(streamObject);

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

PyObject *pa_abort_stream(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  if (!is_stream_open(streamObject)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_AbortStream(streamObject->stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if ((err != paNoError) && (err != paStreamIsStopped)) {
    cleanup_stream(streamObject);

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

PyObject *pa_is_stream_stopped(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  if (!is_stream_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_IsStreamStopped(streamObject->stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err < 0) {
    cleanup_stream(streamObject);

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

PyObject *pa_is_stream_active(PyObject *self, PyObject *args) {
  int err;
  PyObject *stream_arg;
  PyAudioStream *streamObject;

  if (!PyArg_ParseTuple(args, "O!", &PyAudioStreamType, &stream_arg)) {
    return NULL;
  }

  streamObject = (PyAudioStream *)stream_arg;

  if (!is_stream_open(streamObject)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_IsStreamActive(streamObject->stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err < 0) {
    cleanup_stream(streamObject);

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
