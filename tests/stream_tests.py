"""Stream tests."""

import os
import time
import threading
import unittest

import pyaudio
import alsa_utils

# To skip tests requiring hardware, set this environment variable:
SKIP_HW_TESTS = 'PYAUDIO_SKIP_HW_TESTS' in os.environ
# If unset, defaults to default devices.
INPUT_DEVICE_INDEX = os.environ.get('PYAUDIO_INPUT_DEVICE_INDEX', None)
OUTPUT_DEVICE_INDEX = os.environ.get('PYAUDIO_OUTPUT_DEVICE_INDEX', None)

setUpModule = alsa_utils.disable_error_handler_output
tearDownModule = alsa_utils.disable_error_handler_output


class StreamTests(unittest.TestCase):

    def setUp(self):
        self.p = pyaudio.PyAudio()
        self.input_device = INPUT_DEVICE_INDEX and int(INPUT_DEVICE_INDEX)
        self.output_device = OUTPUT_DEVICE_INDEX and int(OUTPUT_DEVICE_INDEX)

        # Different platforms/devices support different number of channels for
        # input streams. Inspect the desired input device and use the maximum
        # number of channels.
        try:
            input_device_info = self.p.get_host_api_info_by_index(
                self.input_device
            ) if self.input_device else self.p.get_default_input_device_info()
        except OSError as err:
            raise OSError(f"Invalid device index {self.input_device}") from err
        self.input_channels = input_device_info['maxInputChannels']
        if self.input_channels < 1:
            raise OSError("Invalid number of input channels for device")

    def tearDown(self):
        self.p.terminate()

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_output_start_stop_stream_properties(self):
        out_stream = self.p.open(
            format=self.p.get_format_from_width(2),
            channels=2,
            rate=44100,
            output=True)

        self.assertTrue(out_stream.is_active())
        self.assertFalse(out_stream.is_stopped())
        self.assertGreaterEqual(out_stream.get_output_latency(), 0)
        self.assertGreaterEqual(out_stream.get_input_latency(), 0)
        self.assertGreaterEqual(out_stream.get_time(), 0)
        self.assertGreaterEqual(out_stream.get_cpu_load(), 0)
        self.assertGreaterEqual(out_stream.get_write_available(), 0)

        out_stream.stop_stream()

        self.assertTrue(out_stream.is_stopped())
        self.assertFalse(out_stream.is_active())
        # Make sure we can still read PaStreamInfo, as stream is not closed yet.
        self.assertGreater(out_stream.get_output_latency(), 0)
        self.assertGreaterEqual(out_stream.get_input_latency(), 0)
        self.assertGreaterEqual(out_stream.get_time(), 0)
        self.assertGreaterEqual(out_stream.get_cpu_load(), 0)

        out_stream.start_stream()

        self.assertFalse(out_stream.is_stopped())
        self.assertTrue(out_stream.is_active())
        # Make sure we can still read PaStreamInfo, as stream is not closed yet.
        self.assertGreater(out_stream.get_output_latency(), 0)
        self.assertGreaterEqual(out_stream.get_input_latency(), 0)
        self.assertGreaterEqual(out_stream.get_time(), 0)
        self.assertGreaterEqual(out_stream.get_cpu_load(), 0)
        self.assertGreaterEqual(out_stream.get_write_available(), 0)

        out_stream.close()

        with self.assertRaises(IOError):
            self.assertTrue(out_stream.is_stopped())
        with self.assertRaises(IOError):
            self.assertFalse(out_stream.is_active())
        with self.assertRaises(IOError):
            out_stream.get_output_latency()
        with self.assertRaises(IOError):
            out_stream.get_time()
        with self.assertRaises(IOError):
            out_stream.get_cpu_load()
        with self.assertRaises(IOError):
            out_stream.get_write_available()

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_input_start_stop_stream_properties(self):
        in_stream = self.p.open(
            format=self.p.get_format_from_width(2),
            channels=self.input_channels,
            rate=44100,
            input=True)

        self.assertTrue(in_stream.is_active())
        self.assertFalse(in_stream.is_stopped())
        self.assertGreaterEqual(in_stream.get_output_latency(), 0)
        self.assertGreaterEqual(in_stream.get_input_latency(), 0)
        self.assertGreaterEqual(in_stream.get_time(), 0)
        self.assertGreaterEqual(in_stream.get_cpu_load(), 0)
        self.assertGreaterEqual(in_stream.get_read_available(), 0)

        in_stream.stop_stream()

        self.assertTrue(in_stream.is_stopped())
        self.assertFalse(in_stream.is_active())
        # Make sure we can still read PaStreamInfo, as stream is not closed yet.
        self.assertGreaterEqual(in_stream.get_output_latency(), 0)
        self.assertGreater(in_stream.get_input_latency(), 0)
        self.assertGreaterEqual(in_stream.get_time(), 0)
        self.assertGreaterEqual(in_stream.get_cpu_load(), 0)

        in_stream.start_stream()

        self.assertFalse(in_stream.is_stopped())
        self.assertTrue(in_stream.is_active())
        # Make sure we can still read PaStreamInfo, as stream is not closed yet.
        self.assertGreaterEqual(in_stream.get_output_latency(), 0)
        self.assertGreater(in_stream.get_input_latency(), 0)
        self.assertGreaterEqual(in_stream.get_time(), 0)
        self.assertGreaterEqual(in_stream.get_cpu_load(), 0)
        self.assertGreaterEqual(in_stream.get_read_available(), 0)

        in_stream.close()

        with self.assertRaises(IOError):
            self.assertTrue(in_stream.is_stopped())
        with self.assertRaises(IOError):
            self.assertFalse(in_stream.is_active())
        with self.assertRaises(IOError):
            in_stream.get_output_latency()
        with self.assertRaises(IOError):
            in_stream.get_time()
        with self.assertRaises(IOError):
            in_stream.get_cpu_load()
        with self.assertRaises(IOError):
            in_stream.get_read_available()

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_close_stream_from_pyaudio(self):
        out_stream = self.p.open(
            format=self.p.get_format_from_width(2),
            channels=2,
            rate=44100,
            output=True)

        self.assertTrue(out_stream.is_active())
        self.p.close(out_stream)
        with self.assertRaises(OSError) as err:
            out_stream.is_stopped()
        self.assertEqual(err.exception.args[0], pyaudio.paBadStreamPtr)
        self.assertTrue(out_stream not in self.p._streams)

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_close_unknown_stream(self):
        out_stream = self.p.open(
            format=self.p.get_format_from_width(2),
            channels=2,
            rate=44100,
            output=True)

        # Simulate the condition where out_stream is unknown to self.p:
        self.p._streams = set()
        with self.assertRaises(ValueError):
            self.p.close(out_stream)

        out_stream.close()

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_output_blocking(self):
        out_stream = self.p.open(
            format=self.p.get_format_from_width(2),
            channels=2,
            rate=44100,
            output=True)
        out_stream.write(b'\0' * 512)

        with self.assertRaises(IOError):
            out_stream.read(512)

        out_stream.close()

    @unittest.skipIf(SKIP_HW_TESTS, 'Hardware device required.')
    def test_input_blocking(self):
        width = 2
        in_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=self.input_channels,
            rate=44100,
            input=True)
        samples = in_stream.read(512)

        with self.assertRaises(IOError):
            in_stream.write(b'\0' * 512)

        in_stream.close()
        self.assertEqual(len(samples), 512 * width * self.input_channels)

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_return_none_callback(self):
        """Ensure that return None ends the stream."""
        num_times_called = 0

        def out_callback(_, frame_count, time_info, status):
            nonlocal num_times_called
            num_times_called += 1
            return (None, pyaudio.paContinue)

        out_stream = self.p.open(
            format=self.p.get_format_from_width(2),
            channels=2,
            rate=44100,
            output=True,
            output_device_index=self.output_device,
            stream_callback=out_callback)
        out_stream.start_stream()
        time.sleep(0.5)
        out_stream.stop_stream()
        self.assertEqual(num_times_called, 1)

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_excess_output_callback(self):
        """Ensure that returning more bytes than allowed does not fail."""
        num_times_called = 0
        width = 2
        channels = 2
        bytes_per_frame = width * channels

        def out_callback(_, frame_count, time_info, status):
            nonlocal num_times_called
            num_times_called += 1
            # Make sure this is called twice, so we know that the first time
            # didn't crash (at least).
            result = (pyaudio.paComplete
                      if num_times_called == 2 else pyaudio.paContinue)
            max_allowed_bytes = frame_count * bytes_per_frame
            return (b'\1' * (max_allowed_bytes * 2), result)

        out_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=channels,
            rate=44100,
            output=True,
            output_device_index=self.output_device,
            stream_callback=out_callback)
        out_stream.start_stream()
        time.sleep(0.5)
        out_stream.stop_stream()
        self.assertEqual(num_times_called, 2)

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_device_lock_gil_order(self):
        """Ensure no deadlock between Pa_{Open,Start,Stop}Stream and GIL."""
        # This test targets macOS CoreAudio, which seems to use audio device
        # locks. The test is less relevant on ALSA and Win32 MME, which don't
        # seem to suffer even if the GIL is held while calling PortAudio.
        def in_callback(in_data, frame_count, time_info, status):
            # Release the GIL for a bit, but on macOS, still hold the device
            # lock.
            time.sleep(1)
            # Note: on macOS, must return paContinue; paComplete will deadlock
            # in the underlying call to AudioOutputUnitStop.
            return (None, pyaudio.paContinue)

        in_stream = self.p.open(
            format=self.p.get_format_from_width(2),
            channels=self.input_channels,
            rate=44100,
            input=True,
            start=False,
            input_device_index=self.input_device,
            stream_callback=in_callback)
        # In a separate (C) thread, portaudio/driver will grab the device lock,
        # then the GIL to call in_callback.
        in_stream.start_stream()
        # Wait a bit to let that callback thread start.
        time.sleep(0.5)
        # in_callback will eventually drop the GIL when executing time.sleep
        # (while retaining the device lock), allowing the following code to
        # run. All stream operations MUST release the GIL before attempting to
        # acquire the device lock. If that discipline is violated, the following
        # code would wait for the device lock while holding the GIL, while the
        # in_callback thread would be waiting for the GIL once time.sleep
        # completes (while holding the device lock), leading to deadlock.
        in_stream.stop_stream()

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_stream_state_gil(self):
        """Ensure no deadlock between Pa_IsStream{Active,Stopped} and GIL."""
        rate = 44100  # frames per second
        width = 2  # bytes per sample
        frames_per_chunk = 1024

        def out_callback(_, frame_count, time_info, status):
            return ('', pyaudio.paComplete)

        def in_callback(in_data, frame_count, time_info, status):
            # Release the GIL for a bit
            time.sleep(1)
            return (None, pyaudio.paComplete)

        in_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=self.input_channels,
            rate=rate,
            input=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            input_device_index=self.input_device,
            stream_callback=in_callback)
        out_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=2,
            rate=rate,
            output=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            output_device_index=self.output_device,
            stream_callback=out_callback)
        # In a separate (C) thread, portaudio/driver will grab the device lock,
        # then the GIL to call in_callback.
        in_stream.start_stream()
        # Wait a bit to let that callback thread start.
        time.sleep(0.5)
        # in_callback will eventually drop the GIL when executing
        # time.sleep (while retaining the device lock), allowing the
        # following code to run. Checking the state of the stream MUST
        # not require the device lock, but if it does, it must release the GIL
        # before attempting to acquire the device
        # lock. Otherwise, the following code will wait for the device
        # lock (while holding the GIL), while the in_callback thread
        # will be waiting for the GIL once time.sleep completes (while
        # holding the device lock), leading to deadlock.
        self.assertTrue(in_stream.is_active())
        self.assertFalse(in_stream.is_stopped())

        self.assertTrue(out_stream.is_stopped())
        self.assertFalse(out_stream.is_active())
        out_stream.start_stream()
        self.assertFalse(out_stream.is_stopped())
        self.assertTrue(out_stream.is_active())
        time.sleep(1)
        in_stream.stop_stream()
        out_stream.stop_stream()

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_get_stream_time_gil(self):
        """Ensure no deadlock between PA_GetStreamTime and GIL."""
        rate = 44100 # frames per second
        width = 2    # bytes per sample
        frames_per_chunk = 1024

        def out_callback(_, frame_count, time_info, status):
            return ('', pyaudio.paComplete)

        def in_callback(in_data, frame_count, time_info, status):
            # Release the GIL for a bit
            time.sleep(1)
            return (None, pyaudio.paComplete)

        in_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=self.input_channels,
            rate=rate,
            input=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            input_device_index=self.input_device,
            stream_callback=in_callback)
        out_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=2,
            rate=rate,
            output=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            output_device_index=self.output_device,
            stream_callback=out_callback)
        # In a separate (C) thread, portaudio/driver will grab the device lock,
        # then the GIL to call in_callback.
        in_stream.start_stream()
        # Wait a bit to let that callback thread start.
        time.sleep(0.5)
        # in_callback will eventually drop the GIL when executing
        # time.sleep (while retaining the device lock), allowing the
        # following code to run. Getting the stream time MUST not
        # require the device lock, but if it does, it must release the
        # GIL before attempting to acquire the device lock. Otherwise,
        # the following code will wait for the device lock (while
        # holding the GIL), while the in_callback thread will be
        # waiting for the GIL once time.sleep completes (while holding
        # the device lock), leading to deadlock.
        self.assertGreater(in_stream.get_time(), -1)
        self.assertGreater(out_stream.get_time(), 1)

        time.sleep(1)
        in_stream.stop_stream()
        out_stream.stop_stream()

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_get_stream_cpuload_gil(self):
        """Ensure no deadlock between Pa_GetStreamCpuLoad and GIL."""
        rate = 44100 # frames per second
        width = 2    # bytes per sample
        frames_per_chunk = 1024

        def out_callback(_, frame_count, time_info, status):
            return ('', pyaudio.paComplete)

        def in_callback(in_data, frame_count, time_info, status):
            # Release the GIL for a bit
            time.sleep(1)
            return (None, pyaudio.paComplete)

        in_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=self.input_channels,
            rate=rate,
            input=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            input_device_index=self.input_device,
            stream_callback=in_callback)
        out_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=2,
            rate=rate,
            output=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            output_device_index=self.output_device,
            stream_callback=out_callback)
        # In a separate (C) thread, portaudio/driver will grab the device lock,
        # then the GIL to call in_callback.
        in_stream.start_stream()
        # Wait a bit to let that callback thread start.
        time.sleep(0.5)
        # in_callback will eventually drop the GIL when executing
        # time.sleep (while retaining the device lock), allowing the
        # following code to run. Getting the stream cpuload MUST not
        # require the device lock, but if it does, it must release the
        # GIL before attempting to acquire the device lock. Otherwise,
        # the following code will wait for the device lock (while
        # holding the GIL), while the in_callback thread will be
        # waiting for the GIL once time.sleep completes (while holding
        # the device lock), leading to deadlock.
        self.assertGreater(in_stream.get_cpu_load(), -1)
        self.assertGreater(out_stream.get_cpu_load(), -1)
        time.sleep(1)
        in_stream.stop_stream()
        out_stream.stop_stream()

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_get_stream_write_available_gil(self):
        """Ensure no deadlock between Pa_GetStreamWriteAvailable and GIL."""
        rate = 44100 # frames per second
        width = 2    # bytes per sample
        frames_per_chunk = 1024

        def in_callback(in_data, frame_count, time_info, status):
            # Release the GIL for a bit
            time.sleep(1)
            return (None, pyaudio.paComplete)

        in_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=self.input_channels,
            rate=rate,
            input=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            input_device_index=self.input_device,
            stream_callback=in_callback)
        out_stream = self.p.open(  # Blocking mode
            format=self.p.get_format_from_width(width),
            channels=2,
            rate=rate,
            output=True,
            frames_per_buffer=frames_per_chunk,
            output_device_index=self.output_device)
        # In a separate (C) thread, portaudio/driver will grab the device lock,
        # then the GIL to call in_callback.
        in_stream.start_stream()
        # Wait a bit to let that callback thread start.
        time.sleep(0.5)
        # in_callback will eventually drop the GIL when executing
        # time.sleep (while retaining the device lock), allowing the
        # following code to run. Getting the stream write available MUST not
        # require the device lock, but if it does, it must release the
        # GIL before attempting to acquire the device lock. Otherwise,
        # the following code will wait for the device lock (while
        # holding the GIL), while the in_callback thread will be
        # waiting for the GIL once time.sleep completes (while holding
        # the device lock), leading to deadlock.
        self.assertGreaterEqual(out_stream.get_write_available(), 0)

        time.sleep(1)
        in_stream.stop_stream()

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_get_stream_read_available_gil(self):
        """Ensure no deadlock between Pa_GetStreamReadAvailable and GIL."""
        rate = 44100  # frames per second
        width = 2  # bytes per sample
        frames_per_chunk = 1024

        def out_callback(in_data, frame_count, time_info, status):
            # Release the GIL for a bit
            time.sleep(1)
            return (None, pyaudio.paComplete)

        in_stream = self.p.open(  # Blocking mode
            format=self.p.get_format_from_width(width),
            channels=self.input_channels,
            rate=rate,
            input=True,
            frames_per_buffer=frames_per_chunk,
            input_device_index=self.input_device)
        out_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=2,
            rate=rate,
            output=True,
            start=False,
            frames_per_buffer=frames_per_chunk,
            output_device_index=self.output_device,
            stream_callback=out_callback)
        # In a separate (C) thread, portaudio/driver will grab the device lock,
        # then the GIL to call in_callback.
        out_stream.start_stream()
        # Wait a bit to let that callback thread start.
        time.sleep(0.5)
        # in_callback will eventually drop the GIL when executing
        # time.sleep (while retaining the device lock), allowing the
        # following code to run. Getting the stream read available MUST not
        # require the device lock, but if it does, it must release the
        # GIL before attempting to acquire the device lock. Otherwise,
        # the following code will wait for the device lock (while
        # holding the GIL), while the in_callback thread will be
        # waiting for the GIL once time.sleep completes (while holding
        # the device lock), leading to deadlock.
        self.assertGreaterEqual(in_stream.get_read_available(), 0)

        time.sleep(1)
        in_stream.stop_stream()

    @unittest.skipIf(SKIP_HW_TESTS, 'Sound hardware required.')
    def test_terminate_gil(self):
        """Ensure no deadlock between Pa_Terminate and GIL."""
        # This test targets macOS CoreAudio, which seems to use audio device
        # locks. The test is less relevant on ALSA and Win32 MME, which don't
        # seem to suffer even if the GIL is held while calling PortAudio.
        width = 2
        channels = 2
        bytes_per_frame = width * channels
        event = threading.Event()

        def out_callback(in_data, frame_count, time_info, status):
            event.set()
            time.sleep(0.5)  # Release the GIL for a bit
            event.clear()
            return (b'\1' * frame_count * bytes_per_frame, pyaudio.paContinue)

        out_stream = self.p.open(
            format=self.p.get_format_from_width(width),
            channels=channels,
            rate=44100,
            output=True,
            start=False,
            output_device_index=self.output_device,
            stream_callback=out_callback)
        # In a separate (C) thread, portaudio/driver will grab the device lock,
        # then the GIL to call in_callback.
        out_stream.start_stream()
        # Wait a bit to let that callback thread start. For output streams on
        # macOS, it's important to have one complete call to out_callback before
        # attempting to terminate. Otherwise, the underlying call to
        # AudioOutputUnitStop will deadlock.
        time.sleep(0.6)
        # out_callback will eventually drop the GIL when executing time.sleep
        # (while retaining the device lock), allowing the following code to
        # run. Terminating PyAudio MUST release the GIL before attempting to
        # acquire the device lock (if the lock is needed). If that discipline is
        # violated, the following code would wait for the device lock while
        # holding the GIL, while the out_callback thread would be waiting for
        # the GIL once time.sleep completes (while holding the device lock),
        # leading to deadlock.
        #
        # Wait until out_callback is about to sleep, plus a little extra to
        # help ensure that sleep() is called first before we concurrently call
        # into self.p.terminate().
        event.wait()
        time.sleep(0.1)
        self.p.terminate()
