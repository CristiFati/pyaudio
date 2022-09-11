"""PyAudio Mac Core Stream Info Tests."""

import unittest
import sys

import pyaudio

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
