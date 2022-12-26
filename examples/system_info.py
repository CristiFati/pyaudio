"""PyAudio Example: Query PortAudio Host APIs, devices, and supported rates."""

import pyaudio


STANDARD_SAMPLE_RATES = [
    8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
    44100.0, 48000.0, 88200.0, 96000.0, 192000.0
]


def print_header(header):
    print()
    print(header)
    print("=" * len(header))


def print_device_info(p, device_info):
    for name, value in device_info.items():
        # If host API, value is an index, so also print the friendly name.
        if name == "hostApi":
            value = f"{value} ({p.get_host_api_info_by_index(value)['name']})"
        # Crashing?  See http://stackoverflow.com/a/5146914
        print(f"{name}: {value}")

    # Print supported format rates
    input_supported_rates = []
    output_supported_rates = []
    full_duplex_rates = []

    for rate in STANDARD_SAMPLE_RATES:
        if device_info["maxInputChannels"] > 0:
            try:
                if p.is_format_supported(
                        rate,
                        input_device=device_info["index"],
                        input_channels=device_info["maxInputChannels"],
                        input_format=pyaudio.paInt16):
                    input_supported_rates.append(rate)
            except ValueError:
                pass

        if device_info["maxOutputChannels"] > 0:
            try:
                if p.is_format_supported(
                        rate,
                        output_device=device_info["index"],
                        output_channels=device_info["maxOutputChannels"],
                        output_format=pyaudio.paInt16):
                    output_supported_rates.append(rate)
            except ValueError:
                pass

        if (device_info["maxInputChannels"] > 0
                and device_info["maxOutputChannels"] > 0):
            try:
                if p.is_format_supported(
                        rate,
                        input_device=device_info["index"],
                        input_channels=device_info["maxInputChannels"],
                        input_format=pyaudio.paInt16,
                        output_device=device_info["index"],
                        output_channels=device_info["maxOutputChannels"],
                        output_format=pyaudio.paInt16):
                    full_duplex_rates.append(rate)
            except ValueError:
                pass

    if input_supported_rates:
        print(f"Input rates: {input_supported_rates}")
    if output_supported_rates:
        print(f"Output rates: {output_supported_rates}")
    if full_duplex_rates:
        print(f"Full duplex rates: {full_duplex_rates}")
    print("-" * 32)


def print_system_info(p):
    print_header("PortAudio System Info")
    print(f"Version: {pyaudio.get_portaudio_version()}")
    print(f"Version Text: {pyaudio.get_portaudio_version_text()}")
    print(f"Number of Host APIs: {p.get_host_api_count()}")
    print(f"Number of Devices: {p.get_device_count()}")


def print_all_host_apis(p):
    print_header("Host APIs")
    for i in range(p.get_host_api_count()):
        for key, value in p.get_host_api_info_by_index(i).items():
            print(f"{key}: {value}")
        print("-" * 26)


def print_all_device_infos(p):
    print_header("Devices")
    for i in range(p.get_device_count()):
        print_device_info(p, p.get_device_info_by_index(i))


def print_default_devices(p):
    print_header("Default Devices")
    try:
        device_info = p.get_default_input_device_info()
    except IOError as err:
        print(f"No Input devices: {err}")
    else:
        print(f"Default Input Device: {device_info['index']}")
        print_device_info(p, device_info)

    try:
        device_info = p.get_default_output_device_info()
    except IOError as err:
        print(f"No Output devices: {err}")
    else:
        print(f"Default Output Device: {device_info['index']}")
        print_device_info(p, device_info)


if __name__ == "__main__":
    p = pyaudio.PyAudio()

    print_system_info(p)
    print_all_host_apis(p)
    print_all_device_infos(p)
    print_default_devices(p)

    p.terminate()
