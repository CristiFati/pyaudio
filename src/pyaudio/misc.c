#include "misc.h"

#ifndef PY_SSIZE_T_CLEAN
#define PY_SSIZE_T_CLEAN
#endif
#include "Python.h"
#include "portaudio.h"

PyObject *PyAudio_GetPortAudioVersion(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyLong_FromLong(Pa_GetVersion());
}

PyObject *PyAudio_GetPortAudioVersionText(PyObject *self, PyObject *args) {
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyUnicode_FromString(Pa_GetVersionText());
}

PyObject *PyAudio_GetSampleSize(PyObject *self, PyObject *args) {
  PaSampleFormat format;
  int size_in_bytes;

  if (!PyArg_ParseTuple(args, "k", &format)) {
    return NULL;
  }

  size_in_bytes = Pa_GetSampleSize(format);

  if (size_in_bytes < 0) {
    PyErr_SetObject(
        PyExc_ValueError,
        Py_BuildValue("(s,i)", Pa_GetErrorText(size_in_bytes), size_in_bytes));
    return NULL;
  }

  return PyLong_FromLong(size_in_bytes);
}

PyObject *PyAudio_IsFormatSupported(PyObject *self, PyObject *args,
                                    PyObject *kwargs) {
  // clang-format off
  static char *kwlist[] = {
    "sample_rate",
    "input_device",
    "input_channels",
    "input_format",
    "output_device",
    "output_channels",
    "output_format",
    NULL
  };
  // clang-format on

  int input_device, input_channels;
  int output_device, output_channels;
  float sample_rate;
  PaStreamParameters inputParams;
  PaStreamParameters outputParams;
  PaSampleFormat input_format, output_format;
  PaError error;

  input_device = input_channels = output_device = output_channels = -1;

  input_format = output_format = -1;

  // clang-format off
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "f|iikiik", kwlist,
                                   &sample_rate,
                                   &input_device,
                                   &input_channels,
                                   &input_format,
                                   &output_device,
                                   &output_channels,
                                   &output_format)) {
    return NULL;
  }
  // clang-format on

  if (!(input_device < 0)) {
    inputParams.device = input_device;
    inputParams.channelCount = input_channels;
    inputParams.sampleFormat = input_format;
    inputParams.suggestedLatency = 0;
    inputParams.hostApiSpecificStreamInfo = NULL;
  }

  if (!(output_device < 0)) {
    outputParams.device = output_device;
    outputParams.channelCount = output_channels;
    outputParams.sampleFormat = output_format;
    outputParams.suggestedLatency = 0;
    outputParams.hostApiSpecificStreamInfo = NULL;
  }

  error = Pa_IsFormatSupported((input_device < 0) ? NULL : &inputParams,
                               (output_device < 0) ? NULL : &outputParams,
                               sample_rate);

  if (error == paFormatIsSupported) {
    Py_INCREF(Py_True);
    return Py_True;
  } else {
    PyErr_SetObject(PyExc_ValueError,
                    Py_BuildValue("(s,i)", Pa_GetErrorText(error), error));
    return NULL;
  }
}
