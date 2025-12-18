/**\file ttfe_emscripten_fullscreen.h
 *  Emscripten fullscreen callback handling
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef __TTFE_EMSCRIPTEN_FULLSCREEN__
#define __TTFE_EMSCRIPTEN_FULLSCREEN__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __EMSCRIPTEN__

#include <emscripten/html5.h>

/* Callback function for handling fullscreen state changes in Emscripten. */
EM_BOOL on_fullscreen_change(int eventType,
                             const EmscriptenFullscreenChangeEvent* e,
                             void* userData);

#endif

#ifdef __cplusplus
}
#endif

#endif
