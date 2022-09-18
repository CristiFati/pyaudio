Example: Blocking Mode Audio I/O
--------------------------------

.. literalinclude:: ../examples/play_wave.py

To use PyAudio, first instantiate PyAudio using :py:func:`pyaudio.PyAudio` (1),
which acquires system resources for PortAudio.

To record or play audio, open a stream on the desired device with the
desired audio parameters using :py:func:`pyaudio.PyAudio.open`
(2). This sets up a :py:class:`pyaudio.PyAudio.Stream` to play or record
audio.

Play audio by writing audio data to the stream using
:py:func:`pyaudio.PyAudio.Stream.write`, or read audio data from the stream
using :py:func:`pyaudio.PyAudio.Stream.read`. (3)

Note that in "blocking mode", each :py:func:`pyaudio.PyAudio.Stream.write` or
:py:func:`pyaudio.PyAudio.Stream.read` blocks until all frames have been
played/recorded. An alternative approach is "callback mode", described below, in
which PyAudio invokes a user-defined function to process recorded audio or
generate output audio.

Use :py:func:`pyaudio.PyAudio.Stream.close` to close the stream. (4)

Finally, terminate the PortAudio session and release system resources using
:py:func:`pyaudio.PyAudio.terminate`. (5)

Example: Callback Mode Audio I/O
--------------------------------

.. literalinclude:: ../examples/play_wave_callback.py

In callback mode, PyAudio will call a user-defined callback function (1)
whenever it needs new audio data to play and/or when new recorded audio data
becomes available. PyAudio calls the callback function in a separate thread. The
callback function must have the following signature ``callback(<input_data>,
<frame_count>, <time_info>, <status_flag>)``. It must return a tuple containing
``frame_count`` frames of audio data to output (for output streams) and a flag
signifying whether there are more expected frames to play or record. (For
input-only streams, the audio data portion of the return value is ignored.)

The audio stream starts processing once the stream is opened (3), which will
call the callback function repeatedly until that function returns
:py:data:`pyaudio.paComplete` or :py:data:`pyaudio.paAbort`, or until either
:py:data:`pyaudio.PyAudio.Stream.stop` or
:py:data:`pyaudio.PyAudio.Stream.close` is called. Note that if the callback
returns fewer frames than the :py:data:`frame_count` argument (2), the stream
automatically closes after those frames are played.

To keep the stream active, the main thread must remain alive, e.g., by sleeping
(4). In the example above, once the entire wavefile is read,
:py:data:`wf.readframes(frame_count)` will eventually return fewer than the
requested frames. The stream will stop, and the while loop (4) will end.
