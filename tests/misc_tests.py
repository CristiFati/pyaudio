"""PyAudio misc tests."""

import unittest

import pyaudio
import alsa_utils

setUpModule = alsa_utils.disable_error_handler_output
tearDownModule = alsa_utils.disable_error_handler_output


class MiscTests(unittest.TestCase):

    def test_get_sample_size(self):
        self.assertEqual(pyaudio.get_sample_size(pyaudio.paFloat32), 4)
        self.assertEqual(pyaudio.get_sample_size(pyaudio.paInt32), 4)
        self.assertEqual(pyaudio.get_sample_size(pyaudio.paInt24), 3)
        self.assertEqual(pyaudio.get_sample_size(pyaudio.paInt16), 2)
        self.assertEqual(pyaudio.get_sample_size(pyaudio.paInt8), 1)
        self.assertEqual(pyaudio.get_sample_size(pyaudio.paUInt8), 1)

    def test_get_format_from_width(self):
        self.assertEqual(pyaudio.get_format_from_width(1, unsigned=True),
                         pyaudio.paUInt8)
        self.assertEqual(pyaudio.get_format_from_width(1, unsigned=False),
                         pyaudio.paInt8)
        self.assertEqual(pyaudio.get_format_from_width(2), pyaudio.paInt16)
        self.assertEqual(pyaudio.get_format_from_width(3), pyaudio.paInt24)
        self.assertEqual(pyaudio.get_format_from_width(4), pyaudio.paFloat32)
        with self.assertRaises(ValueError):
            pyaudio.get_format_from_width(-1)
        with self.assertRaises(ValueError):
            pyaudio.get_format_from_width(5)

    def test_get_format_from_width_pyaudio(self):
        p = pyaudio.PyAudio()
        self.assertEqual(p.get_format_from_width(1, unsigned=True),
                         pyaudio.paUInt8)
        self.assertEqual(p.get_format_from_width(1, unsigned=False),
                         pyaudio.paInt8)
        self.assertEqual(p.get_format_from_width(2), pyaudio.paInt16)
        self.assertEqual(p.get_format_from_width(3), pyaudio.paInt24)
        self.assertEqual(p.get_format_from_width(4), pyaudio.paFloat32)
        with self.assertRaises(ValueError):
            p.get_format_from_width(-1)
        with self.assertRaises(ValueError):
            p.get_format_from_width(5)

        p.terminate()

    def test_get_portaudio_version(self):
        self.assertGreater(pyaudio.get_portaudio_version(), 0)

    def test_get_portaudio_version_text(self):
        self.assertGreater(len(pyaudio.get_portaudio_version_text()), 0)
