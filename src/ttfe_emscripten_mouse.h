/**\file ttfe_emscripten_mouse.h
 *  Emscripten pointer lock and mouse handling
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef TTFE_EMSCRIPTEN_MOUSE_HEADER_FOR_HACKS
#define TTFE_EMSCRIPTEN_MOUSE_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __EMSCRIPTEN__

#include <stdbool.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <ttfe_game_context.h>

/* Focus the canvas using JS (portable across Emscripten versions). */
void web_focus_canvas(void);

/* Request pointer lock on the canvas (must be called during a user gesture). */
void web_request_pointer_lock(void);

/* Exit pointer lock (safe even if not active). */
void web_exit_pointer_lock(void);

/*
  Small helper: are we truly allowed to consume infinite mouse deltas?

  We require both:
    - the game wants lock (ctx->mouse_locked, playing, not paused)
    - the browser actually granted pointer lock (g_pl_active)
*/
bool mouse_capture_active(const GameContext* ctx);

/* Pointer lock change callback (browser grants/revokes lock). */
static EM_BOOL on_pl_change(int eventType, const EmscriptenPointerlockChangeEvent* e, void* userData);

/*
  Pointer lock error callback.

  Note: many Emscripten versions do NOT define a dedicated "error event" struct.
  The callback type is: EM_BOOL (*)(int, const void*, void*)
*/
static EM_BOOL on_pl_error(int eventType, const void* reserved, void* userData);

/*
  Mouse move callback:
  Use movementX/movementY to feed your existing pending_mdx/pending_mdy.
  This is the most reliable way to get deltas in browsers under pointer lock.
*/
static EM_BOOL on_mousemove(int eventType, const EmscriptenMouseEvent* e, void* userData);

/*
  Click callback:
  If gameplay wants mouse lock, clicking the canvas will acquire pointer lock.
*/
static EM_BOOL on_canvas_click(int eventType, const EmscriptenMouseEvent* e, void* userData);

/* Install callbacks once after ctx is created. */
void web_init_pointer_lock(GameContext* ctx);

#endif /* __EMSCRIPTEN__ */

#ifdef __cplusplus
}
#endif

#endif
