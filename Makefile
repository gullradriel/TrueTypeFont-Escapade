# 
# Makefile for "TrueTypeFont Escapade"
#
# 2025 KrampusHack entry by GullRaDriel
#
# Supports Linux / Windows / Emscripten (wasm) build
#
# Build targets:
#   make            - Build native Linux/Windows executable
#   make all        - Same as above
#   make wasm       - Build WebAssembly version (requires Emscripten SDK)
#   make clean      - Clean native build artifacts
#   make wasm-clean - Clean WebAssembly build artifacts
#   make clean-all  - Clean all build artifacts
#

# Delete command and C compiler setup
RM=rm -f
CC=gcc
EXT=
CLIBS=

# Compilation flags: enable warnings, XOPEN features, use C99, optimize level 3
OPT=-W -Wall -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENTED -std=gnu99 -O3
# Debug options (uncomment for debugging):
#OPT=-g -W -Wall -Wextra -std=gnu99 -O3 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -D_REENTRANT -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENTED -static-libgcc -static-libstdc++

# Source and include directories
VPATH=src/
INCLUDE=src
OBJDIR=obj

# Allegro5 library dependencies
ALLEGRO_LIBS=-lallegro_acodec -lallegro_audio -lallegro_color -lallegro_image -lallegro_main -lallegro_primitives -lallegro_ttf -lallegro_font -lallegro 
CFLAGS+= -DALLEGRO_UNSTABLE

# Timestamp for directory naming (used for backups)
dir_name=$(shell date +%Y_%m_%d_%HH%MM%SS )

#
# Platform-specific configuration
#
ifeq ($(OS),Windows_NT)
    CFLAGS+= -I$(INCLUDE) -D__USE_MINGW_ANSI_STDIO $(OPT)
    RM= del /Q
    CC= gcc
    ifeq (${MSYSTEM},MINGW32)
        RM=rm -f
        CFLAGS+= -m32
        EXT=.exe
        CLIBS=-IC:/msys64/mingw32/include -LC:/msys64/mingw32/lib
    endif
    # Windows with MSYS2 MinGW64
    ifeq (${MSYSTEM},MINGW64)
        RM=rm -f
        CFLAGS+= -DARCH64BITS
        EXT=.exe
        CLIBS=-IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib
    endif
    # Windows with MSYS2 MinGW64 CodeBlocks
    ifeq (${MSYSTEM},MINGW64CB)
        RM=del /Q
        CFLAGS+= -DARCH64BITS
        EXT=.exe
        CLIBS=-IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib
    endif
    CLIBS+= $(ALLEGRO_LIBS) -Wl,-Bstatic -lpthread  -Wl,-Bdynamic -lws2_32  -L../LIB/.
else
    # Unix-like systems (Linux, SunOS, etc.)
    UNAME_S= $(shell uname -s)
    RM=rm -f
    CC=gcc
    EXT=
    # Linux configuration
    ifeq ($(UNAME_S),Linux)
        CFLAGS+= -I$(INCLUDE) $(OPT)
        CLIBS+= $(ALLEGRO_LIBS) -lpthread -lm -no-pie
    endif
    # SunOS/Solaris configuration
    ifeq ($(UNAME_S),SunOS)
        CC=cc
        CFLAGS+= -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -g -v -xc99 -I ../../LIB/include/ -mt -lm
        CLIBS+= $(ALLEGRO_LIBS) -lm -lsocket -lnsl -lpthread -lrt -L..
    endif
endif

#
# Source files and object files
#
SRC=n_common.c n_log.c n_str.c n_list.c cJSON.c ttfe_app_config.c TTF_Escapade.c
OBJ=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# Create object directory if it does not exist
$(shell mkdir -p $(OBJDIR))

# Pattern rule: compile .c to .o
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

# Main target: link object files into executable
TTF_Escapade$(EXT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(CLIBS)

# Default target
all: TTF_Escapade$(EXT)


#
# WebAssembly (WASM) build using Emscripten
# Requires: Emscripten SDK installed and emsdk_env.sh sourced
#

WASM_BUILD_DIR=build_emscripten

# External dependencies for WASM build
ALLEGRO_DIR=allegro5
LIBOGG_DIR=libogg
LIBVORBIS_DIR=libvorbis

# Git repositories for dependencies
ALLEGRO_REPO=https://github.com/liballeg/allegro5.git
LIBOGG_REPO=https://github.com/xiph/ogg.git
LIBVORBIS_REPO=https://github.com/xiph/vorbis.git

# Build directories for WASM dependencies
DEPS_PREFIX=$(WASM_BUILD_DIR)/deps/prefix

ALLEGRO_BUILD_DIR=$(ALLEGRO_DIR)/build_emscripten
LIBOGG_BUILD_DIR=$(LIBOGG_DIR)/build_emscripten
LIBVORBIS_BUILD_DIR=$(LIBVORBIS_DIR)/build_emscripten

# Emscripten tools
EMCC=emcc
EMCMAKE=emcmake

# Emscripten compiler and linker flags
USE_FLAGS = -s USE_FREETYPE=1 \
            -s USE_LIBJPEG=1 \
            -s USE_SDL=2 \
            -s USE_LIBPNG=1 \
            -s FULL_ES3=1 \
            -s ASYNCIFY \
            -s INITIAL_MEMORY=2147418112 \
            -s STACK_SIZE=2097152 \
            -O3
# Debug options (uncomment for debugging WASM builds):
#            -s SAFE_HEAP=1 \
#            -s ASSERTIONS=2 \
#            -O0 \
#            -gsource-map

# WASM C compiler flags
WASM_CFLAGS=$(USE_FLAGS) \
            -Isrc \
            -I$(DEPS_PREFIX)/include \
            -I$(ALLEGRO_DIR)/include \
            -I$(ALLEGRO_BUILD_DIR)/include \
            -I$(ALLEGRO_DIR)/addons/font \
            -I$(ALLEGRO_DIR)/addons/ttf \
            -I$(ALLEGRO_DIR)/addons/primitives \
            -I$(ALLEGRO_DIR)/addons/image \
            -I$(ALLEGRO_DIR)/addons/audio \
            -I$(ALLEGRO_DIR)/addons/acodec \
            -I$(ALLEGRO_DIR)/addons/color \
            -I$(ALLEGRO_DIR)/addons/native_dialog \
            -DALLEGRO_UNSTABLE

# WASM linker flags
WASM_LDFLAGS=$(USE_FLAGS) --preload-file DATA \
             -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
             $(DEPS_PREFIX)/lib/libvorbisfile.a \
             $(DEPS_PREFIX)/lib/libvorbisenc.a \
             $(DEPS_PREFIX)/lib/libvorbis.a \
             $(DEPS_PREFIX)/lib/libogg.a \
             $(ALLEGRO_BUILD_DIR)/lib/liballegro_monolith-static.a

# Clone WASM dependencies if not present
wasm-deps:
	@echo "Ensuring wasm deps are present (git clone if missing)..."
	@if [ ! -d "$(LIBOGG_DIR)" ]; then \
		echo "Cloning $(LIBOGG_DIR)..."; \
		git clone --depth 1 "$(LIBOGG_REPO)" "$(LIBOGG_DIR)"; \
	fi
	@if [ ! -d "$(LIBVORBIS_DIR)" ]; then \
		echo "Cloning $(LIBVORBIS_DIR)..."; \
		git clone --depth 1 "$(LIBVORBIS_REPO)" "$(LIBVORBIS_DIR)"; \
	fi
	@if [ ! -d "$(ALLEGRO_DIR)" ]; then \
		echo "Cloning $(ALLEGRO_DIR)..."; \
		git clone --depth 1 "$(ALLEGRO_REPO)" "$(ALLEGRO_DIR)"; \
	fi

# Setup WASM build environment
wasm-setup:
	@echo "Setting up WebAssembly build environment..."
	@if [ -z "$EMSDK" ]; then \
		echo "ERROR: Emscripten SDK not found!"; \
		echo "Please run: source /path/to/emsdk/emsdk_env.sh"; \
		exit 1; \
	fi
	@mkdir -p $(WASM_BUILD_DIR)
	@mkdir -p $(DEPS_PREFIX)
	@echo "Build directory created: $(WASM_BUILD_DIR)"

# Build libogg for WASM
wasm-libogg: wasm-setup wasm-deps
	@echo "Building libogg (WASM)..."
	@mkdir -p $(LIBOGG_BUILD_DIR)
	@cd $(LIBOGG_BUILD_DIR) && \
		$(EMCMAKE) cmake .. \
			-DCMAKE_BUILD_TYPE=Release \
			-DBUILD_SHARED_LIBS=OFF \
			-DCMAKE_INSTALL_PREFIX="$(abspath $(DEPS_PREFIX))" \
			-DCMAKE_C_FLAGS="$(USE_FLAGS)" && \
		make -j4 && \
		make install
	@echo "libogg build complete."

# Build libvorbis for WASM (depends on libogg)
wasm-libvorbis: wasm-libogg
	@echo "Building libvorbis (WASM)..."
	@mkdir -p $(LIBVORBIS_BUILD_DIR)
	@cd $(LIBVORBIS_BUILD_DIR) && \
		$(EMCMAKE) cmake .. \
			-DCMAKE_BUILD_TYPE=Release \
			-DBUILD_SHARED_LIBS=OFF \
			-DCMAKE_INSTALL_PREFIX="$(abspath $(DEPS_PREFIX))" \
			-DCMAKE_PREFIX_PATH="$(abspath $(DEPS_PREFIX))" \
			-DOGG_INCLUDE_DIR="$(abspath $(DEPS_PREFIX))/include" \
			-DOGG_LIBRARY="$(abspath $(DEPS_PREFIX))/lib/libogg.a" \
			-DCMAKE_C_FLAGS="$(USE_FLAGS)" && \
		make -j4 && \
		make install
	@echo "libvorbis build complete."

# Build Allegro5 for WASM (depends on libvorbis)
wasm-allegro: wasm-libvorbis
	@echo "Building Allegro5 with Emscripten..."
	@if [ ! -d "$(ALLEGRO_DIR)" ]; then \
		echo "ERROR: $(ALLEGRO_DIR) directory not found!"; \
		exit 1; \
	fi
	@mkdir -p $(ALLEGRO_BUILD_DIR)
	@cd $(ALLEGRO_BUILD_DIR) && \
	if [ -z "${EM_CACHE}" ]; then \
		EM_CACHE=${EMSDK}/upstream/emscripten/cache ; \
	fi && \
	$(EMCMAKE) cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DALLEGRO_SDL=ON \
		-DSHARED=OFF \
		-DWANT_MONOLITH=ON \
		-DWANT_ALLOW_SSE=OFF \
		-DWANT_DOCS=OFF \
		-DWANT_TESTS=OFF \
		-DWANT_EXAMPLES=OFF \
		-DWANT_OPENAL=OFF \
		-DALLEGRO_WAIT_EVENT_SLEEP=ON \
		-DCMAKE_PREFIX_PATH="$(abspath $(DEPS_PREFIX))" \
		-DOGG_INCLUDE_DIR="$(abspath $(DEPS_PREFIX))/include" \
		-DOGG_LIBRARY="$(abspath $(DEPS_PREFIX))/lib/libogg.a" \
		-DVORBIS_INCLUDE_DIR="$(abspath $(DEPS_PREFIX))/include" \
		-DVORBIS_LIBRARY="$(abspath $(DEPS_PREFIX))/lib/libvorbis.a" \
		-DVORBISFILE_LIBRARY="$(abspath $(DEPS_PREFIX))/lib/libvorbisfile.a" \
		-DVORBISENC_LIBRARY="$(abspath $(DEPS_PREFIX))/lib/libvorbisenc.a" \
		-DSDL2_INCLUDE_DIR=${EM_CACHE}/sysroot/include \
		-DCMAKE_C_FLAGS="$(USE_FLAGS)" \
		-DCMAKE_CXX_FLAGS="$(USE_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(USE_FLAGS)" && make -j4
	@echo "Allegro5 build complete!"

# Build the game for WASM
wasm: wasm-allegro
	@echo "Compiling TTF_Escapade for WebAssembly..."
	@echo "Note: Make sure you have sourced emsdk_env.sh first!"
	$(EMCC) $(WASM_CFLAGS) \
		$(addprefix src/,$(SRC)) \
		$(WASM_LDFLAGS) \
		-o $(WASM_BUILD_DIR)/TTF_Escapade.html
	@echo ""
	@echo "Build complete! Files generated in $(WASM_BUILD_DIR)/"
	@echo "To test, run from $(WASM_BUILD_DIR):"
	@echo "  python3 -m http.server"
	@echo "Then open http://localhost:8000/TTF_Escapade.html in your browser"

# Clean WASM build artifacts
wasm-clean:
	@echo "Cleaning WebAssembly build..."
	$(RM) -r $(WASM_BUILD_DIR)
	$(RM) -r $(LIBOGG_BUILD_DIR)
	$(RM) -r $(LIBVORBIS_BUILD_DIR)
	$(RM) -r $(ALLEGRO_BUILD_DIR)

# Clean native build artifacts
clean:
	$(RM) $(OBJDIR)/*.o
	$(RM) TTF_Escapade$(EXT)

# Clean all build artifacts (native + WASM)
clean-all: clean wasm-clean

# Declare phony targets (not actual files)
.PHONY: all clean clean-all wasm wasm-setup wasm-deps wasm-libogg wasm-libvorbis wasm-allegro wasm-clean