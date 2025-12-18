/**\file ttfe_loading.h
 *  'loading' progress bar helpers
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 13/12/2025
 */

#ifndef TTFE_PROGRESS_LOADER_HEADER_FOR_HACKS
#define TTFE_PROGRESS_LOADER_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

/*
 * Draws:
 * - sentence over a filled background rectangle
 * - a surrounding border around (text bg + progress bar)
 * - a progress bar representing current_value from start_value..end_value
 *
 * You provide placement + font because otherwise there's nowhere to draw it.
 * If you truly must not pass font/x/y, you can wrap this with your own globals.
 */
void draw_text_box_with_progress(
    const char* sentence,
    ALLEGRO_FONT* font,
    float x,
    float y,

    ALLEGRO_COLOR text_color,
    ALLEGRO_COLOR bg_color,
    ALLEGRO_COLOR border_color,
    ALLEGRO_COLOR bar_color,

    int start_value,
    int end_value,
    int current_value);

#ifdef __cplusplus
}
#endif

#endif
