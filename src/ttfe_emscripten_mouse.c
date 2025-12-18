/**\file ttfe_emscripten_mouse.c
 *  Emscripten pointer lock and mouse handling
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 18/12/2025
 */

#ifdef __EMSCRIPTEN__

#include <stdbool.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "ttfe_emscripten_mouse.h"

/*
  Browser Pointer Lock integration for Allegro5 mouse-look.

  Why this exists:
  - On the Web (WASM), you cannot reliably "warp" the OS cursor (al_set_mouse_xy).
  - Without Pointer Lock, the cursor can leave the canvas and the browser stops sending mouse move deltas.
  - With Pointer Lock active, the browser provides relative deltas (movementX/movementY) indefinitely.

  IMPORTANT BROWSER RULE:
  Pointer Lock can only be *entered* from a user gesture (mouse click / key press).
  So "lock on startup" must be implemented as:
    - ctx->mouse_locked = true (game wants it)
    - request_pointerlock is performed on the first user gesture (or on unpause key).
*/

/* These are defined in TTF_Escapade.c (global accumulators). */
extern float pending_mdx;
extern float pending_mdy;

/* Real browser lock state (true only when the browser granted pointer lock). */
static bool g_pl_active = false;

/* Optional: remember if the game is currently trying to own the pointer. */
static bool g_pl_wanted = false;

/* Focus the canvas using JS (portable across Emscripten versions). */
void web_focus_canvas(void) {
    EM_ASM({
        if (Module && Module['canvas']) Module['canvas'].focus();
    });
}

/* Request pointer lock on the canvas (must be called during a user gesture). */
void web_request_pointer_lock(void) {
    g_pl_wanted = true;
    web_focus_canvas();
    emscripten_request_pointerlock("#canvas", /*unadjustedMovement=*/1);
}

/* Exit pointer lock (safe even if not active). */
void web_exit_pointer_lock(void) {
    g_pl_wanted = false;
    emscripten_exit_pointerlock();
}

/*
  Small helper: are we truly allowed to consume infinite mouse deltas?

  We require both:
    - the game wants lock (ctx->mouse_locked, playing, not paused)
    - the browser actually granted pointer lock (g_pl_active)
*/
bool mouse_capture_active(const GameContext* ctx) {
    return (ctx->mouse_locked && !ctx->paused && ctx->state == STATE_PLAY && g_pl_active);
}

/* Pointer lock change callback (browser grants/revokes lock). */
static EM_BOOL on_pl_change(int eventType, const EmscriptenPointerlockChangeEvent* e, void* userData) {
    (void)eventType;
    (void)userData;
    g_pl_active = e->isActive;
    return EM_TRUE;
}

/*
  Pointer lock error callback.

  Note: many Emscripten versions do NOT define a dedicated "error event" struct.
  The callback type is: EM_BOOL (*)(int, const void*, void*)
*/
static EM_BOOL on_pl_error(int eventType, const void* reserved, void* userData) {
    (void)eventType;
    (void)reserved;
    (void)userData;
    g_pl_active = false;
    return EM_TRUE;
}

/*
  Mouse move callback:
  Use movementX/movementY to feed your existing pending_mdx/pending_mdy.
  This is the most reliable way to get deltas in browsers under pointer lock.
*/
static EM_BOOL on_mousemove(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    (void)eventType;
    GameContext* ctx = (GameContext*)userData;

    if (!mouse_capture_active(ctx)) return EM_TRUE;

    pending_mdx += (float)e->movementX;
    pending_mdy += (float)e->movementY;
    return EM_TRUE;
}

/*
  Click callback:
  If gameplay wants mouse lock, clicking the canvas will acquire pointer lock.
*/
static EM_BOOL on_canvas_click(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    (void)eventType;
    (void)e;
    GameContext* ctx = (GameContext*)userData;

    /* Only request if gameplay *wants* lock and we don't have it yet. */
    if (ctx->mouse_locked && !g_pl_active) {
        web_request_pointer_lock();
    }
    return EM_TRUE;
}

/* Install callbacks once after ctx is created. */
void web_init_pointer_lock(GameContext* ctx) {
    emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, ctx, 1, on_pl_change);
    emscripten_set_pointerlockerror_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, ctx, 1, on_pl_error);

    emscripten_set_mousemove_callback("#canvas", ctx, 1, on_mousemove);
    emscripten_set_click_callback("#canvas", ctx, 1, on_canvas_click);
}

#endif /* __EMSCRIPTEN__ */
