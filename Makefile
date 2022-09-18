# This is the PyAudio distribution makefile.

.PHONY: docs clean build

VERSION := 0.2.13
PYTHON ?= python3
SPHINX ?= sphinx-build
DOCS_OUTPUT = docs/
# To build the docs, we need to first build the library. Hardcode the lib output
# directory, since different versions of Python and setuptools use different
# default directory names.
BUILD_DIR := build/lib
BUILD_STAMP := $(BUILD_DIR)/build
BUILD_ARGS := --build-platlib $(BUILD_DIR)
SRCFILES := src/pyaudio/*.c src/pyaudio/*.h src/pyaudio/*.py
EXAMPLES := examples/*.py
TESTS := tests/*.py

what:
	@echo "make targets:"
	@echo
	@echo " tarball    : build source tarball"
	@echo " docs       : generate documentation (requires sphinx)"
	@echo " clean      : remove build files"
	@echo
	@echo "To build pyaudio, run:"
	@echo
	@echo "   python setup.py install"

clean:
	@rm -rf build dist MANIFEST $(DOCS_OUTPUT) src/pyaudio/*.pyc \
	src/pyaudio/*.so src/pyaudio/__pycache__

######################################################################
# Documentation
######################################################################

build: $(BUILD_STAMP)

$(BUILD_STAMP): $(SRCFILES)
	$(PYTHON) setup.py build $(BUILD_ARGS)
	touch $@

docs: build
	PYTHONPATH=$(BUILD_DIR) $(SPHINX) -b html sphinx/ $(DOCS_OUTPUT)

######################################################################
# Source Tarball
######################################################################
tarball: $(SRCFILES) $(EXAMPLES) $(TESTS) MANIFEST.in
	@$(PYTHON) setup.py sdist
