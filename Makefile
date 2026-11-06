# Wing Commander (Kilrathi Saga, Win32) source reconstruction Makefile.
#
# This WC2 branch uses Microsoft Visual C++ 4.1 under wibo. Matching the
# compiler, its flags, and the linker input order is part of the recovery
# process: binary-comp checks the rebuilt executable against the original at
# the instruction and data-layout level.
#
# The toolchain identification is evidence-based; see docs/COMPILER.md. WC2's
# PE header reports linker 3.10.6038, exactly matching the MSVC 4.1 package.
#
# Common entry points:
#   make                 # build WC2.EXE
#   make WC2.EXE         # build WC2.EXE
#   make run             # build and launch in DREAMM
#   make report          # WC2 per-function similarity report on this branch
#   make order           # compilation-unit boundary hints
#   make verify          # primary recovery verification checklist
#   make progress        # reimplementation progress summary

# ---------------------------------------------------------------------------
# Original toolchain
# ---------------------------------------------------------------------------

WIBO = ./wibo
MSVC41_DIR = compilers/msvc41
MSVC41_STAMP = $(MSVC41_DIR)/.installed
CC = $(WIBO) $(MSVC41_DIR)/Bin/CL.EXE
LINK = $(WIBO) $(MSVC41_DIR)/Bin/LINK.EXE
LIBEXE = $(WIBO) $(MSVC41_DIR)/Bin/LIB.EXE

# MSVC expects Windows-style include/library search paths.  The recipes pass
# these through the host shell to wibo, so command-line /I paths use doubled
# backslashes while env vars keep normal Windows separators.
MSVC_INC = compilers\msvc41\Include;compilers\msvc41\SdkInclude
MSVC_LIB = compilers\msvc41\Lib

# Keep these flags synchronized with the recovered binary.  Changing optimizer
# or codegen flags will usually invalidate binary-comp comparisons.
#
# Evidence for each choice (see docs/COMPILER.md):
#   /MTd   static DEBUG multithreaded CRT.  The binary links LIBCMTD: dbgheap.c,
#          dbgrpt.c, dbgdel.cpp, _CrtCheckMemory, "DAMAGE: after %hs block",
#          "Detected memory leaks!", plus tidtable.c/_mtinit for the MT variant.
#   /Oy    frame pointers omitted.  Almost every game function reads arguments
#          as [esp+N] with no `push ebp` prologue.
#   NO /Gf string pooling.  Two byte-identical "DIBsetWholePalette   SetEntries"
#          literals exist at 0x0046b6e0 and 0x0046b71c, so identical strings
#          were NOT merged.
#   /GX is off for the core by default.  The one proven exception is
#          console.cpp: its debug-console new expression at 0x40FCD0 has a C++
#          unwind map and jumps to __CxxFrameHandler.  A target-specific flag
#          below reproduces it.
# Extra compiler flags for temporary diagnostic builds; empty for the reference
# build so binary-comp still sees the shipped code.  See the vport-debug target.
EXTRA_CFLAGS ?=

CFLAGS_COMMON = \
	/nologo \
	/c \
	/MTd \
	/I include \
	$(EXTRA_CFLAGS)

# WC1 used different optimizer settings for its game core and ix library. WC2
# does not: both areas have the unoptimized debug-build shape:
#
# Every ix function and the mapped WC2 game-core functions open with
#   `PUSH EBP / MOV EBP,ESP / PUSH EBX / PUSH ESI / PUSH EDI` -- saving all three
#   registers whether or not they are used -- spills intermediates to stack
#   temporaries such as [EBP-4], and jumps to one shared
#   `POP EDI / POP ESI / POP EBX / LEAVE / RET` epilogue.
#
# The MSVC 4.1 control improved the remapped report from 47.48% with WC1's
# optimized flags to 74.01% with /Od, while ix remains 99.13% similar.
CFLAGS_CORE = $(CFLAGS_COMMON) /Od /Oi
CFLAGS_IX   = $(CFLAGS_COMMON) /Od /Oi

# Default for anything not covered by a more specific rule.
CFLAGS = $(CFLAGS_CORE)

# The original was the IDE's Debug configuration: /DEBUG (which keeps every
# import thunk, /OPT:NOREF) without incremental linking, and no /ENTRY.
# Without /ENTRY LINK adds WinMainCRTStartup as an undefined symbol after
# reading every command-line object, so wincrt0.obj lands after the CRT
# members the game itself needs, at 0x00473E50.  The image's MISC and
# CodeView debug records name Debug/wc2.exe and C:\WCB\wc2\Debug\wc2.pdb.
LINKFLAGS = /nologo /SUBSYSTEM:WINDOWS /DEBUG /INCREMENTAL:NO \
	/PDB:$(OUT_DIR)/wc2.pdb /ALIGN:4096

# DDRAW.DLL and DSOUND.DLL are bound statically through the import table, so
# import libraries are required at link time.  The MSVC420 submodule already
# ships DDRAW.LIB/DSOUND.LIB and DDRAW.H/DSOUND.H, so no extra SDK is needed.
GAME_LIBPATH = $(MSVC_LIB)
#
# The import descriptors run DDRAW, WINMM, KERNEL32, USER32, GDI32, ADVAPI32,
# DSOUND: only the dsp library members, pulled after the CRT, need DirectSound,
# so its descriptor and thunks come from the second search pass.
GAME_LIBS = \
	ddraw.lib \
	dsound.lib \
	winmm.lib \
	kernel32.lib \
	user32.lib \
	gdi32.lib \
	advapi32.lib

# The ix code reached the link as five libraries.  The sound and streamer
# libraries precede the import libraries: their members sit before the first
# pass's import thunks at 0x0046DF08, and sample.cpp's ldexp is pulled after
# wincrt0.obj.  The dsp, LZO and file libraries are searched after LIBCMTD and
# OLDNAMES: their members follow the whole first-pass CRT at 0x00489990.
IX_LIBS_BEFORE_CRT = $(IX_SOUND_LIB) $(IX_STREAMER_LIB)
IX_LIBS_AFTER_CRT = $(IX_DSP_LIB) $(IX_LZO_LIB) $(IX_FILE_LIB)
CRT_LIBS = libcmtd.lib oldnames.lib

# ---------------------------------------------------------------------------
# Project inputs and generated outputs
# ---------------------------------------------------------------------------

TARGET = WC2.EXE
# LINK records this spelling in IMAGE_DEBUG_MISC, as in the retail image.
LINK_TARGET = Debug/wc2.exe
MAPFILE = WC2.map
OUT_DIR = out
LINKMAPFILE = $(OUT_DIR)/WC2.link.map
# PE header and three debug-directory entries: 1996-09-24 17:41:25 UTC.
ORIGINAL_TIMESTAMP = 0x32481D45

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

MODERN_EXE_SUFFIX :=
MODERN_PLATFORM_LIBS :=
ifneq (,$(filter MINGW% MSYS% CYGWIN%,$(UNAME_S)))
MODERN_EXE_SUFFIX := .exe
MODERN_PLATFORM_LIBS := -limm32
endif

# The native build uses the host compiler and never contributes objects to the
# MSVC 4.20 reference executable.
MODERN_OUT_DIR = out-modern
MODERN_TARGET = $(MODERN_OUT_DIR)/wc2-modern$(MODERN_EXE_SUFFIX)
MODERN_GUI_TARGET = $(MODERN_OUT_DIR)/wc2-modern-gui$(MODERN_EXE_SUFFIX)
MODERN_EMPTY :=
MODERN_SPACE := $(MODERN_EMPTY) $(MODERN_EMPTY)
# The port runs from the game's own directory: it chdir()s into gamedat/ and
# resolves every packet relative to that.  data/full holds only the reference
# executable, so pointing here at the installed tree is what makes run-modern
# find brief.pal and the rest of the resources.
MODERN_RUN_DIR ?= data/wc2-full
MODERN_ARGS ?=
SERIES ?= 1
MISSION ?= 0
MISSION_FLAGS ?=
MODERN_CUTSCENE_SERIES = 1 2 3 4 5 6 7 8 9 10 11 12 13
MODERN_CUTSCENE_JOBS ?= 4
MODERN_CUTSCENE_SELECTORS := $(filter-out v7-t3 v13-t3,\
	$(foreach series,$(MODERN_CUTSCENE_SERIES),\
		$(addprefix v$(series)-t,0 1 2 3)))

MODERN_CC ?= cc
MODERN_CXX ?= c++
MODERN_CMAKE ?= cmake
MODERN_SDL2_CONFIG ?= sdl2-config
MODERN_SDL_CFLAGS = $(shell \
	$(MODERN_SDL2_CONFIG) --cflags 2>/dev/null || \
	pkg-config --cflags sdl2 2>/dev/null)
MODERN_SDL_LIBS = $(shell \
	$(MODERN_SDL2_CONFIG) --libs 2>/dev/null || \
	pkg-config --libs sdl2 2>/dev/null)
MODERN_LZO_CFLAGS = $(shell pkg-config --cflags lzo2 2>/dev/null)
MODERN_LZO_INCLUDEDIR = $(shell pkg-config --variable=includedir lzo2 2>/dev/null)
MODERN_LZO_LIBS = $(shell pkg-config --libs lzo2 2>/dev/null)
# SDL_MAIN_HANDLED keeps SDL's headers from renaming main() to SDL_main on
# Windows, which needs SDL2main.a to supply a WinMain wrapper.  The port has
# its own main and calls SDL_SetMainReady() itself, so the rename is only a
# way to fail the link.
MODERN_CPPFLAGS = -DSDL_PORT=1 -DSDL_MAIN_HANDLED -Iinclude \
	$(MODERN_SDL_CFLAGS) \
	$(MODERN_LZO_CFLAGS) $(addprefix -I,$(MODERN_LZO_INCLUDEDIR))
# The reconstruction reproduces MSVC 4.1's tolerance for mismatched pointer
# and integer arguments; clang treats those as errors by default.  Demote them
# so the port compiles the same sources the reference build does.
# -fno-common so a global defined in two translation units is an error here
# too.  Apple's linker merges those tentative definitions and GNU ld does not,
# which is how a duplicate reached CI having built cleanly on macOS.
MODERN_CFLAGS ?= -O2 -std=c11 -fno-common -Wno-return-type \
	-Wno-return-mismatch -Wno-error=incompatible-pointer-types \
	-Wno-int-conversion
MODERN_CXXFLAGS ?= -O2 -std=c++11
MODERN_DEPFLAGS = -MMD -MP
MODERN_SECTION_FLAGS = -ffunction-sections -fdata-sections
# The game's packet structures are byte-packed, so a pointer member lands on an
# odd offset by design and every access to one trips UBSan's alignment check.
# aarch64 and x86-64 both perform those loads correctly; the rest of UBSan and
# all of ASan stay on.
MODERN_UBSAN_EXCLUDE = -fno-sanitize=alignment
# Developer and test builds retain runtime diagnostics.  Tagged release jobs
# explicitly select the optimized, uninstrumented host build.
ifeq ($(MODERN_RELEASE),1)
override MODERN_SANITIZER_FLAGS =
else
override MODERN_SANITIZER_FLAGS = -fsanitize=address,undefined \
	$(MODERN_UBSAN_EXCLUDE) -fno-omit-frame-pointer
endif
MODERN_DEAD_STRIP_DARWIN = -Wl,-dead_strip -Wl,-no_fixup_chains
MODERN_DEAD_STRIP_OTHER = -Wl,--gc-sections
MODERN_DEAD_STRIP_FLAGS = $(if $(filter Darwin,$(UNAME_S)),\
	$(MODERN_DEAD_STRIP_DARWIN),$(MODERN_DEAD_STRIP_OTHER))

# Where the retail executable lives. `make data/full/WC2.ORI.EXE` copies it out
# of the sibling analysis tree so this repo never has to vendor the binary.
ORIGINAL_EXE = data/full/WC2.ORI.EXE
ORIGINAL_SRC ?= ../releases/win32/WC2.EXE

VERIFY_CONFIG = config/binary-comp.json
CODE_DIR = code-full
CODE_EXPORT_STAMP = $(CODE_DIR)/.wc2-exported
EXPORT_ASM_FLAGS ?= --discover --max-functions 4096
# Optional verification dependency; install with pip as documented in README.md.
BINARY_COMP ?= binary-comp

VALUE_MIN_SIMILARITY ?= 80
STACK_LOCAL_VALUE_MIN_SIMILARITY ?= 90
STACK_LOCAL_VALUES_FLAGS ?= --no-offsets
VALUES_FLAGS ?=

# Address windows for the globals audit.  The developer/CRT boundary is at
# 0x00449100: the last ix assert anchor (sample.cpp) sits at 0x00449044 and the
# first CRT __FILE__ anchor (fclose.c) at 0x004492e0.  Data above the game's own
# statics belongs to the CRT and is not recovered game state.
GLOBALS_MISSING_MIN_ADDRESS = 0x00465000
GLOBALS_MISSING_MAX_ADDRESS = 0x004751ff

# Global definitions are migrating back to their original compilation units.
# binary-comp currently accepts one definition source, so generate a parser-only
# manifest from globals.c and the declaration block at the top of each owner.
GLOBALS_DISTRIBUTED_SOURCES = \
	$(filter-out src/globals.c %.asm,$(SRCS_ORDERED_CORE)) \
	src/ix/streamer.cpp src/ix/dsp.cpp src/ix/dsps.cpp \
	src/ix/mixer.cpp src/ix/dspv.cpp src/ix/system.cpp
GLOBALS_DEFINITION_SOURCES = src/globals.c $(GLOBALS_DISTRIBUTED_SOURCES)
GLOBALS_AUDIT_SOURCE = $(OUT_DIR)/globals-audit.c
GLOBALS_AUDIT_DIR = $(OUT_DIR)/globals-audit
GLOBALS_AUDIT_MANIFEST = $(GLOBALS_AUDIT_DIR)/sources.txt

# ---------------------------------------------------------------------------
# Host platform and DREAMM runtime
# ---------------------------------------------------------------------------
#
# DREAMM is the only supported way to run the rebuilt executable.  Wine is not
# used: the Kilrathi Saga port is a 1996 Win32 binary that drives DirectDraw and
# DirectSound directly and expects a real Windows 95 environment, which is what
# DREAMM emulates.  Wine's own reimplementation of those APIs changes exactly
# the behaviour this project is trying to observe.
#
# DREAMM is downloaded on demand into .dreamm/ so the repository does not need
# to vendor platform-specific runtime binaries.

DREAMM_DIR = .dreamm
DREAMM_VERSION = 4.0x47
DREAMM_BASE_URL = https://dreamm.aarongiles.com/releases/4.0x

# 8-bit, not 16: the DIB layer creates a DirectDraw palette and pushes entries
# into it (DIBcascade -> CreatePalette, DIBsetPalette/DIBramPalette ->
# SetEntries).  Those calls only succeed against a palettized primary surface,
# so the game expects an 8-bit display mode; in 16-bit it takes the
# DIBerror("DIBmakeDIB   CreatePalette") path.  Override with DREAMM_PROPS=.
DREAMM_PROPS ?= -prop winres=640x480x8

ifeq ($(UNAME_S),Darwin)
DREAMM_ARCHIVE = dreamm-$(DREAMM_VERSION)-macos.dmg
DREAMM_BIN = $(DREAMM_DIR)/DREAMM.app/Contents/MacOS/dreamm
else
ifeq ($(UNAME_M),aarch64)
DREAMM_ARCHIVE = dreamm-$(DREAMM_VERSION)-linux-arm64.tgz
else
DREAMM_ARCHIVE = dreamm-$(DREAMM_VERSION)-linux-x64.tgz
endif
DREAMM_BIN = $(DREAMM_DIR)/dreamm
endif
DREAMM = $(CURDIR)/$(DREAMM_BIN)
DREAMM_STAMP = $(DREAMM_DIR)/.$(DREAMM_ARCHIVE).stamp

# The game runs out of the installed data directory.  C: is mounted writable
# from data/wc2-full/hd because DREAMM otherwise discards every write to C:, and the
# game rewrites INSTALL.DAT and its save slots.
#
# The disc is mounted at D: when present: the binary really does look for it
# (LocateStreamsDirOnDisc, FindCdRomDriveByVolumeLabel, PromptInsertCorrectCd),
# and the streaming music lives there. Point ISO at an image or a directory.
RUN_DIR = data/wc2-full
# Any disc image dropped in data/ or data/wc2-full/ is picked up automatically.
# The recipes cd into RUN_DIR, so the mount path is made relative to it.
ISO ?= $(firstword $(wildcard data/*.iso data/*.ISO data/wc2-full/*.iso data/wc2-full/*.ISO))
DREAMM_MOUNTS = -mount rw:C=hd \
                $(if $(ISO),-mount d=$(patsubst $(RUN_DIR)/%,%,$(patsubst data/%,../%,$(ISO))))

# ---------------------------------------------------------------------------
# Source order
# ---------------------------------------------------------------------------
#
# The object link order fixes addresses and therefore every binary comparison.
# MSVC emits functions in source order and the linker concatenates objects in
# command-line order, so address order == link order.
#
# Game-core link order.  Every core object is on the command line, so object
# order is address order.  The order and each object's extent are recovered
# from the retail image (docs/ORDER.md): the compiler starts each object's
# code on a 16-byte boundary and the linker fills the gap with INT3, the
# string literals of each object form one band in .data in the same order,
# and the DOS release's modules map one-to-one onto these objects with the
# same function order.
SRCS_ORDERED_CORE = \
	src/config.c \
	src/damage.c \
	src/joypres.c \
	src/palalloc.c \
	src/taperec.c \
	src/raster.asm \
	src/intro.c \
	src/hotspot.c \
	src/campaign.c \
	src/timer.c \
	src/geom.c \
	src/iffload.c \
	src/disk.c \
	src/screen.c \
	src/console.cpp \
	src/ship.c \
	src/palette.c \
	src/fields.c \
	src/titlemnu.c \
	src/mousepos.c \
	src/hugemove.c \
	src/spc.c \
	src/smart.c \
	src/textfmt.c \
	src/nearheap.c \
	src/input.c \
	src/orders.c \
	src/fade.c \
	src/auto.c \
	src/joycal.c \
	src/viewrect.c \
	src/sound.c \
	src/jump.c \
	src/gr.c \
	src/stream.cpp \
	src/campglob.c \
	src/logic.c \
	src/suffix.c \
	src/gameflow.c \
	src/pan.c \
	src/text.c \
	src/personnel.c \
	src/clock.c \
	src/mono.c \
	src/message.c \
	src/eject.c \
	src/cockpt.c \
	src/target.c \
	src/loadmisn.c \
	src/version.c \
	src/fl.c \
	src/brains.c \
	src/grdriver.c \
	src/pload.c \
	src/destroy.c \
	src/fstrcat.c \
	src/replay.c \
	src/shpext.c \
	src/vpfree.c \
	src/comm.c \
	src/wipe.c \
	src/fleetvw.c \
	src/mission.c \
	src/midscene.c \
	src/packet.c \
	src/joystick.c \
	src/speech.c \
	src/nav.c \
	src/kbmouse.c \
	src/hexnum.c \
	src/music.c \
	src/palfade.c \
	src/pktsize.c \
	src/strwidth.c \
	src/parsenum.c \
	src/winmain.c \
	src/carrier.c \
	src/fltsetup.c \
	src/usedcols.c \
	src/iffindex.c \
	src/memapi.c \
	src/optmenu.c \
	src/datafile.c \
	src/starfld.c \
	src/debug.cpp \
	src/getmode.c \
	src/startup.c \
	src/vpalloc.c \
	src/dib.cpp \
	src/turret.c \
	src/select.c \
	src/eventmgr.c \
	src/waitkey.c \
	src/main.c \
	src/death.c \
	src/memfree.c \
	src/resread.c \
	src/globals.c

# Each ix library's members, in the order LINK pulls them.
SRCS_IX_SOUND = \
	src/ix/system.cpp \
	src/ix/sound.cpp \
	src/ix/sample.cpp
SRCS_IX_STREAMER = \
	src/ix/streamer.cpp \
	src/ix/thread.cpp
SRCS_IX_DSP = \
	src/ix/dsp.cpp \
	src/ix/dspv.cpp \
	src/ix/dsps.cpp \
	src/ix/mixer.cpp \
	src/ix/dspmix.asm
SRCS_IX_LZO = \
	src/ix/lzo1x.cpp
SRCS_IX_FILE = \
	src/ix/ixfile.cpp

SRCS_ORDERED_IX = $(SRCS_IX_SOUND) $(SRCS_IX_STREAMER) $(SRCS_IX_DSP) \
	$(SRCS_IX_LZO) $(SRCS_IX_FILE)

SRCS_ORDERED = $(SRCS_ORDERED_CORE) $(SRCS_ORDERED_IX)

ALL_SRCS = $(wildcard src/*.c) $(wildcard src/*.cpp) $(wildcard src/*.asm) \
           $(wildcard src/ix/*.cpp) $(wildcard src/ix/*.asm)
SRCS = $(filter $(ALL_SRCS), $(SRCS_ORDERED)) \
       $(filter-out $(SRCS_ORDERED), $(ALL_SRCS))

# Every unit sees the shared declarations, and MSVC 4.1's codegen is
# sensitive to them: adding a prototype can move an operand or a stack
# slot in an unrelated file.  Rebuild on any header change so a stale
# object never gets compared against the original.
GAME_HEADERS = $(wildcard include/*.h) $(wildcard src/ix/*.h)

objects_of = $(patsubst src/%,$(OUT_DIR)/%, \
         $(patsubst %.c,%.obj, $(patsubst %.cpp,%.obj, \
         $(patsubst %.asm,%.obj, $(1)))))

OBJS = $(call objects_of,$(SRCS))

# Everything except the ix library members is an object on the command line.
LINK_OBJS = $(call objects_of,$(filter-out $(SRCS_ORDERED_IX),$(SRCS)))

IX_SOUND_LIB = $(OUT_DIR)/ix/sound.lib
IX_STREAMER_LIB = $(OUT_DIR)/ix/streamer.lib
IX_DSP_LIB = $(OUT_DIR)/ix/dsp.lib
IX_LZO_LIB = $(OUT_DIR)/ix/lzo.lib
IX_FILE_LIB = $(OUT_DIR)/ix/file.lib
IX_LIBS = $(IX_LIBS_BEFORE_CRT) $(IX_LIBS_AFTER_CRT)

# Platform-neutral recovered units: every core C and C++ unit.  sound.c keeps
# its C file name but is compiled as C++ (see its Win32 rule).  The assembled
# raster library has portable counterparts in src/sdl/raster.c.
MODERN_GAMEPLAY_SRCS = \
	$(filter-out src/sound.c,$(filter %.c,$(SRCS_ORDERED_CORE)))

MODERN_GAMEPLAY_CXX_SRCS = \
	$(filter %.cpp,$(SRCS_ORDERED_CORE))

MODERN_GAMEPLAY_CXX_C_SRCS = \
	src/sound.c

MODERN_IX_SRCS = \
	src/ix/streamer.cpp \
	src/ix/thread.cpp \
	src/ix/dsp.cpp \
	src/ix/dsps.cpp \
	src/ix/mixer.cpp \
	src/ix/dspv.cpp \
	src/ix/system.cpp \
	src/ix/sound.cpp \
	src/ix/sample.cpp \
	src/ix/lzo1x.cpp \
	src/ix/ixfile.cpp

MODERN_BASE_HOST_SRCS = \
	src/sdl/compat.c \
	src/sdl/input.c \
	src/sdl/resources.c \
	src/sdl/registry.c \
	src/sdl/thread.c \
	src/sdl/timer.c \
	src/sdl/video_state.c
MODERN_GAME_HOST_SRCS = \
	src/sdl/audio.c \
	src/sdl/cutscene.c \
	src/sdl/difficulty.c \
	src/sdl/events.c \
	src/sdl/gl_renderer.c \
	src/sdl/joystick.c \
	src/sdl/music.c \
	src/sdl/raster.c \
	src/sdl/video.c
MODERN_GAME_HOST_CXX_SRCS = \
	src/sdl/originfx.cpp \
	src/sdl/wave.cpp
MODERN_YMFM_SRCS = \
	third_party/ymfm/ymfm_adpcm.cpp \
	third_party/ymfm/ymfm_opl.cpp \
	third_party/ymfm/ymfm_pcm.cpp
MODERN_LAUNCHER_SRC = src/sdl/launcher.c
MODERN_GUI_SOURCE_DIR = src/sdl/slint
MODERN_GUI_BUILD_DIR = $(MODERN_OUT_DIR)/slint-gui
MODERN_GUI_BUILD_TYPE ?= Release
MODERN_GUI_SRCS = \
	$(MODERN_GUI_SOURCE_DIR)/CMakeLists.txt \
	$(MODERN_GUI_SOURCE_DIR)/launcher.cpp \
	$(MODERN_GUI_SOURCE_DIR)/launcher.slint \
	$(MODERN_GUI_SOURCE_DIR)/launcher_api.h \
	$(MODERN_GUI_SOURCE_DIR)/slint_static.h \
	$(MODERN_GUI_SOURCE_DIR)/wc2_logo.h

MODERN_GAMEPLAY_OBJS = \
	$(patsubst src/%.c,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_GAMEPLAY_SRCS)) \
	$(patsubst src/%.cpp,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_GAMEPLAY_CXX_SRCS)) \
	$(patsubst src/%.c,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_GAMEPLAY_CXX_C_SRCS))
MODERN_IX_OBJS = \
	$(patsubst src/%.cpp,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_IX_SRCS))
MODERN_BASE_HOST_OBJS = $(patsubst src/%.c,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_BASE_HOST_SRCS))
MODERN_YMFM_OBJS = $(patsubst %.cpp,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_YMFM_SRCS))
MODERN_GAME_HOST_OBJS = \
	$(patsubst src/%.c,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_GAME_HOST_SRCS)) \
	$(patsubst src/%.cpp,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_GAME_HOST_CXX_SRCS)) \
	$(MODERN_YMFM_OBJS)
MODERN_STATIC_GUI_LAUNCHER_OBJ = \
	$(MODERN_OUT_DIR)/obj/sdl/launcher_gui.o
MODERN_TEST_LAUNCHER_OBJ = $(MODERN_OUT_DIR)/obj/sdl/launcher_test.o
MODERN_GUI_GAME_OBJS = \
	$(MODERN_STATIC_GUI_LAUNCHER_OBJ) \
	$(MODERN_BASE_HOST_OBJS) \
	$(MODERN_GAME_HOST_OBJS) \
	$(MODERN_GAMEPLAY_OBJS) \
	$(MODERN_IX_OBJS)
# MSYS2 does not convert each path in a semicolon-separated CMake argument.
# Native Windows CMake and Ninja need drive-qualified object paths.
MODERN_GUI_CMAKE_OBJECTS = $(abspath $(MODERN_GUI_GAME_OBJS))
ifneq (,$(filter MINGW% MSYS% CYGWIN%,$(UNAME_S)))
MODERN_GUI_CMAKE_OBJECTS = $(shell cygpath -m $(abspath $(MODERN_GUI_GAME_OBJS)))
endif
MODERN_EVENT_HOST_OBJS = \
	$(MODERN_OUT_DIR)/obj/sdl/events.o \
	$(MODERN_OUT_DIR)/obj/sdl/video.o
MODERN_VIDEO_HOST_OBJS = \
	$(MODERN_OUT_DIR)/obj/dib.o \
	$(MODERN_OUT_DIR)/obj/sdl/video.o
MODERN_LAUNCHER_OBJ = $(patsubst src/%.c,$(MODERN_OUT_DIR)/obj/%.o,$(MODERN_LAUNCHER_SRC))
MODERN_INPUT_CORE_OBJS = \
	$(MODERN_OUT_DIR)/obj/eventmgr.o \
	$(MODERN_OUT_DIR)/obj/globals.o \
	$(MODERN_OUT_DIR)/obj/sysinput.o
MODERN_BASE_C_TEST_NAMES = sdl_compat_smoke sdl_crt_compat sdl_text_compat \
	sdl_dos_resources sdl_input_compat sdl_event_compat sdl_video_compat \
	sdl_ship_evasion sdl_adaptive_difficulty sdl_turret_guns sdl_ship_exhaust \
	sdl_capital_weapons sdl_music_paths sdl_mission_objectives sdl_cutscene_input
MODERN_BASE_C_TEST_BINS = $(addsuffix $(MODERN_EXE_SUFFIX),\
	$(addprefix $(MODERN_OUT_DIR)/tests/,$(MODERN_BASE_C_TEST_NAMES)))
MODERN_RUNTIME_TEST_BIN = $(MODERN_OUT_DIR)/tests/sdl_runtime_safety$(MODERN_EXE_SUFFIX)
MODERN_CXX_TEST_BIN = $(MODERN_OUT_DIR)/tests/sdl_ix_compat_smoke$(MODERN_EXE_SUFFIX)
MODERN_ADLIB_TEST_BIN = $(MODERN_OUT_DIR)/tests/sdl_dos_adlib$(MODERN_EXE_SUFFIX)
MODERN_GL_VIDEO_TEST_BIN = $(MODERN_OUT_DIR)/tests/sdl_gl_renderer$(MODERN_EXE_SUFFIX)
MODERN_HEADLESS_TEST_BINS = \
	$(MODERN_BASE_C_TEST_BINS) \
	$(MODERN_RUNTIME_TEST_BIN) \
	$(MODERN_CXX_TEST_BIN) \
	$(MODERN_ADLIB_TEST_BIN)
MODERN_TEST_BINS = $(MODERN_HEADLESS_TEST_BINS) $(MODERN_GL_VIDEO_TEST_BIN)
MODERN_DEPFILES = \
	$(MODERN_GAMEPLAY_OBJS:.o=.d) \
	$(MODERN_IX_OBJS:.o=.d) \
	$(MODERN_BASE_HOST_OBJS:.o=.d) \
	$(MODERN_GAME_HOST_OBJS:.o=.d) \
	$(MODERN_LAUNCHER_OBJ:.o=.d) \
	$(MODERN_STATIC_GUI_LAUNCHER_OBJ:.o=.d) \
	$(MODERN_TEST_LAUNCHER_OBJ:.o=.d) \
	$(addsuffix .d,$(addprefix $(MODERN_OUT_DIR)/tests/,$(MODERN_BASE_C_TEST_NAMES))) \
	$(MODERN_OUT_DIR)/tests/sdl_gl_renderer.d \
	$(MODERN_OUT_DIR)/tests/sdl_runtime_safety.d \
	$(MODERN_OUT_DIR)/tests/sdl_ix_compat_smoke.d \
	$(MODERN_OUT_DIR)/tests/sdl_dos_adlib.d

# ---------------------------------------------------------------------------
# Build targets and tool bootstrap
# ---------------------------------------------------------------------------

all: $(TARGET)

build: $(TARGET)

# Naming parity with the sibling project, which has separate full/demo builds.
# WC1 shipped no demo, so there is only one target here and the `*-demo`
# counterparts (build-demo, report-demo, seh-demo, compare-demo, run-demo,
# progress-demo) intentionally do not exist.
build-full: $(TARGET)

# The native port is deliberately built in a separate output tree.  It must
# never supply objects to the assembly-comparison target above.
modern: $(MODERN_TARGET)

# Slint is kept out of the normal native build dependency set. This target
# links it statically into a separate executable that opens the launcher by
# default without arguments, or explicitly with --gui alongside other options.
modern-gui: $(MODERN_GUI_TARGET)

# Slint enables full Rust LTO upstream; the small launcher does not need its
# substantial clean-build cost.
$(MODERN_GUI_TARGET): $(MODERN_GUI_GAME_OBJS) $(MODERN_GUI_SRCS) Makefile
	@command -v $(MODERN_CMAKE) >/dev/null 2>&1 || { \
		echo "CMake 3.21 or newer is required for modern-gui." >&2; \
		exit 1; \
	}
	@command -v cargo >/dev/null 2>&1 || { \
		echo "Rust 1.88 or newer is required to build Slint." >&2; \
		exit 1; \
	}
	$(MODERN_CMAKE) -S $(MODERN_GUI_SOURCE_DIR) \
		-B $(MODERN_GUI_BUILD_DIR) \
		-DCMAKE_CXX_COMPILER="$(MODERN_CXX)" \
		-DCMAKE_BUILD_TYPE=$(MODERN_GUI_BUILD_TYPE) \
		-DWC2_GAME_OBJECTS="$(subst $(MODERN_SPACE),;,$(MODERN_GUI_CMAKE_OBJECTS))" \
		-DWC2_GAME_SANITIZERS=$(if $(MODERN_SANITIZER_FLAGS),ON,OFF) \
		-DWC2_GUI_OUTPUT_DIRECTORY=$(abspath $(MODERN_OUT_DIR))
	CARGO_PROFILE_RELEASE_LTO=false \
		$(MODERN_CMAKE) --build $(MODERN_GUI_BUILD_DIR) \
		--config $(MODERN_GUI_BUILD_TYPE) --target wc2-modern-gui
	@test -s $@

modern-check-deps:
	@if test -z "$(strip $(MODERN_SDL_CFLAGS))" || \
	   test -z "$(strip $(MODERN_SDL_LIBS))" || \
	   test -z "$(strip $(MODERN_LZO_LIBS))"; then \
		echo "SDL2 or LZO2 development files were not found." >&2; \
		echo "Install SDL2 and LZO2 development files." >&2; \
		exit 1; \
	fi

$(MODERN_STATIC_GUI_LAUNCHER_OBJ): MODERN_LAUNCHER_CPPFLAGS = -DWC2_STATIC_GUI=1
$(MODERN_TEST_LAUNCHER_OBJ): MODERN_LAUNCHER_CPPFLAGS = -DWC2_LAUNCHER_NO_MAIN=1
$(MODERN_STATIC_GUI_LAUNCHER_OBJ) $(MODERN_TEST_LAUNCHER_OBJ): \
		$(MODERN_LAUNCHER_SRC) Makefile | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CC) $(MODERN_CPPFLAGS) $(MODERN_LAUNCHER_CPPFLAGS) $(MODERN_CFLAGS) \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -c $< -o $@

$(MODERN_OUT_DIR)/obj/%.o: src/%.c | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CC) $(MODERN_CPPFLAGS) $(MODERN_CFLAGS) \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -c $< -o $@

$(MODERN_OUT_DIR)/obj/%.o: src/%.cpp | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CXX) $(MODERN_CPPFLAGS) -Isrc/ix $(MODERN_CXXFLAGS) \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -c $< -o $@

$(MODERN_OUT_DIR)/obj/sdl/originfx.o: src/sdl/originfx.cpp | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CXX) $(MODERN_CPPFLAGS) -Ithird_party/ymfm \
		$(MODERN_CXXFLAGS) -std=c++14 \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -c $< -o $@

$(MODERN_OUT_DIR)/obj/third_party/ymfm/%.o: third_party/ymfm/%.cpp | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CXX) $(MODERN_CPPFLAGS) -Ithird_party/ymfm \
		$(MODERN_CXXFLAGS) -std=c++14 \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -c $< -o $@

$(MODERN_OUT_DIR)/obj/sound.o: src/sound.c | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CXX) $(MODERN_CPPFLAGS) -Isrc/ix $(MODERN_CXXFLAGS) \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -x c++ -c $< -o $@

# Keep the recovered functions intact while the native objects enter the SDL
# positional-audio bridge.
$(MODERN_OUT_DIR)/obj/music.o: MODERN_CPPFLAGS += \
	-DSdlUsingOriginFxSoundEffects=SdlHandlesGameSoundEffects
$(MODERN_OUT_DIR)/obj/music.o: Makefile
$(MODERN_OUT_DIR)/obj/sound.o: MODERN_CPPFLAGS += \
	-Dix_system_new_sound=SdlNewWaveSound
$(MODERN_OUT_DIR)/obj/sound.o: Makefile

$(MODERN_OUT_DIR)/tests/%.o: tests/%.c | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CC) $(MODERN_CPPFLAGS) $(MODERN_TEST_CPPFLAGS) $(MODERN_CFLAGS) \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -c $< -o $@

$(MODERN_OUT_DIR)/tests/%.o: tests/%.cpp | modern-check-deps
	@mkdir -p $(dir $@)
	$(MODERN_CXX) $(MODERN_CPPFLAGS) -Isrc/ix $(MODERN_CXXFLAGS) \
		$(MODERN_SECTION_FLAGS) $(MODERN_SANITIZER_FLAGS) \
		$(MODERN_DEPFLAGS) -c $< -o $@

$(MODERN_OUT_DIR)/tests/sdl_compat_smoke.o: MODERN_TEST_CPPFLAGS = -DANALYSIS_BUILD=1
$(MODERN_OUT_DIR)/tests/sdl_gl_renderer.o: MODERN_TEST_CPPFLAGS = -Isrc/sdl
$(MODERN_OUT_DIR)/tests/sdl_video_compat.o: MODERN_TEST_CPPFLAGS = -Isrc/sdl

$(MODERN_TARGET): \
		$(MODERN_LAUNCHER_OBJ) \
		$(MODERN_BASE_HOST_OBJS) \
		$(MODERN_GAME_HOST_OBJS) \
		$(MODERN_GAMEPLAY_OBJS) \
		$(MODERN_IX_OBJS)
	@mkdir -p $(dir $@)
	$(MODERN_CXX) $(MODERN_CXXFLAGS) $(MODERN_SANITIZER_FLAGS) \
		$^ $(MODERN_SDL_LIBS) $(MODERN_LZO_LIBS) \
		$(MODERN_PLATFORM_LIBS) \
		$(MODERN_DEAD_STRIP_FLAGS) -o $@

# These checks supply their own main, but also reach the launcher's cutscene-
# exit callback through the resource objects.
$(MODERN_OUT_DIR)/tests/sdl_crt_compat$(MODERN_EXE_SUFFIX) \
$(MODERN_OUT_DIR)/tests/sdl_cutscene_input$(MODERN_EXE_SUFFIX): $(MODERN_TEST_LAUNCHER_OBJ)

# The host compatibility layer reaches into the game core for packet loads and
# into the event pump for key waits, so these link the same objects the heavier
# checks do.
$(MODERN_BASE_C_TEST_BINS): $(MODERN_OUT_DIR)/tests/%$(MODERN_EXE_SUFFIX): \
		$(MODERN_OUT_DIR)/tests/%.o \
		$(MODERN_BASE_HOST_OBJS) \
		$(MODERN_GAME_HOST_OBJS) \
		$(MODERN_GAMEPLAY_OBJS) \
		$(MODERN_IX_OBJS)
	$(MODERN_CXX) $(MODERN_CXXFLAGS) $(MODERN_SANITIZER_FLAGS) \
		$^ $(MODERN_SDL_LIBS) $(MODERN_LZO_LIBS) \
		$(MODERN_DEAD_STRIP_FLAGS) -o $@

$(MODERN_RUNTIME_TEST_BIN): \
		$(MODERN_OUT_DIR)/tests/sdl_runtime_safety.o \
		$(MODERN_BASE_HOST_OBJS) \
		$(MODERN_GAME_HOST_OBJS) \
		$(MODERN_GAMEPLAY_OBJS) \
		$(MODERN_IX_OBJS)
	$(MODERN_CXX) $(MODERN_CXXFLAGS) $(MODERN_SANITIZER_FLAGS) \
		$^ $(MODERN_SDL_LIBS) $(MODERN_LZO_LIBS) \
		$(MODERN_DEAD_STRIP_FLAGS) -o $@

$(MODERN_GL_VIDEO_TEST_BIN): \
		$(MODERN_OUT_DIR)/tests/sdl_gl_renderer.o \
		$(MODERN_BASE_HOST_OBJS) \
		$(MODERN_GAME_HOST_OBJS) \
		$(MODERN_GAMEPLAY_OBJS) \
		$(MODERN_IX_OBJS)
	$(MODERN_CXX) $(MODERN_CXXFLAGS) $(MODERN_SANITIZER_FLAGS) \
		$^ $(MODERN_SDL_LIBS) $(MODERN_LZO_LIBS) \
		$(MODERN_DEAD_STRIP_FLAGS) -o $@

$(MODERN_CXX_TEST_BIN): \
		$(MODERN_OUT_DIR)/tests/sdl_ix_compat_smoke.o \
		$(MODERN_BASE_HOST_OBJS) \
		$(MODERN_GAME_HOST_OBJS) \
		$(MODERN_GAMEPLAY_OBJS) \
		$(MODERN_IX_OBJS)
	$(MODERN_CXX) $(MODERN_CXXFLAGS) $(MODERN_SANITIZER_FLAGS) \
		$^ $(MODERN_SDL_LIBS) $(MODERN_LZO_LIBS) \
		$(MODERN_DEAD_STRIP_FLAGS) -o $@

# globals.o carries the OriginFX sound-effect records the retail check plays.
$(MODERN_ADLIB_TEST_BIN): \
		$(MODERN_OUT_DIR)/tests/sdl_dos_adlib.o \
		$(MODERN_BASE_HOST_OBJS) \
		$(MODERN_OUT_DIR)/obj/globals.o \
		$(MODERN_OUT_DIR)/obj/sdl/originfx.o \
		$(MODERN_YMFM_OBJS)
	$(MODERN_CXX) $(MODERN_CXXFLAGS) $(MODERN_SANITIZER_FLAGS) \
		$^ $(MODERN_SDL_LIBS) $(MODERN_DEAD_STRIP_FLAGS) -o $@

modern-test: modern
	@echo "Running $(MODERN_TARGET) --check"
	@SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy $(MODERN_TARGET) --check
	@echo "Running $(MODERN_TARGET) --balanced-difficulty --check"
	@SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
		$(MODERN_TARGET) --balanced-difficulty --check
	@echo "Running $(MODERN_TARGET) --ega --check"
	@SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
		$(MODERN_TARGET) --ega --check

modern-test-full: $(MODERN_TARGET) $(MODERN_TEST_BINS)
	@set -e; for test_bin in $(MODERN_HEADLESS_TEST_BINS); do \
		echo "Running $$test_bin"; \
		SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$$test_bin"; \
	done
	@echo "Running $(MODERN_GL_VIDEO_TEST_BIN)"
	@SDL_AUDIODRIVER=dummy $(MODERN_GL_VIDEO_TEST_BIN); \
	status=$$?; \
	if test $$status -eq 77; then \
		echo "GL renderer test skipped: no OpenGL display"; \
	elif test $$status -ne 0; then \
		exit $$status; \
	fi
	@echo "Running $(MODERN_TARGET) --check"
	@SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy $(MODERN_TARGET) --check
	@echo "Running $(MODERN_TARGET) --balanced-difficulty --check"
	@SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
		$(MODERN_TARGET) --balanced-difficulty --check
	@echo "Running $(MODERN_TARGET) --ega --check"
	@SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
		$(MODERN_TARGET) --ega --check

run-modern: modern
	@case "$(MODERN_RUN_DIR)" in \
		/*) modern_run_dir="$(MODERN_RUN_DIR)" ;; \
		*) modern_run_dir="$(CURDIR)/$(MODERN_RUN_DIR)" ;; \
	esac; \
	test -d "$$modern_run_dir" || { \
		echo "Modern run directory does not exist: $$modern_run_dir" >&2; \
		exit 1; \
	}; \
	cd "$$modern_run_dir" && "$(CURDIR)/$(MODERN_TARGET)" $(MODERN_ARGS)

run-modern-gui: modern-gui
	@case "$(MODERN_RUN_DIR)" in \
		/*) modern_run_dir="$(MODERN_RUN_DIR)" ;; \
		*) modern_run_dir="$(CURDIR)/$(MODERN_RUN_DIR)" ;; \
	esac; \
	test -d "$$modern_run_dir" || { \
		echo "Modern run directory does not exist: $$modern_run_dir" >&2; \
		exit 1; \
	}; \
	cd "$$modern_run_dir" && \
		"$(CURDIR)/$(MODERN_GUI_TARGET)" --gui $(MODERN_ARGS)

# The SDL2 host recognizes the compressed resources in an installed DOS copy
# and plays its OriginFX music and synthesized effects through an embedded
# YM3812 emulator.
run-modern-dos: MODERN_RUN_DIR = data/dos
run-modern-dos: run-modern

# The original startup has a hidden direct-flight path selected by the ordered
# tokens "Origin vN tN e".  The modern build forwards these through the recovered
# WC2 option loader just as a native argument vector would be handled.
# The option loader exposes one fewer token than it reads, so retain a sentinel.
run-modern-mission: MODERN_ARGS = Origin v$(SERIES) t$(MISSION) e \
	$(MISSION_FLAGS) ignored
run-modern-mission: run-modern

# Run the campaign cutscene selected by the original developer arguments.  The
# SDL host invokes the campaign VM directly and exits when it returns.
run-modern-cutscene: MODERN_ARGS = --cutscene-only Origin v$(SERIES) \
	t$(MISSION) ignored
run-modern-cutscene: run-modern

# MODULE.000 has four mission slots per series.  The base campaign uses every
# slot in series 1-13 except the empty fourth slots in series 7 and 13.
modern-test-cutscenes: modern
	+@status=0; \
	$(MAKE) --no-print-directory --keep-going \
		-j$(MODERN_CUTSCENE_JOBS) \
		$(addprefix modern-test-cutscene-,$(MODERN_CUTSCENE_SELECTORS)) || \
		status=$$?; \
	if test $$status -ne 0; then \
		echo "Cutscene sanitizer sweep failed; inspect the logs above." >&2; \
		exit $$status; \
	fi; \
	echo "Cutscene sanitizer sweep passed: 50 sequences."

modern-test-cutscene-%: modern
	@selector="$*"; \
	series=$${selector#v}; \
	mission=$${series##*-t}; \
	series=$${series%%-t*}; \
	case "$(MODERN_RUN_DIR)" in \
		/*) modern_run_dir="$(MODERN_RUN_DIR)" ;; \
		*) modern_run_dir="$(CURDIR)/$(MODERN_RUN_DIR)" ;; \
	esac; \
	test -d "$$modern_run_dir" || { \
		echo "Modern run directory does not exist: $$modern_run_dir" >&2; \
		exit 1; \
	}; \
	log_dir="$(CURDIR)/$(MODERN_OUT_DIR)/cutscene-asan"; \
	mkdir -p "$$log_dir"; \
	log="$$log_dir/$$selector.log"; \
	echo "Running cutscene $$selector"; \
	cd "$$modern_run_dir" && \
		SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
		ASAN_OPTIONS=halt_on_error=1:abort_on_error=1 \
		UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
		"$(CURDIR)/$(MODERN_TARGET)" --cutscene-only \
		Origin "v$$series" "t$$mission" ignored \
		>"$$log" 2>&1; \
	status=$$?; \
	if test $$status -ne 0 || \
		grep -Eq 'ERROR: AddressSanitizer|runtime error:' "$$log"; then \
		echo "Cutscene $$selector failed; see $$log" >&2; \
		grep -E 'ERROR: AddressSanitizer|runtime error:|SUMMARY: (AddressSanitizer|UndefinedBehaviorSanitizer)' \
			"$$log" >&2 || tail -n 20 "$$log" >&2; \
		exit 1; \
	fi; \
	echo "Passed cutscene $$selector"

-include $(MODERN_DEPFILES)

ifeq ($(UNAME_S),Linux)
WIBO_PRESET = release64-clang
WIBO_BIN = wibo-src/build/release64-clang/wibo
else
WIBO_PRESET = release-macos
WIBO_BIN = wibo-src/build/release/wibo
endif

$(WIBO):
	cd wibo-src && cmake --preset $(WIBO_PRESET) && cmake --build --preset $(WIBO_PRESET)
	ln -sf $(WIBO_BIN) $@

# The decomp.me MSVC 4.1 package contains Bin/ and Include/ but no linker
# libraries or DirectX headers. Bootstrap the compiler from the requested
# release and add only build-support files from the old pinned SDK snapshot;
# the compiler and linker themselves always come from MSVC 4.1.
#
# The support snapshot's CRT and Win32 import libraries are the MSVC 4.20
# ones.  The original was linked against the MSVC 4.10 set instead:
# WinMainCRTStartup calls _heap_init without testing its result before
# _mtinit/_amsg_exit(_RT_THREAD), and every KERNEL32/USER32/GDI32/ADVAPI32/
# WINMM import hint matches the 4.10 import libraries, not the 4.20 ones.
# The 4.10 libraries are therefore installed over the support set; the
# DirectX import libraries (DDRAW.LIB, DSOUND.LIB), which 4.10 lacks, still
# come from the support snapshot.
MSVC41_URL = https://github.com/decompme/compilers/releases/download/compilers/msvc4.1.tar.gz
MSVC41_SHA256 = ba2a100dad6b5f1f097860c8cb5e0d3b5f78881e69f96f16168b174e12af80b0
MSVC41_ARCHIVE = 3rdparty/msvc4.1.tar.gz
MSVC41_SUPPORT_COMMIT = df2c13aad74c094988c6c7e784234c2e778a0e91
MSVC41_SUPPORT_URL = https://github.com/itsmattkc/MSVC420/archive/$(MSVC41_SUPPORT_COMMIT).tar.gz
MSVC41_SUPPORT_SHA256 = caddbd356e57106050565297c1931871339ee488a58019ba766dc6cb8b214939
MSVC41_SUPPORT_ARCHIVE = 3rdparty/msvc420-support.tar.gz
MSVC410_LIBS_COMMIT = 373f5e621ac8fb7b219873b37334ef1d0a2149e6
MSVC410_LIBS_URL = https://github.com/archaic-msvc/msvc410/archive/$(MSVC410_LIBS_COMMIT).tar.gz
MSVC410_LIBS_SHA256 = 72904772611a8cb23ea51fc2e670e58cf35d4e5d26c80d7a9c740ac73ef3636a
MSVC410_LIBS_ARCHIVE = 3rdparty/msvc410.tar.gz

# wibo needs this compatible runtime beside CL.EXE and LINK.EXE.
MSVCRT40_URL = https://raw.githubusercontent.com/neuromancer/my-teacher-is-an-alien-re/3d1bfe60522ae05b86bbd2252fd01c8d0a11c3df/3rdparty/msvcrt40.dll
MSVCRT40_SHA256 = ab55a2de2b6faf3daacd3e69473d385ceaead8033f7c79beb6bbf802f230f030
MSVCRT_SOURCE = 3rdparty/msvcrt40.dll
MSVCRT_DLL = $(MSVC41_DIR)/Bin/msvcrt40.dll

$(MSVC41_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading Microsoft Visual C++ 4.1..."
	@curl -fL --retry 3 -o "$@.tmp" "$(MSVC41_URL)"
	@printf '%s  %s\n' "$(MSVC41_SHA256)" "$@.tmp" | \
		shasum -a 256 -c - >/dev/null || \
		(rm -f "$@.tmp"; echo "Error: MSVC 4.1 archive checksum mismatch." >&2; exit 1)
	@mv "$@.tmp" "$@"

$(MSVC41_SUPPORT_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading pinned linker-library support files..."
	@curl -fL --retry 3 -o "$@.tmp" "$(MSVC41_SUPPORT_URL)"
	@printf '%s  %s\n' "$(MSVC41_SUPPORT_SHA256)" "$@.tmp" | \
		shasum -a 256 -c - >/dev/null || \
		(rm -f "$@.tmp"; echo "Error: support archive checksum mismatch." >&2; exit 1)
	@mv "$@.tmp" "$@"

$(MSVC410_LIBS_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading pinned Microsoft Visual C++ 4.10 libraries..."
	@curl -fL --retry 3 -o "$@.tmp" "$(MSVC410_LIBS_URL)"
	@printf '%s  %s\n' "$(MSVC410_LIBS_SHA256)" "$@.tmp" | \
		shasum -a 256 -c - >/dev/null || \
		(rm -f "$@.tmp"; echo "Error: MSVC 4.10 library archive checksum mismatch." >&2; exit 1)
	@mv "$@.tmp" "$@"

$(MSVC41_STAMP): $(MSVC41_ARCHIVE) $(MSVC41_SUPPORT_ARCHIVE) $(MSVC410_LIBS_ARCHIVE)
	@mkdir -p compilers
	@msvc41_install=$$(mktemp -d compilers/.msvc41-install.XXXXXX); \
		msvc41_support=$$(mktemp -d compilers/.msvc41-support.XXXXXX); \
		msvc410_libs=$$(mktemp -d compilers/.msvc41-libs.XXXXXX); \
		trap 'rm -rf "$$msvc41_install" "$$msvc41_support" "$$msvc410_libs"' 0; \
		tar -xzf "$(MSVC41_ARCHIVE)" -C "$$msvc41_install"; \
		tar -xzf "$(MSVC41_SUPPORT_ARCHIVE)" -C "$$msvc41_support"; \
		tar -xzf "$(MSVC410_LIBS_ARCHIVE)" -C "$$msvc410_libs" \
			'msvc410-$(MSVC410_LIBS_COMMIT)/lib'; \
		support_root=$$(find "$$msvc41_support" -mindepth 1 -maxdepth 1 \
			-type d -name 'MSVC420-*' -print -quit); \
		test -n "$$support_root"; \
		libs_root="$$msvc410_libs/msvc410-$(MSVC410_LIBS_COMMIT)"; \
		test -f "$$libs_root/lib/LIBCMTD.LIB"; \
		mkdir -p "$$msvc41_install/Lib" "$$msvc41_install/SdkInclude"; \
		cp -R "$$support_root/lib/." "$$msvc41_install/Lib/"; \
		cp -R "$$libs_root/lib/." "$$msvc41_install/Lib/"; \
		cp -R "$$support_root/include/." "$$msvc41_install/SdkInclude/"; \
		touch "$$msvc41_install/.installed"; \
		rm -rf "$(MSVC41_DIR)"; \
		mv "$$msvc41_install" "$(MSVC41_DIR)"; \
		rm -rf "$$msvc41_support" "$$msvc410_libs"; \
		trap - 0

msvc41-toolchain: $(MSVCRT_DLL)

$(MSVCRT_DLL): $(MSVCRT_SOURCE) | $(MSVC41_STAMP)
	cp -f $< $@

$(MSVCRT_SOURCE):
	@mkdir -p $(dir $@)
	@echo "Downloading wibo-compatible msvcrt40.dll..."
	@curl -fL --retry 3 -o "$@.tmp" "$(MSVCRT40_URL)"
	@printf '%s  %s\n' "$(MSVCRT40_SHA256)" "$@.tmp" | \
		shasum -a 256 -c - >/dev/null || \
		(rm -f "$@.tmp"; echo "Error: msvcrt40.dll checksum mismatch." >&2; exit 1)
	@mv "$@.tmp" "$@"

# The raster library was hand-written assembly (docs/COMPILER.md).  UASM, a
# MASM-compatible assembler built from a pinned source snapshot, reproduces it.
UASM_COMMIT = 29a2237acab283c572aff734c6fbbf26295068c1
UASM_URL = https://github.com/neuviemeporte/UASM/archive/$(UASM_COMMIT).tar.gz
UASM_SHA256 = 5f5fe810e641d118f16ba87135e4c26e4a13733c5f96dcd9db0c1edc618f34f9
UASM_ARCHIVE = 3rdparty/uasm.tar.gz
UASM_DIR = compilers/uasm
UASM = $(OUT_DIR)/uasm/uasm
HOST_CC ?= cc

$(UASM_ARCHIVE):
	@mkdir -p $(dir $@)
	@echo "Downloading pinned UASM sources..."
	@curl -fL --retry 3 -o "$@.tmp" "$(UASM_URL)"
	@printf '%s  %s\n' "$(UASM_SHA256)" "$@.tmp" | \
		shasum -a 256 -c - >/dev/null || \
		(rm -f "$@.tmp"; echo "Error: UASM archive checksum mismatch." >&2; exit 1)
	@mv "$@.tmp" "$@"

$(UASM_DIR)/gccmod.inc: $(UASM_ARCHIVE)
	@mkdir -p compilers
	@uasm_install=$$(mktemp -d compilers/.uasm-install.XXXXXX); \
		trap 'rm -rf "$$uasm_install"' 0; \
		tar -xzf "$(UASM_ARCHIVE)" -C "$$uasm_install"; \
		rm -rf "$(UASM_DIR)"; \
		mv "$$uasm_install/UASM-$(UASM_COMMIT)" "$(UASM_DIR)"; \
		touch "$@"; \
		trap - 0

# UASM's own makefile links with GNU ld's -Map option, which Apple's linker
# rejects, so build its objects there and link them here.
$(UASM): $(UASM_DIR)/gccmod.inc
	@mkdir -p $(dir $@)
	@echo "Building UASM..."
	@uasm_objs="main.o $$(sed -n 's|^\$$(OUTD)/\([A-Za-z0-9_]*\.o\).*|\1|p' \
			$(UASM_DIR)/gccmod.inc)"; \
		$(MAKE) -C $(UASM_DIR) CC=$(HOST_CC) \
			OUTD=$(abspath $(dir $@)) \
			$$(for o in $$uasm_objs; do printf '%s ' "$(abspath $(dir $@))/$$o"; done) \
			> $(dir $@)build.log 2>&1 || (cat $(dir $@)build.log >&2; exit 1); \
		$(HOST_CC) $$(for o in $$uasm_objs; do printf '%s ' "$(dir $@)$$o"; done) -o $@

# The raster library is assembled; everything else is compiled.
$(OUT_DIR)/%.obj: src/%.asm | $(UASM)
	@mkdir -p $(dir $(OUT_DIR)/$*)
	@$(UASM) -q -coff -Fo$@ -Fw$(OUT_DIR)/$*.err $< > $(OUT_DIR)/$*.stdout || \
		(cat $(OUT_DIR)/$*.stdout >&2; exit 1)

$(IX_SOUND_LIB): $(call objects_of,$(SRCS_IX_SOUND))
$(IX_STREAMER_LIB): $(call objects_of,$(SRCS_IX_STREAMER))
$(IX_DSP_LIB): $(call objects_of,$(SRCS_IX_DSP))
$(IX_LZO_LIB): $(call objects_of,$(SRCS_IX_LZO))
$(IX_FILE_LIB): $(call objects_of,$(SRCS_IX_FILE))
$(IX_LIBS): | $(MSVCRT_DLL)
	@rm -f $@
	$(LIBEXE) /nologo /OUT:$@ $(filter %.obj,$^)

$(TARGET): $(LINK_OBJS) $(IX_LIBS) Makefile bin/appendStaticSymbols.py bin/stampPeTimestamp.py | $(MSVCRT_DLL)
	@mkdir -p $(dir $(LINK_TARGET))
	env LIB='$(GAME_LIBPATH)' $(LINK) $(LINKFLAGS) /MAP:$(LINKMAPFILE) \
		$(LINK_OBJS) $(IX_LIBS_BEFORE_CRT) $(GAME_LIBS) $(CRT_LIBS) \
		$(IX_LIBS_AFTER_CRT) /OUT:$(LINK_TARGET)
	@python3 bin/stampPeTimestamp.py $(LINK_TARGET) $(ORIGINAL_TIMESTAMP)
	@cp -f $(LINK_TARGET) $@
	@python3 bin/appendStaticSymbols.py --map $(LINKMAPFILE) \
		--rebuilt $(TARGET) --output $(MAPFILE) $(OBJS)

# Recreate missing analysis metadata without changing the executable. LINK's
# original map is retained; private addresses come from COFF and relocations.
$(MAPFILE): $(TARGET) bin/appendStaticSymbols.py
	@python3 bin/appendStaticSymbols.py --map $(LINKMAPFILE) \
		--rebuilt $(TARGET) --output $@ $(OBJS)

# The game-side wave module calls directly into IxSample and IxSound member
# functions.  Those ECX-based calls at 0x42B4A0 and 0x42B680 prove this one
# source unit was compiled as C++ while retaining its original .c filename.
$(OUT_DIR)/sound.obj $(OUT_DIR)/sound.asm: src/sound.c $(GAME_HEADERS) | $(WIBO) $(MSVCRT_DLL)
	@mkdir -p $(dir $(OUT_DIR)/sound)
	@env INCLUDE='$(MSVC_INC)' $(CC) $(CFLAGS_CORE) /TP $< \
		/Fo$(OUT_DIR)/sound.obj \
		/Fa$(OUT_DIR)/sound.asm \
		> $(OUT_DIR)/sound.stdout

$(OUT_DIR)/%.obj $(OUT_DIR)/%.asm: src/%.c $(GAME_HEADERS) | $(WIBO) $(MSVCRT_DLL)
	@mkdir -p $(dir $(OUT_DIR)/$*)
	@env INCLUDE='$(MSVC_INC)' $(CC) $(CFLAGS_CORE) $< \
		/Fo$(OUT_DIR)/$*.obj \
		/Fa$(OUT_DIR)/$*.asm \
		> $(OUT_DIR)/$*.stdout

# ix/ is built unoptimised; see the CFLAGS_IX note above.
$(OUT_DIR)/ix/%.obj $(OUT_DIR)/ix/%.asm: src/ix/%.cpp $(GAME_HEADERS) | $(WIBO) $(MSVCRT_DLL)
	@mkdir -p $(OUT_DIR)/ix
	@env INCLUDE='$(MSVC_INC)' $(CC) $(CFLAGS_IX) /I src\\ix $< \
		/Fo$(OUT_DIR)/ix/$*.obj \
		/Fa$(OUT_DIR)/ix/$*.asm \
		> $(OUT_DIR)/ix/$*.stdout

# The console-owner wrapper at 0x40FCD0 contains compiler-generated C++
# construction cleanup.  /GX on this unit reproduces it exactly; debug.cpp has
# no such unwind records and stays on the core defaults.
$(OUT_DIR)/console.obj $(OUT_DIR)/console.asm: CFLAGS_CORE_CPP_EXTRA = /GX

$(OUT_DIR)/%.obj $(OUT_DIR)/%.asm: src/%.cpp $(GAME_HEADERS) | $(WIBO) $(MSVCRT_DLL)
	@mkdir -p $(dir $(OUT_DIR)/$*)
	@env INCLUDE='$(MSVC_INC)' $(CC) $(CFLAGS_CORE) $(CFLAGS_CORE_CPP_EXTRA) $< \
		/Fo$(OUT_DIR)/$*.obj \
		/Fa$(OUT_DIR)/$*.asm \
		> $(OUT_DIR)/$*.stdout

# ---------------------------------------------------------------------------
# Recovery reports and binary-comp audits
# ---------------------------------------------------------------------------

BC = --config $(VERIFY_CONFIG) --target full

CALLS_FLAGS ?=
GLOBAL_ACCESS_FLAGS ?=
ORDER_FLAGS ?=

# binary-comp command coverage (mirrors the sibling project):
#   calls         -> verify-calls
#   compare       -> compare-func FUNC=Name
#   data          -> globals-data, globals-data-verbose, missing-data
#   exe           -> compare, compare-functions
#   export-asm    -> export-asm
#   global-access -> verify-global-access
#   globals       -> verify-globals, verify-globals-code, globals-missing,
#                    audit-auto-complete-globals, audit-rebuilt-global-layout
#   order         -> order
#   report        -> report
#   seh           -> seh
#   triage        -> triage
#   values        -> verify-values, verify-values-stack-locals
#   vtables       -> verify-vtables

sort:
	@python3 bin/sortByAddress.py
	@python3 bin/sortGlobalsByAddress.py
	@python3 bin/expandOneLiners.py --check

# Rewrite `T f(void) { body; }` onto separate lines.  A body folded onto the
# signature hides how many statements the original has (AGENTS.md).
expand-one-liners:
	@python3 bin/expandOneLiners.py

# Verify every `Function start:` annotation names the function really at that
# address.  A wrong annotation makes `report` compare against the wrong original.
audit-addresses:
	@python3 bin/auditAddresses.py

# Compiler-generated tail calls and calling-convention adapters must come from
# ordinary C source, never from hand-written jump/call assembly.
audit-compiler-glue:
	@python3 bin/auditCompilerGlue.py

progress:
	@python3 bin/showProgress.py

$(GLOBALS_AUDIT_SOURCE) $(GLOBALS_AUDIT_MANIFEST): bin/collectGlobalDefinitions.py Makefile include/game_data.h $(GLOBALS_DEFINITION_SOURCES)
	@python3 bin/collectGlobalDefinitions.py \
		--output $(GLOBALS_AUDIT_SOURCE) \
		--unit-dir $(GLOBALS_AUDIT_DIR) \
		--constants-header include/game_data.h \
		$(GLOBALS_DEFINITION_SOURCES)

# Function annotations were first projected through the reviewed WC1-to-WC2
# map.  That migration is complete: the source annotations are authoritative
# and src/map is rebuilt from them (bin/updateFunctionMap.py).  The remap
# audit is kept for provenance and is no longer a gate, because functions the
# WC1 map never covered have since been implemented and named.
remap-audit:
	@python3 bin/remapWC1ToWC2.py --check

function-map:
	@python3 bin/updateFunctionMap.py

report: $(TARGET) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) report $(BC) --no-build --bytes \
		$(if $(FILTER),--filter $(FILTER))

# Compare a single rebuilt function against the original.  This is the inner
# loop while implementing:  make compare-func FUNC=MinShort
# Exports are named FUN_<ADDRESS>.disassembled.txt (see ExportToCompile.java),
# so resolve the file from the `Function:` header rather than the symbol name.
# Temporary diagnostic build: replace the "bad vport" text with the failing
# viewport's address, its fields and the allocation registry, so the caller can
# be identified.  Rebuilds only gr.c; `make` restores the reference build.
vport-debug:
	@rm -f $(OUT_DIR)/gr.obj
	@$(MAKE) EXTRA_CFLAGS=/DVPORT_DEBUG

compare-func: $(TARGET) $(GLOBALS_AUDIT_SOURCE) | code-full $(ORIGINAL_EXE)
	@test -n "$(FUNC)" || (echo "usage: make compare-func FUNC=<FunctionName>" >&2 && exit 1)
	@f=$$(grep -lE "^Function: $(FUNC)$$" $(CODE_DIR)/*.disassembled.txt 2>/dev/null | head -1); \
	test -n "$$f" || (echo "no export for $(FUNC) in $(CODE_DIR)/ -- run 'make export-asm'" >&2 && exit 1); \
	$(BINARY_COMP) compare $(BC) --no-build $(FUNC) "$$f"

# Regenerate code-full/ straight from the original PE with Capstone.  Preferred
# over scraping Ghidra; bin/exportGhidra.py remains for names Ghidra knows and
# the PE does not.
export-asm: | $(ORIGINAL_EXE)
	@$(BINARY_COMP) export-asm $(BC) --clean $(EXPORT_ASM_FLAGS)
	@touch $(CODE_EXPORT_STAMP)

# Split near-miss functions into source-reachable vs allocator churn.
triage: $(TARGET) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) triage $(BC) $(if $(FILTER),--filter $(FILTER))

# Original-address compilation-unit ordering and boundary evidence.  This is the
# main tool for recovering game-core module boundaries, which are still unknown.
order: $(TARGET) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) order $(BC) --no-build $(if $(FILTER),--filter $(FILTER)) $(ORDER_FLAGS)

seh: $(TARGET) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) seh $(BC) --report $(if $(FILTER),--filter $(FILTER)) || true

globals-data: $(TARGET) $(GLOBALS_AUDIT_SOURCE) | $(ORIGINAL_EXE)
	@$(BINARY_COMP) data $(BC)

globals-data-verbose: $(TARGET) $(GLOBALS_AUDIT_SOURCE) | $(ORIGINAL_EXE)
	@$(BINARY_COMP) data $(BC) --verbose

missing-data: $(GLOBALS_AUDIT_SOURCE) | $(ORIGINAL_EXE)
	@$(BINARY_COMP) data $(BC) \
		--find-missing \
		--min-address $(GLOBALS_MISSING_MIN_ADDRESS) \
		--max-address $(GLOBALS_MISSING_MAX_ADDRESS)

compare: $(TARGET) | $(ORIGINAL_EXE)
	@$(BINARY_COMP) exe $(BC)

# Literal file identity, including PE/debug metadata and every padding byte.
compare-bytes: $(TARGET) | $(ORIGINAL_EXE)
	@$(BINARY_COMP) byte-compare --fail-on-difference $(ORIGINAL_EXE) $(TARGET)

compare-functions: $(TARGET) | $(ORIGINAL_EXE)
	@$(BINARY_COMP) exe $(BC) --functions

compare-full: compare

compare-full-functions: compare-functions

# ---------------------------------------------------------------------------
# Verification
# ---------------------------------------------------------------------------

# These commands consume addresses, including compiler-private data symbols.
all build build-full report compare-func triage order seh globals-data \
globals-data-verbose compare compare-functions verify-globals \
verify-globals-data verify-globals-code audit-auto-complete-globals \
audit-rebuilt-global-layout verify-vtables verify-values \
verify-values-stack-locals: $(MAPFILE)

verify:
	@$(MAKE) audit-compiler-glue
	@$(MAKE) verify-globals
	@$(MAKE) verify-globals-data
	@$(MAKE) verify-globals-code
	@$(MAKE) audit-rebuilt-global-layout
	@$(MAKE) verify-calls
	@$(MAKE) verify-global-access
	@$(MAKE) verify-values
	@$(MAKE) verify-values-stack-locals
	@$(MAKE) verify-vtables

# Declaration order is a per-compilation-unit property: MSVC emits each unit's
# data in source order and the linker concatenates the units.  The combined
# audit source is every definition file end to end, so while src/globals.c
# still holds globals belonging to the owner units, every unit boundary reads
# as an address decrease that no ordering can remove.  Score the order per
# definition file instead, and keep the combined run for everything else.
verify-globals: $(TARGET) $(GLOBALS_AUDIT_SOURCE) $(GLOBALS_AUDIT_MANIFEST) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) globals $(BC) --fail-on-issues --fail-on-warnings \
		--no-source-order
	@set -e; while IFS= read -r global_source; do \
		$(BINARY_COMP) globals $(BC) \
			--globals-source "$$global_source" \
			--globals-h "$$global_source" \
			--no-address-warnings \
			--fail-on-issues; \
	done < $(GLOBALS_AUDIT_MANIFEST)

# The source-level globals audit cannot serialize every nested or symbolic
# initializer.  Compare the linked bytes too, but keep the normal verify output
# to the actionable mismatches and summary.
verify-globals-data: $(TARGET) $(GLOBALS_AUDIT_SOURCE) | $(ORIGINAL_EXE)
	@audit_output=$$(mktemp "$${TMPDIR:-/tmp}/globals-data.XXXXXX"); \
		trap 'rm -f "$$audit_output"' 0 1 2 15; \
		status=0; \
		$(BINARY_COMP) data $(BC) > "$$audit_output" || status=$$?; \
		awk '/ MISMATCH / || / NO_SYMBOL / || /^Summary:/' \
			"$$audit_output"; \
		exit $$status

verify-globals-code: $(TARGET) $(GLOBALS_AUDIT_SOURCE) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) globals $(BC) \
		--include-code-globals \
		--fail-on-issues \
		--fail-on-warnings \
		--min-address $(GLOBALS_MISSING_MIN_ADDRESS) \
		--max-address $(GLOBALS_MISSING_MAX_ADDRESS)

globals-missing: verify-globals-code

# Compatibility aliases used by the sibling project's notes and scripts.
globals: globals-data

globals-verbose: globals-data-verbose

audit-auto-complete-globals: $(TARGET) $(GLOBALS_AUDIT_SOURCE) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) globals $(BC) --show-auto-complete-reviewed

audit-rebuilt-global-layout: $(TARGET) $(GLOBALS_AUDIT_SOURCE) $(GLOBALS_AUDIT_MANIFEST) | code-full $(ORIGINAL_EXE)
	@set -e; while IFS= read -r global_source; do \
		$(BINARY_COMP) globals $(BC) \
			--globals-source "$$global_source" \
			--globals-h "$$global_source" \
			--check-rebuilt-layout \
			--no-address-warnings \
			--fail-on-issues; \
	done < $(GLOBALS_AUDIT_MANIFEST)

# WC1's own code is C and has no vtables, but the DirectDraw/DirectSound COM
# interfaces are dispatched through vtables, so this stays in the checklist.
verify-vtables: $(TARGET) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) vtables $(BC)

verify-calls: | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) calls $(BC) $(CALLS_FLAGS) $(if $(FILTER),$(FILTER))

verify-global-access: $(GLOBALS_AUDIT_SOURCE) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) global-access $(BC) $(GLOBAL_ACCESS_FLAGS) $(if $(FILTER),$(FILTER))

verify-values: $(TARGET) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) values $(BC) --no-build --min-similarity $(VALUE_MIN_SIMILARITY) $(VALUES_FLAGS)

verify-values-stack-locals: $(TARGET) | code-full $(ORIGINAL_EXE)
	@$(BINARY_COMP) values $(BC) --no-build \
		--min-similarity $(STACK_LOCAL_VALUE_MIN_SIMILARITY) \
		--include-stack-locals \
		$(STACK_LOCAL_VALUES_FLAGS) \
		$(VALUES_FLAGS)

# ---------------------------------------------------------------------------
# Original binary and Ghidra export
# ---------------------------------------------------------------------------

# The retail executable is not vendored.  Copy it in from the analysis tree.
$(ORIGINAL_EXE):
	@test -f "$(ORIGINAL_SRC)" || \
		(echo "Error: original not found at $(ORIGINAL_SRC). Set ORIGINAL_SRC=/path/to/WC2.EXE" >&2 && exit 1)
	@mkdir -p $(dir $@)
	@cp -f "$(ORIGINAL_SRC)" $@
	@echo "Staged original -> $@"

# code-full/ is a reproducible binary-comp export of WC2. A WC1-era directory
# has no WC2 stamp, so `make report` replaces it automatically.
code-full: $(CODE_EXPORT_STAMP)

$(CODE_EXPORT_STAMP): $(SRCS) | $(ORIGINAL_EXE)
	@$(MAKE) export-asm
	@test -f $@

# ---------------------------------------------------------------------------
# DREAMM launch targets
# ---------------------------------------------------------------------------

$(DREAMM_STAMP):
	@mkdir -p $(DREAMM_DIR)
	@echo "Downloading DREAMM $(DREAMM_VERSION)..."
	@curl -L -o $(DREAMM_DIR)/$(DREAMM_ARCHIVE) $(DREAMM_BASE_URL)/$(DREAMM_ARCHIVE)
ifeq ($(UNAME_S),Darwin)
	@rm -rf $(DREAMM_DIR)/mnt
	@hdiutil attach $(DREAMM_DIR)/$(DREAMM_ARCHIVE) -mountpoint $(DREAMM_DIR)/mnt -nobrowse -quiet
	@rm -rf $(DREAMM_DIR)/DREAMM.app
	@cp -R $(DREAMM_DIR)/mnt/DREAMM.app $(DREAMM_DIR)/
	@hdiutil detach $(DREAMM_DIR)/mnt -quiet
	@xattr -dr com.apple.quarantine $(DREAMM_DIR)/DREAMM.app
else
	@rm -rf $(DREAMM_DIR)/dreamm
	@tar xzf $(DREAMM_DIR)/$(DREAMM_ARCHIVE) -C $(DREAMM_DIR) --strip-components=1
endif
	@rm $(DREAMM_DIR)/$(DREAMM_ARCHIVE)
	@rm -f $(DREAMM_DIR)/.dreamm-*.stamp
	@touch "$(DREAMM_STAMP)"

$(DREAMM_BIN): $(DREAMM_STAMP)
	@test -x "$(DREAMM_BIN)" || \
		(echo "Error: DREAMM did not unpack to $(DREAMM_BIN)." >&2 && exit 1)

dreamm: $(DREAMM_BIN)

# The Kilrathi Saga disc carries a ready-to-run WC2 tree at /WC2 (WC2.EXE,
# GAMEDAT with the MODULE/CAMP/BRIEFING files, STREAMS, WINGCMDR.CFG).  Extract
# just that -- 142 MB of the disc's 634 -- rather than asking for a separate
# install.  bsdtar reads ISO9660 directly and ships with macOS and most Linuxes.
$(RUN_DIR)/GAMEDAT:
	@test -n "$(ISO)" || \
		(echo "Error: no disc image found. Put the Kilrathi Saga ISO in data/ or set ISO=." >&2 && exit 1)
	@command -v bsdtar >/dev/null 2>&1 || \
		(echo "Error: bsdtar not found; needed to read the ISO." >&2 && exit 1)
	@echo "Extracting WC2 from $(ISO)..."
	@mkdir -p "$(RUN_DIR)"
	@bsdtar -xf "$(ISO)" -C "$(RUN_DIR)" --strip-components=1 WC2

run-check: $(RUN_DIR)/GAMEDAT
	@mkdir -p "$(RUN_DIR)/hd"

stage-run: $(TARGET) run-check
	cp -f $(TARGET) "$(RUN_DIR)/WC2.EXE"

run: stage-run | $(DREAMM_BIN)
	cd "$(RUN_DIR)" && $(DREAMM) $(DREAMM_MOUNTS) $(DREAMM_PROPS) -launch WC2.EXE

run-original: run-check $(ORIGINAL_EXE) | $(DREAMM_BIN)
	cp -f $(ORIGINAL_EXE) "$(RUN_DIR)/WC2.ORI.EXE"
	cd "$(RUN_DIR)" && $(DREAMM) $(DREAMM_MOUNTS) $(DREAMM_PROPS) -launch WC2.ORI.EXE

# DREAMM's own debugger, the same target the sibling project uses.
debug: $(TARGET) run-check | $(DREAMM_BIN)
	cp -f $(TARGET) "$(RUN_DIR)/WC2.EXE"
	cd "$(RUN_DIR)" && $(DREAMM) $(DREAMM_MOUNTS) $(DREAMM_PROPS) -debug \
		-launch WC2.EXE > debug.log

# ---------------------------------------------------------------------------
# Cleanup and phony declarations
# ---------------------------------------------------------------------------

clean:
	rm -rf $(OUT_DIR)/*.obj $(OUT_DIR)/*.asm $(OUT_DIR)/*.stdout \
	       $(OUT_DIR)/ix $(TARGET) $(LINK_TARGET) $(MAPFILE) $(LINKMAPFILE)

clean-run:
	rm -f "$(RUN_DIR)/WC2.EXE" "$(RUN_DIR)/WC2.ORI.EXE" \
	       "$(RUN_DIR)/debug.log"

clean-dreamm:
	rm -rf $(DREAMM_DIR)

clean-modern:
	rm -rf $(MODERN_OUT_DIR)

.PHONY: \
	all \
	audit-addresses \
	audit-compiler-glue \
	audit-auto-complete-globals \
	audit-rebuilt-global-layout \
	build \
	build-full \
	clean-modern \
	compare-full \
	compare-full-functions \
	compare-func \
	debug \
	run-original \
	export-asm \
	globals \
	globals-verbose \
	triage \
	verify-vtables \
	clean \
	clean-run \
	clean-dreamm \
	dreamm \
	compare \
	compare-bytes \
	compare-functions \
	globals-data \
	globals-data-verbose \
	globals-missing \
	missing-data \
	modern \
	modern-gui \
	modern-check-deps \
	modern-test \
	modern-test-cutscenes \
	modern-test-full \
	msvc41-toolchain \
	order \
	progress \
	report \
	run \
	run-check \
	stage-run \
	run-modern \
	run-modern-gui \
	run-modern-cutscene \
	run-modern-dos \
	run-modern-mission \
	seh \
	sort \
	verify \
	verify-calls \
	verify-global-access \
	verify-globals \
	verify-globals-data \
	verify-globals-code \
	verify-values \
	verify-values-stack-locals \
	remap-audit \
	function-map \
