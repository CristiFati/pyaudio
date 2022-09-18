"""PyAudio Example: Half-duplex wire between input and output."""

import sys

import pyaudio


CHUNK = 1024
WIDTH = 2
CHANNELS = 1 if sys.platform == 'darwin' else 2
RATE = 44100
RECORD_SECONDS = 5

p = pyaudio.PyAudio()

# Open input stream using default device:
stream_input = p.open(format=p.get_format_from_width(WIDTH),
                      channels=CHANNELS,
                      rate=RATE,
                      input=True,
                      frames_per_buffer=CHUNK)

# Open output stream using default device:
stream_output = p.open(format=p.get_format_from_width(WIDTH),
                       channels=CHANNELS,
                       rate=RATE,
                       output=True,
                       frames_per_buffer=CHUNK)

print('* recording')
for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
    stream_output.write(stream_input.read(CHUNK))
print('* done')

stream_input.close()
stream_output.close()
p.terminate()
