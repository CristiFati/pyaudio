"""PyAudio Host API and Device API tests."""

import os
import unittest

import pyaudio
import alsa_utils

# To skip tests requiring hardware, set this environment variable:
SKIP_HW_TESTS = 'PYAUDIO_SKIP_HW_TESTS' in os.environ

setUpModule = alsa_utils.disable_error_handler_output
tearDownModule = alsa_utils.disable_error_handler_output


class HostDeviceApiTests(unittest.TestCase):
    def setUp(self):
        self.p = pyaudio.PyAudio()

    def tearDown(self):
        self.p.terminate()

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_host_api(self):
        """Host API tests"""
        self.assertTrue(self.p.get_host_api_count() > 0)
        default_info = self.p.get_default_host_api_info()
        default_index = default_info['index']
        default_type = default_info['type']
        self.assertDictEqual(
            default_info,
            self.p.get_host_api_info_by_index(default_index))
        self.assertDictEqual(
            default_info,
            self.p.get_host_api_info_by_type(default_type))

        with self.assertRaises(TypeError):
            self.p.get_host_api_info_by_type("not an integer")
        with self.assertRaises(IOError):
            self.p.get_host_api_info_by_type(-2)
        with self.assertRaises(TypeError):
            self.p.get_host_api_info_by_index("not an integer")
        with self.assertRaises(IOError):
            self.p.get_host_api_info_by_index(-2)

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_device_api(self):
        """Device API tests"""
        self.assertTrue(self.p.get_device_count() > 0)

        default_input = self.p.get_default_input_device_info()
        default_output = self.p.get_default_output_device_info()
        self.assertDictEqual(
            default_input,
            self.p.get_device_info_by_index(default_input['index']))
        self.assertDictEqual(
            default_output,
            self.p.get_device_info_by_index(default_output['index']))

        with self.assertRaises(TypeError):
            self.p.get_device_info_by_index("not an integer")
        with self.assertRaises(IOError):
            self.p.get_device_info_by_index(-2)

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_format_supported(self):
        with self.assertRaises(ValueError):
            # Need to specify input_device or output_device
            self.p.is_format_supported(44100)

        with self.assertRaises(ValueError):
            self.p.is_format_supported(44100, input_device=0,
                                       input_channels=-2,
                                       input_format=pyaudio.paInt16)
