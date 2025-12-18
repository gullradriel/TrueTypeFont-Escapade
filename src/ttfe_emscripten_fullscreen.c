/**\file ttfe_emscripten_fullscreen.h
 *  Emscripten fullscreen callback handling
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 18/12/2025
 */

#ifdef __EMSCRIPTEN__

#include <ttfe_emscripten_fullscreen.h>
#include <ttfe_game_context.h>
#include <nilorea/n_log.h>

/* Callback function for handling fullscreen state changes in Emscripten. */
EM_BOOL on_fullscreen_change(int eventType,
                             const EmscriptenFullscreenChangeEvent* e,
                             void* userData) {
    GameContext* ctx = (GameContext*)userData;
    ctx->pending_w = e->elementWidth;
    ctx->pending_h = e->elementHeight;
    ctx->pending_resize = true;
    n_log(LOG_INFO, "fullscreen=%d element=%dx%d", (int)e->isFullscreen, e->elementWidth, e->elementHeight);
    return EM_TRUE;
}

#endif
