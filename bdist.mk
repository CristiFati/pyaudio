distclean: clean
	@rm -rf $(PYAUDIODIR)/PyAudio.egg-info

######################################################################
# Binary Distribution
######################################################################

PYAUDIODIR   := src
BUILDDIR     := build
DISTDIR      := dist
PACKAGINGDIR := packaging
OUTPUT       := $(DISTDIR)
PORTAUDIODIR ?= portaudio-v19

SRCFILES = $(addprefix $(PYAUDIODIR)/,\
	   _portaudiomodule.c         \
	   _portaudiomodule.h         \
           pyaudio.py)                \
           setup.py

# windows / mingw

WIN32_PYTHON26 := /cygdrive/c/Python26/python.exe
WIN32_PYTHON27 := /cygdrive/c/Python27/python.exe
WIN32_PYTHON32 := /cygdrive/c/Python32/python.exe
WIN32_PYTHON33 := /cygdrive/c/Python33/python.exe

# distribution output:
WIN32_PKG          := PyAudio-$(VERSION).win32.exe
WIN32_PYTHON26_PKG := $(OUTPUT)/pyaudio-$(VERSION).py26.exe
WIN32_PYTHON27_PKG := $(OUTPUT)/pyaudio-$(VERSION).py27.exe
WIN32_PYTHON32_PKG := $(OUTPUT)/pyaudio-$(VERSION).py32.exe
WIN32_PYTHON33_PKG := $(OUTPUT)/pyaudio-$(VERSION).py33.exe

# mac os x:
CLEAN_MPKG       := $(PACKAGINGDIR)/remove-macosx-mpkg-rules.py

SYS_PYTHON26_DIR := /System/Library/Frameworks/Python.framework/Versions/2.6/
SYS_PYTHON27_DIR := /System/Library/Frameworks/Python.framework/Versions/2.7/
MAC_PYTHON27_DIR := /Library/Frameworks/Python.framework/Versions/2.7/
MAC_PYTHON32_DIR := /Library/Frameworks/Python.framework/Versions/3.2/
MAC_PYTHON33_DIR := /Library/Frameworks/Python.framework/Versions/3.3/

# targets
SYS_PYTHON26_PKG := $(OUTPUT)/PyAudio-$(VERSION)-sys-py2.6-macosx10.8.mpkg
SYS_PYTHON27_PKG := $(OUTPUT)/PyAudio-$(VERSION)-sys-py2.7-macosx10.8.mpkg
MAC_PYTHON27_PKG := $(OUTPUT)/PyAudio-$(VERSION)-mac-py2.7-macosx10.8.mpkg
MAC_PYTHON32_PKG := $(OUTPUT)/PyAudio-$(VERSION)-mac-py3.2-macosx10.8.mpkg
MAC_PYTHON33_PKG := $(OUTPUT)/PyAudio-$(VERSION)-mac-py3.3-macosx10.8.mpkg

# meta package containing all installers
MPKG_INSTALLER := $(OUTPUT)/pyaudio-$(VERSION).mpkg

PACKAGEMAKER := /Applications/Xcode.app/Contents/Applications/PackageMaker.app/Contents/MacOS/PackageMaker
PACKAGEDOC := packaging/pyaudio-$(VERSION)-mpkg.pmdoc

VOLNAME_FILENAME := pyaudio-$(VERSION)
VOLNAME := PyAudio\ $(VERSION)
DS_STORE := DS_Store-$(VERSION)
# add 20480 sectors (~10MB) for safety; we'll shrink it later
SECTORS = `du -s $(MPKG_INSTALLER) | awk '{print $$1 + 20480}'`
HDIUTIL = hdiutil
MAC_SNAKEY = snakey.tif

# final dmg:
DMG := $(DISTDIR)/$(VOLNAME_FILENAME).dmg

######################################################################
# Mac OS X
######################################################################

$(MAC_PYTHON27_PKG): PYTHON_DIR=$(MAC_PYTHON27_DIR)
$(MAC_PYTHON27_PKG): BDIST_MPKG_DIR=$(PYTHON_DIR)/bin
$(MAC_PYTHON27_PKG): PYTHON=python
$(MAC_PYTHON27_PKG): CC=/usr/bin/gcc
$(MAC_PYTHON27_PKG):
	$(call _build_mac_package,$@)

$(MAC_PYTHON32_PKG): PYTHON_DIR=$(MAC_PYTHON32_DIR)
$(MAC_PYTHON32_PKG): BDIST_MPKG_DIR=$(PYTHON_DIR)/bin
$(MAC_PYTHON32_PKG): PYTHON=python3.2
$(MAC_PYTHON32_PKG): CC=/usr/bin/gcc
$(MAC_PYTHON32_PKG): SYSROOT_PATH=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
$(MAC_PYTHON32_PKG):
	$(call _build_mac_package,$@)

$(MAC_PYTHON33_PKG): PYTHON_DIR=$(MAC_PYTHON33_DIR)
$(MAC_PYTHON33_PKG): BDIST_MPKG_DIR=$(PYTHON_DIR)/bin
$(MAC_PYTHON33_PKG): PYTHON=python3.3
$(MAC_PYTHON33_PKG): CC=/usr/bin/gcc
$(MAC_PYTHON33_PKG): SYSROOT_PATH=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
$(MAC_PYTHON33_PKG):
	$(call _build_mac_package,$@)

$(SYS_PYTHON26_PKG): PYTHON_DIR=$(SYS_PYTHON26_DIR)
$(SYS_PYTHON26_PKG): BDIST_MPKG_DIR=$(PYTHON_DIR)/Extras/bin/
$(SYS_PYTHON26_PKG): CC=/usr/bin/gcc
$(SYS_PYTHON26_PKG): PYTHON=python
$(SYS_PYTHON26_PKG):
	$(call _build_mac_package,$@)
	$(call _fix_bundle_id,$@)

$(SYS_PYTHON27_PKG): PYTHON_DIR=$(SYS_PYTHON27_DIR)
$(SYS_PYTHON27_PKG): BDIST_MPKG_DIR=$(PYTHON_DIR)/Extras/bin/
$(SYS_PYTHON27_PKG): PYTHON=python
$(SYS_PYTHON27_PKG): CC=/usr/bin/gcc
$(SYS_PYTHON27_PKG):
	$(call _build_mac_package,$@)
	$(call _fix_bundle_id,$@)

_build_mac_package = \
	SYSROOT_PATH=$(SYSROOT_PATH)    \
	PORTAUDIO_PATH=$(PORTAUDIODIR)  \
	$(PYTHON_DIR)/bin/$(PYTHON) setup.py build --static-link && \
	$(BDIST_MPKG_DIR)/bdist_mpkg && \
	sleep 2 &&                      \
	mv $(DISTDIR)/$(notdir $(subst -mac-,-,$(subst -sys-,-,$(1)))) \
		$(1) &&                 \
	$(PYTHON_DIR)/bin/$(PYTHON) $(CLEAN_MPKG) -i \
	   $(1)/Contents/Packages/PyAudio-platlib-$(VERSION)-*/Contents/Info.plist  -o $(1)/Contents/Packages/PyAudio-platlib-$(VERSION)-*/Contents/Info.plist

# Change the default system python bundle ID to include .systemdefault
# so as to not conflict with the MacPython bundles (they are
# identically named).  PackageMaker will fail in mysterious ways if
# bundle IDs are not unique.

_fix_bundle_id = \
	sed -i .backup -e 's/org\.pythonmac/org\.pythonmac\.systemdefault/g' \
        $(1)/Contents/Packages/PyAudio-platlib-$(VERSION)-*/Contents/Info.plist


$(MPKG_INSTALLER): $(OUTPUT) $(SYS_PYTHON26_PKG) \
		   $(SYS_PYTHON27_PKG) $(MAC_PYTHON27_PKG) \
		   $(MAC_PYTHON32_PKG) $(MAC_PYTHON33_PKG)
	@echo "Making Meta Package"
	@$(PACKAGEMAKER) --doc $(PACKAGEDOC) --out $(MPKG_INSTALLER)

$(DMG): $(MPKG_INSTALLER)
	@rm -f $(OUTPUT)/$(VOLNAME_FILENAME).dmg
	@echo "Creating a DMG with $(SECTORS) sectors"

	@$(HDIUTIL) create -sectors $(SECTORS) -fs HFS+ \
		-volname $(VOLNAME) $(OUTPUT)/$(VOLNAME_FILENAME).dmg

	@$(HDIUTIL) attach $(OUTPUT)/$(VOLNAME_FILENAME).dmg

	@echo "Copying Data"
	@mkdir /Volumes/$(VOLNAME)/.packaging
	@cp $(PACKAGINGDIR)/$(MAC_SNAKEY) \
		/Volumes/$(VOLNAME)/.packaging/$(MAC_SNAKEY)
	@cp -r $(MPKG_INSTALLER) /Volumes/$(VOLNAME)/Install\ PyAudio.mpkg
	@SetFile -a E /Volumes/$(VOLNAME)/Install\ PyAudio.mpkg
	@cat $(PACKAGINGDIR)/setup_dmg | osascript

	@echo "Done Copying"
	@$(HDIUTIL) detach /Volumes/$(VOLNAME)

	cp $(OUTPUT)/$(VOLNAME_FILENAME).dmg $(OUTPUT)/$(VOLNAME_FILENAME).rw.dmg

	@echo "Resizing DMG"
	@$(HDIUTIL) resize -sectors \
		`$(HDIUTIL) resize $(OUTPUT)/$(VOLNAME_FILENAME).dmg | awk '{print $$1}'` $(OUTPUT)/$(VOLNAME_FILENAME).dmg

	@echo "Compressing DMG"
	@$(HDIUTIL) convert -imagekey zlib-level=9 -format UDZO \
		$(OUTPUT)/$(VOLNAME_FILENAME).dmg           \
		-o $(OUTPUT)/$(VOLNAME_FILENAME).tmp.dmg

	@mv $(OUTPUT)/$(VOLNAME_FILENAME).tmp.dmg \
	    $(OUTPUT)/$(VOLNAME_FILENAME).dmg

macosx: $(DMG)

######################################################################
# Win32
######################################################################

$(WIN32_PYTHON26_PKG): $(SRCFILES)
$(WIN32_PYTHON26_PKG): PYTHON=$(WIN32_PYTHON26)
$(WIN32_PYTHON26_PKG):
	$(call _build_win_package,$@)

$(WIN32_PYTHON27_PKG): $(SRCFILES)
$(WIN32_PYTHON27_PKG): PYTHON=$(WIN32_PYTHON27)
$(WIN32_PYTHON27_PKG):
	$(call _build_win_package,$@)

$(WIN32_PYTHON32_PKG): $(SRCFILES)
$(WIN32_PYTHON32_PKG): PYTHON=$(WIN32_PYTHON32)
$(WIN32_PYTHON32_PKG):
	$(call _build_win_package,$@)

$(WIN32_PYTHON33_PKG): $(SRCFILES)
$(WIN32_PYTHON33_PKG): PYTHON=$(WIN32_PYTHON33)
$(WIN32_PYTHON33_PKG):
	$(call _build_win_package,$@)

_build_win_package = \
	PACKAGING_PATH=$(PACKAGINGDIR)                              \
	PORTAUDIO_PATH=$(PORTAUDIODIR)                              \
	$(PYTHON) setup.py build -cmingw32 --static-link &&         \
	$(PYTHON) setup.py bdist_wininst --skip-build -t PyAudio    \
		--bitmap=$(PACKAGINGDIR)/win-background.bmp &&   \
	mv $(DISTDIR)/$(WIN32_PKG) $(1)

win32: $(OUTPUT) $(WIN32_PYTHON26_PKG) $(WIN32_PYTHON27_PKG) \
	$(WIN32_PYTHON32_PKG) $(WIN32_PYTHON33_PKG)

$(OUTPUT):
	@mkdir -p $(OUTPUT)
