#include "stream_io.h"

#include <assert.h>
#include <stdio.h>

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

#include "stream.h"

int PyAudioStream_CallbackCFunc(const void *input, void *output,
                                unsigned long frame_count,
                                const PaStreamCallbackTimeInfo *time_info,
                                PaStreamCallbackFlags status_flags,
                                void *user_data) {
  PyGILState_STATE _state = PyGILState_Ensure();

#ifdef VERBOSE
  if (status_flags != 0) {
    printf("Status flag set: ");
    if (status_flags & paInputUnderflow) {
      printf("input underflow!\n");
    }
    if (status_flags & paInputOverflow) {
      printf("input overflow!\n");
    }
    if (status_flags & paOutputUnderflow) {
      printf("output underflow!\n");
    }
    if (status_flags & paOutputUnderflow) {
      printf("output overflow!\n");
    }
    if (status_flags & paPrimingOutput) {
      printf("priming output!\n");
    }
  }
#endif

  int return_val = paAbort;
  PyAudioStream *stream = (PyAudioStream *)user_data;
  PyObject *py_callback = stream->context.callback;
  unsigned int bytes_per_frame = stream->context.frame_size;
  long main_thread_id = stream->context.main_thread_id;

  // Prepare arguments for calling the python callback:
  PyObject *py_frame_count = PyLong_FromUnsignedLong(frame_count);
  // clang-format off
  PyObject *py_time_info = Py_BuildValue("{s:d,s:d,s:d}",
                                         "input_buffer_adc_time",
                                         time_info->inputBufferAdcTime,
                                         "current_time",
                                         time_info->currentTime,
                                         "output_buffer_dac_time",
                                         time_info->outputBufferDacTime);
  // clang-format on
  PyObject *py_status_flags = PyLong_FromUnsignedLong(status_flags);
  PyObject *py_input_samples;
  if (input != NULL) {
    py_input_samples =
        PyBytes_FromStringAndSize(input, bytes_per_frame * frame_count);
  } else {
    // Output stream, so provide None to the callback.
    Py_INCREF(Py_None);
    py_input_samples = Py_None;
  }

  PyObject *callback_result = PyObject_CallFunctionObjArgs(
      py_callback, py_input_samples, py_frame_count, py_time_info,
      py_status_flags, NULL);
  if (callback_result == NULL) {
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error message: Could not call callback function\n");
#endif
    PyObject *err = PyErr_Occurred();
    if (err) {
      PyThreadState_SetAsyncExc(main_thread_id, err);
      // Print out a stack trace to help debugging.
      // TODO: make VERBOSE a runtime flag so users can control
      // the amount of logging.
      PyErr_Print();
    }
    goto end;
  }

  // Parse the callback's response, which should be the samples to playback (if
  // output stream; ignored otherwise) and the desired next stream state
  // (paContinue, pAbort, or paComplete):
  const char *samples_for_output;
  Py_ssize_t output_len;
  // clang-format off
  if (!PyArg_ParseTuple(callback_result,
                        "z#i",
                        &samples_for_output,
                        &output_len,
                        &return_val)) {
// clang-format on
#ifdef VERBOSE
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error message: Could not parse callback return value\n");
#endif
    PyObject *err = PyErr_Occurred();
    if (err) {
      PyThreadState_SetAsyncExc(main_thread_id, err);
      // Print out a stack trace to help debugging.
      // TODO: make VERBOSE a runtime flag so users can control
      // the amount of logging.
      PyErr_Print();
    }

    Py_XDECREF(callback_result);
    return_val = paAbort;  // Quit the callback loop
    goto end;
  }

  if ((return_val != paComplete) && (return_val != paAbort) &&
      (return_val != paContinue)) {
    PyErr_SetString(PyExc_ValueError,
                    "Invalid PaStreamCallbackResult from callback");
    PyThreadState_SetAsyncExc(main_thread_id, PyErr_Occurred());
    PyErr_Print();

    Py_XDECREF(callback_result);
    return_val = paAbort;  // Quit the callback loop
    goto end;
  }

  // Copy bytes for playback only if this is an output stream:
  if (output) {
    char *output_data = (char *)output;
    size_t pa_max_num_bytes = bytes_per_frame * frame_count;
    // Though PyArg_ParseTuple returns the size of samples_for_output in
    // output_len, a signed Py_ssize_t, that value should never be negative.
    assert(output_len >= 0);
    // Only copy min(output_len, pa_max_num_bytes) bytes.
    size_t bytes_to_copy = (size_t)output_len < pa_max_num_bytes
                               ? (size_t)output_len
                               : pa_max_num_bytes;
    if (samples_for_output != NULL && bytes_to_copy > 0) {
      memcpy(output_data, samples_for_output, bytes_to_copy);
    }
    // If callback returned too few frames, pad out the rest of the buffer with
    // 0s and assume the stream is done (paComplete).
    if (bytes_to_copy < pa_max_num_bytes) {
      memset(output_data + bytes_to_copy, 0, pa_max_num_bytes - bytes_to_copy);
      return_val = paComplete;
    }
  }
  Py_DECREF(callback_result);

end:
  // Decrement py_input_samples at the end, after the memcpy above, in case the
  // user returns py_input_samples (from the callback) for playback.
  Py_XDECREF(py_input_samples);
  Py_XDECREF(py_frame_count);
  Py_XDECREF(py_time_info);
  Py_XDECREF(py_status_flags);

  PyGILState_Release(_state);
  return return_val;
}

/*************************************************************
 * Stream Read/Write
 *************************************************************/

PyObject *PyAudio_WriteStream(PyObject *self, PyObject *args) {
  const char *data;
  Py_ssize_t total_size;
  int total_frames;
  int err;
  int should_throw_exception = 0;

  PyObject *stream_arg;
  // clang-format off
  if (!PyArg_ParseTuple(args, "O!s#i|i",
                        &PyAudioStreamType,
                        &stream_arg,
                        &data,
                        &total_size,
                        &total_frames,
                        &should_throw_exception)) {
    return NULL;
  }
  // clang-format on

  if (total_frames < 0) {
    PyErr_SetString(PyExc_ValueError, "Invalid number of frames");
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
  err = Pa_WriteStream(stream->context.stream, data, total_frames);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err != paNoError) {
    if (err == paOutputUnderflowed) {
      if (should_throw_exception) {
        goto error;
      }
    } else
      goto error;
  }

  Py_INCREF(Py_None);
  return Py_None;

error:
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

PyObject *PyAudio_ReadStream(PyObject *self, PyObject *args) {
  int err;
  int total_frames;
  int should_raise_exception = 0;

  PyObject *stream_arg;
  // clang-format off
  if (!PyArg_ParseTuple(args, "O!i|i",
                        &PyAudioStreamType,
                        &stream_arg,
                        &total_frames,
                        &should_raise_exception)) {
    return NULL;
  }
  // clang-format on

  if (total_frames < 0) {
    PyErr_SetString(PyExc_ValueError, "Invalid number of frames");
    return NULL;
  }

  PyAudioStream *stream = (PyAudioStream *)stream_arg;
  if (!PyAudioStream_IsOpen(stream)) {
    PyErr_SetObject(PyExc_IOError,
                    Py_BuildValue("(i,s)", paBadStreamPtr, "Stream closed"));
    return NULL;
  }

  int num_bytes = total_frames * stream->context.frame_size;
#ifdef VERBOSE
  fprintf(stderr, "Allocating %d bytes\n", num_bytes);
#endif
  PyObject *rv = PyBytes_FromStringAndSize(NULL, num_bytes);
  short *sample_block = (short *)PyBytes_AsString(rv);

  if (sample_block == NULL) {
    PyErr_SetObject(PyExc_IOError, Py_BuildValue("(i,s)", paInsufficientMemory,
                                                 "Out of memory"));
    return NULL;
  }

  // clang-format off
  Py_BEGIN_ALLOW_THREADS
  err = Pa_ReadStream(stream->context.stream, sample_block, total_frames);
  Py_END_ALLOW_THREADS
  // clang-format on

  if (err != paNoError) {
    if (err == paInputOverflowed) {
      if (should_raise_exception) {
        goto error;
      }
    } else {
      goto error;
    }
  }

  return rv;

error:
  PyAudioStream_Cleanup(stream);
  Py_XDECREF(rv);
  PyErr_SetObject(PyExc_IOError,
                  Py_BuildValue("(i,s)", err, Pa_GetErrorText(err)));

#ifdef VERBOSE
  fprintf(stderr, "An error occured while using the portaudio stream\n");
  fprintf(stderr, "Error number: %d\n", err);
  fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
#endif

  return NULL;
}

PyObject *PyAudio_GetStreamWriteAvailable(PyObject *self, PyObject *args) {
  signed long frames;
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
  frames = Pa_GetStreamWriteAvailable(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  return PyLong_FromLong(frames);
}

PyObject *PyAudio_GetStreamReadAvailable(PyObject *self, PyObject *args) {
  signed long frames;
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
  frames = Pa_GetStreamReadAvailable(stream->context.stream);
  Py_END_ALLOW_THREADS
  // clang-format on

  return PyLong_FromLong(frames);
}
