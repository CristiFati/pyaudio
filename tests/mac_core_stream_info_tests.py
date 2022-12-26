"""PyAudio Mac Core Stream Info Tests."""

import os
import unittest
import sys

import pyaudio

# To skip tests requiring hardware, set this environment variable:
SKIP_HW_TESTS = 'PYAUDIO_SKIP_HW_TESTS' in os.environ
# If unset, defaults to default devices.
INPUT_DEVICE_INDEX = os.environ.get('PYAUDIO_INPUT_DEVICE_INDEX', None)
OUTPUT_DEVICE_INDEX = os.environ.get('PYAUDIO_OUTPUT_DEVICE_INDEX', None)


@unittest.skipIf('darwin' not in sys.platform, 'macOS-only test.')
class MacCoreStreamInfoTests(unittest.TestCase):

    def test_getters(self):
        channel_map = (1, 0)
        stream_info = pyaudio.PaMacCoreStreamInfo(
            flags=pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice,
            channel_map=channel_map)
        self.assertEqual(stream_info.channel_map, channel_map)
        self.assertEqual(stream_info.flags,
                         pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice)

    def test_default(self):
        stream_info = pyaudio.PaMacCoreStreamInfo()
        self.assertEqual(stream_info.channel_map, None)
        self.assertEqual(stream_info.flags,
                         pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice)

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_macos_channel_map(self):
        # reverse: R-L stereo
        channel_map = (1, 0)
        stream_info = pyaudio.PaMacCoreStreamInfo(
            flags=pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice,  # default
            channel_map=channel_map)

        self.assertEqual(stream_info.flags,
                         pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice)
        self.assertEqual(stream_info.channel_map, channel_map)

        p = pyaudio.PyAudio()
        stream = p.open(
            format=p.get_format_from_width(2),
            channels=2,
            rate=44100,
            output=True,
            output_host_api_specific_stream_info=stream_info,
            start=False)

        # Make sure portaudio no longer depends on state inside this object
        # once the stream is initialized.
        del stream_info

        self.assertFalse(stream.is_active())
        self.assertTrue(stream.is_stopped())

        stream.start_stream()

        self.assertTrue(stream.is_active())
        self.assertFalse(stream.is_stopped())

        stream.stop_stream()

        self.assertFalse(stream.is_active())
        self.assertTrue(stream.is_stopped())

        stream.close()

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_macos_inline_stream_info(self):
        p = pyaudio.PyAudio()
        stream = p.open(
            format=p.get_format_from_width(2),
            channels=2,
            rate=44100,
            output=True,
            # Instantiate inline, so PaMacCoreStreamInfo instance could get GCed
            # subsequently. Ensure the stream still works.
            output_host_api_specific_stream_info=pyaudio.PaMacCoreStreamInfo(
                flags=pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice,  # default
                channel_map=(1, 0)),  # reverse: R-L stereo
            start=False)

        self.assertFalse(stream.is_active())
        self.assertTrue(stream.is_stopped())

        stream.start_stream()

        self.assertTrue(stream.is_active())
        self.assertFalse(stream.is_stopped())

        stream.stop_stream()

        self.assertFalse(stream.is_active())
        self.assertTrue(stream.is_stopped())

        stream.close()
