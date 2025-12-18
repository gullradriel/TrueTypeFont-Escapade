# 
# Makefile for "TrueTypeFont Escapade"
#
# 2025 KrampusHack entry by GullRaDriel
#
# Supports Linux / Windows / Emscripten (wasm) build
#

RM=rm -f
CC=gcc
EXT=
CLIBS=

OPT=-W -Wall -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENTED -std=gnu99 -O3
#OPT=-g -W -Wall -Wextra -std=gnu99 -O3 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -D_REENTRANT -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENTED -static-libgcc -static-libstdc++

VPATH=src/
INCLUDE=src
OBJDIR=obj

ALLEGRO_LIBS=-lallegro_acodec -lallegro_audio -lallegro_color -lallegro_image -lallegro_main -lallegro_primitives -lallegro_ttf -lallegro_font -lallegro 
CFLAGS+= -DALLEGRO_UNSTABLE

dir_name=$(shell date +%Y_%m_%d_%HH%MM%SS )

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
    ifeq (${MSYSTEM},MINGW64)
        RM=rm -f
        CFLAGS+= -DARCH64BITS
        EXT=.exe
        CLIBS=-IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib
    endif
    ifeq (${MSYSTEM},MINGW64CB)
        RM=del /Q
        CFLAGS+= -DARCH64BITS
        EXT=.exe
        CLIBS=-IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib
    endif
    CLIBS+= $(ALLEGRO_LIBS) -Wl,-Bstatic -lpthread  -Wl,-Bdynamic -lws2_32  -L../LIB/. #-mwindows
else
    UNAME_S= $(shell uname -s)
    RM=rm -f
    CC=gcc
    EXT=
    ifeq ($(UNAME_S),Linux)
        CFLAGS+= -I$(INCLUDE) $(OPT)
        CLIBS+= $(ALLEGRO_LIBS) -lpthread -lm -no-pie
    endif
    ifeq ($(UNAME_S),SunOS)
        CC=cc
        CFLAGS+= -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -g -v -xc99 -I ../../LIB/include/ -mt -lm
        CLIBS+= $(ALLEGRO_LIBS) -lm -lsocket -lnsl -lpthread -lrt -L..
    endif
endif


SRC=n_common.c n_log.c n_str.c n_list.c cJSON.c \
    ttfe_text.c ttfe_vector3d.c ttfe_vbo.c ttfe_app_config.c ttfe_game_context.c ttfe_entities.c ttfe_stars.c \
    ttfe_loading.c ttfe_emscripten_fullscreen.c ttfe_emscripten_mouse.c ttfe_particles.c ttfe_level.c \
    TTF_Escapade.c

OBJ=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

$(shell mkdir -p $(OBJDIR))

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

TTF_Escapade$(EXT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(CLIBS)

all: TTF_Escapade$(EXT)


#
# WASM build
#

WASM_BUILD_DIR=build_emscripten

ALLEGRO_DIR=allegro5
LIBOGG_DIR=libogg
LIBVORBIS_DIR=libvorbis

ALLEGRO_REPO=https://github.com/liballeg/allegro5.git
LIBOGG_REPO=https://github.com/xiph/ogg.git
LIBVORBIS_REPO=https://github.com/xiph/vorbis.git

DEPS_PREFIX=$(WASM_BUILD_DIR)/deps/prefix

ALLEGRO_BUILD_DIR=$(ALLEGRO_DIR)/build_emscripten
LIBOGG_BUILD_DIR=$(LIBOGG_DIR)/build_emscripten
LIBVORBIS_BUILD_DIR=$(LIBVORBIS_DIR)/build_emscripten

EMCC=emcc
EMCMAKE=emcmake

USE_FLAGS = -s USE_FREETYPE=1 \
            -s USE_LIBJPEG=1 \
            -s USE_SDL=2 \
            -s USE_LIBPNG=1 \
            -s FULL_ES3=1 \
            -s ASYNCIFY \
            -s INITIAL_MEMORY=2147418112 \
            -s STACK_SIZE=2097152 \
            -O3
# DEBUG OPTIONS
#            -s SAFE_HEAP=1 \
#            -s ASSERTIONS=2 \
#            -O0 \
#            -gsource-map

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

WASM_LDFLAGS=$(USE_FLAGS) --preload-file DATA \
             -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
             $(DEPS_PREFIX)/lib/libvorbisfile.a \
             $(DEPS_PREFIX)/lib/libvorbisenc.a \
             $(DEPS_PREFIX)/lib/libvorbis.a \
             $(DEPS_PREFIX)/lib/libogg.a \
             $(ALLEGRO_BUILD_DIR)/lib/liballegro_monolith-static.a

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

wasm-clean:
	@echo "Cleaning WebAssembly build..."
	$(RM) -r $(WASM_BUILD_DIR)
	$(RM) -r $(LIBOGG_BUILD_DIR)
	$(RM) -r $(LIBVORBIS_BUILD_DIR)
	$(RM) -r $(ALLEGRO_BUILD_DIR)

clean:
	$(RM) $(OBJDIR)/*.o
	$(RM) TTF_Escapade$(EXT)

clean-all: clean wasm-clean

.PHONY: all clean clean-all wasm wasm-setup wasm-deps wasm-libogg wasm-libvorbis wasm-allegro wasm-clean
