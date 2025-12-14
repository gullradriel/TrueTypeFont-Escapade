/**\file ttfe_loading.h
 *  'loading' progress bar helpers
 *\author Castagnier Mickael
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

static inline float progress01(int start_value, int end_value, int current_value) {
    if (end_value == start_value)
        return (current_value >= end_value) ? 1.0f : 0.0f;

    float t = (float)(current_value - start_value) / (float)(end_value - start_value);
    return clampf(t, 0.0f, 1.0f);
}

// Draws:
// - sentence over a filled background rectangle
// - a surrounding border around (text bg + progress bar)
// - a progress bar representing current_value from start_value..end_value
//
// You provide placement + font because otherwise there's nowhere to draw it.
// If you truly must not pass font/x/y, you can wrap this with your own globals.
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
    int current_value) {
    if (!sentence || !font) return;

    // Layout knobs
    const float pad = 8.0f;
    const float border_thickness = 2.0f;
    const float bar_height = 10.0f;
    const float bar_gap = 6.0f;

    const float text_w = (float)al_get_text_width(font, sentence);
    const float text_h = (float)al_get_font_line_height(font);

    const float bg_w = text_w + pad * 2.0f;
    const float bg_h = text_h + pad * 2.0f;

    const float bg_x = x + border_thickness - (text_w / 2);
    const float bg_y = y + border_thickness;

    const float bar_x = bg_x;
    const float bar_y = bg_y + bg_h + bar_gap;

    al_clear_to_color(al_map_rgb(0, 0, 0));

    // Background behind text
    al_draw_filled_rectangle(bg_x, bg_y, bg_x + bg_w, bg_y + bg_h, bg_color);

    // Progress bar (filled portion)
    const float t = progress01(start_value, end_value, current_value);
    al_draw_filled_rectangle(bar_x, bar_y, bar_x + bg_w, bar_y + bar_height, bg_color);
    al_draw_filled_rectangle(bar_x, bar_y, bar_x + (bg_w * t), bar_y + bar_height, bar_color);

    // Optional: bar track outline (subtle) — comment out if you don't want it
    // al_draw_rectangle(bar_x, bar_y, bar_x + bg_w, bar_y + bar_height, border_color, 1.0f);

    // Surrounding border around everything
    al_draw_rectangle(bar_x, bar_y, bar_x + bg_w, bar_y + bar_height, border_color, border_thickness);

    // Text on top
    al_draw_text(font, text_color, bg_x + pad, bg_y + pad, 0, sentence);

    al_flip_display();
}

#ifdef __cplusplus
}
#endif

#endif
