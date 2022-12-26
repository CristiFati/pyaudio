"""PyAudio Example: macOS-only: Play a wave file with channel maps."""

import wave
import sys

import pyaudio


CHUNK = 1024

if len(sys.argv) < 2:
    print(f'Plays a wave file. Usage: {sys.argv[0]} filename.wav')
    sys.exit(-1)

# standard L-R stereo
# channel_map = (0, 1)

# reverse: R-L stereo
# channel_map = (1, 0)

# no audio
# channel_map = (-1, -1)

# left channel audio --> left speaker; no right channel
# channel_map = (0, -1)

# right channel audio --> right speaker; no left channel
# channel_map = (-1, 1)

# left channel audio --> right speaker
# channel_map = (-1, 0)

# right channel audio --> left speaker
channel_map = (1, -1)
# etc...
try:
    stream_info = pyaudio.PaMacCoreStreamInfo(
        flags=pyaudio.PaMacCoreStreamInfo.paMacCorePlayNice,
        channel_map=channel_map)
except AttributeError:
    print(
        'Could not find PaMacCoreStreamInfo. Ensure you are running on macOS.')
    sys.exit(-1)

print('Stream Info Flags:', stream_info.flags)
print('Stream Info Channel Map:', stream_info.channel_map)

with wave.open(sys.argv[1], 'rb') as wf:
    p = pyaudio.PyAudio()
    stream = p.open(
        format=p.get_format_from_width(wf.getsampwidth()),
        channels=wf.getnchannels(),
        rate=wf.getframerate(),
        output=True,
        output_host_api_specific_stream_info=stream_info)

    # Play stream
    while len(data := wf.readframes(CHUNK)):  # Requires Python 3.8+ for :=
        stream.write(data)

    stream.close()
    p.terminate()
