"""PyAudio: Cross-platform audio I/O with PortAudio.

PyAudio provides Python bindings for PortAudio, the cross-platform audio I/O
library. With PyAudio, you can easily use Python to play and record audio on a
variety of platforms, such as GNU/Linux, Microsoft Windows, and Apple macOS.

PyAudio is distributed under the MIT License:

Copyright (c) 2006 Hubert Pham

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import os
import logging
import platform
from setuptools import setup, Extension
import sys

__version__ = "0.2.13"

# setup.py/setuptools will try to locate and link dynamically against portaudio,
# except on Windows. On Windows, setup.py will attempt to statically link in
# portaudio, since most users will install PyAudio from pre-compiled wheels.
#
# If you wish to compile PyAudio on Windows, use vcpkg to install portaudio with
# either:
#  - vcpkg install portaudio (for dynamic linking)
#  - vcpkg install portaudio:x86-windows-static (for 32-bit static linking)
#  - vcpkg install portaudio:x64-windows-static (for 64-bit static linking)

MAC_SYSROOT_PATH = os.environ.get("SYSROOT_PATH", None)
WIN_VCPKG_PATH = os.environ.get("VCPKG_PATH", None)

def setup_extension():
    pyaudio_module_sources = [
        'src/pyaudio/main.c',
        'src/pyaudio/device_api.c',
        'src/pyaudio/host_api.c',
        'src/pyaudio/init.c',
        'src/pyaudio/mac_core_stream_info.c',
        'src/pyaudio/misc.c',
        'src/pyaudio/stream.c',
        'src/pyaudio/stream_io.c',
        'src/pyaudio/stream_lifecycle.c',
    ]
    include_dirs = []
    external_libraries = ["portaudio"]
    external_libraries_path = []
    extra_compile_args = []
    extra_link_args = []
    defines = []

    if sys.platform == 'darwin':
        # Support only dynamic linking with portaudio, since the supported path
        # is to install portaudio using a package manager (e.g., Homebrew).
        # TODO: let users pass in location of portaudio library on command line.
        defines += [('MACOS', '1')]

        include_dirs += [
            '/usr/local/include', '/usr/include', '/opt/homebrew/include'
        ]
        external_libraries_path += [
            path
            for path in ('/usr/local/lib', '/usr/lib', '/opt/homebrew/lib')
            if os.path.exists(path)
        ]

        if MAC_SYSROOT_PATH:
            extra_compile_args += ["-isysroot", MAC_SYSROOT_PATH]
            extra_link_args += ["-isysroot", MAC_SYSROOT_PATH]
    elif sys.platform == 'win32':
        # Only supports statically linking with portaudio, since the typical
        # way users install PyAudio on win32 is through pre-compiled wheels.
        bits = platform.architecture()[0]
        if '64' in bits:
            defines.append(('MS_WIN64', '1'))

        if WIN_VCPKG_PATH:
            include_dirs += [os.path.join(WIN_VCPKG_PATH, 'include')]
            external_libraries_path = [os.path.join(WIN_VCPKG_PATH, 'lib')]
        else:
            # If VCPKG_PATH is not set, it is likely a user oversight, as the
            # extension compiler likely won't be able to find the portaudio
            # library to link against.
            logging.warning("Warning: VCPKG_PATH envrionment variable not set.")
            # So if VCPKG_PATH is not set, be sure to manually add the correct
            # path to portaudio's include and lib dirs, or use setuptools
            # build_ext to specify them on the command line.
            external_libraries.remove("portaudio")

        # The static portaudio lib does not include user32 and advapi32, so
        # those need to be linked manually.
        external_libraries += ["user32", "Advapi32"]
        # For static linking, use MT flag to match both vcpkg's portaudio and
        # the standard portaudio cmake settings. For details, see:
        # https://devblogs.microsoft.com/cppblog/vcpkg-updates-static-linking-is-now-available/
        extra_compile_args += ["/MT"]
    else:
        # GNU/Linux and other posix-like OSes will dynamically link to
        # portaudio, installed by the package manager.
        include_dirs += ['/usr/local/include', '/usr/include']
        external_libraries_path += ['/usr/local/lib', '/usr/lib']

    return Extension(
        'pyaudio._portaudio',
        sources=pyaudio_module_sources,
        include_dirs=include_dirs,
        define_macros=defines,
        libraries=external_libraries,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        library_dirs=external_libraries_path)


with open('README.md', 'r') as fh:
    long_description = fh.read()


setup(
    name='PyAudio',
    version=__version__,
    author="Hubert Pham",
    url="https://people.csail.mit.edu/hubert/pyaudio/",
    description="Cross-platform audio I/O with PortAudio",
    long_description=long_description,
    long_description_content_type='text/markdown',
    license="MIT",
    scripts=[],
    packages=['pyaudio'],
    package_dir={'': 'src'},
    extras_require={
        "test": ["numpy"],
    },
    ext_modules=[setup_extension()],
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Topic :: Multimedia :: Sound/Audio"
    ])
