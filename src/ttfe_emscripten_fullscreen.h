#ifdef __EMSCRIPTEN__

#include <emscripten/html5.h>
#include <allegro5/allegro.h>

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
