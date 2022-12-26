"""PyAudio: ensure deprecated API usage raises warnings."""

import os
import unittest
import sys

import pyaudio
import alsa_utils

# To skip tests requiring hardware, set this environment variable:
SKIP_HW_TESTS = 'PYAUDIO_SKIP_HW_TESTS' in os.environ

setUpModule = alsa_utils.disable_error_handler_output
tearDownModule = alsa_utils.disable_error_handler_output


class DeprecationTests(unittest.TestCase):

    @unittest.skipIf('darwin' not in sys.platform, 'macOS-only test.')
    def test_deprecated_mac_core_stream_info_getters(self):
        stream_info = pyaudio.PaMacCoreStreamInfo(
            flags=pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice,
            channel_map=(1, 0))
        with self.assertWarns(DeprecationWarning):
            self.assertEqual(stream_info.get_flags(), stream_info.flags)
        with self.assertWarns(DeprecationWarning):
            self.assertEqual(stream_info.get_channel_map(),
                             stream_info.channel_map)
        with self.assertWarns(DeprecationWarning):
            self.assertEqual(stream_info._get_host_api_stream_object(),
                             stream_info)

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_deprecated_stream(self):
        p = pyaudio.PyAudio()
        # Do not directly instantiate or reference pyaudio.Stream.
        # Use pyaudio.PyAudio.open instead, and if necessary, reference
        # pyaudio.PyAudio.Stream.
        with self.assertWarns(DeprecationWarning):
            stream = pyaudio.Stream(  # Don't do this!
                p,
                format=p.get_format_from_width(2),
                channels=2,
                rate=44100,
                output=True)
            stream.close()

        p.terminate()
