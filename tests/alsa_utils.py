"""Utilities to silence ALSA console output."""

# ALSA tends to generate noisy console output. The functions here suppress that
# output by patching the ALSA's error output routine. Heavily inspired by:
# https://stackoverflow.com/a/13453192

import ctypes
import sys


ERROR_HANDLER_FUNC = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_int,
                                      ctypes.c_char_p, ctypes.c_int,
                                      ctypes.c_char_p)

def py_error_handler(filename, line, function, err, fmt):
    pass


def disable_error_handler_output():
    if sys.platform != 'linux':
        return

    try:
        asound = ctypes.cdll.LoadLibrary('libasound.so')
        asound.snd_lib_error_set_handler(ERROR_HANDLER_FUNC(py_error_handler))
    except OSError:
        pass


def enable_error_handler_output():
    if sys.platform != 'linux':
        return

    try:
        asound = ctypes.cdll.LoadLibrary('libasound.so')
        asound.snd_lib_error_set_handler(None)
    except OSError:
        pass
