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

#include <GL/gl.h>

static inline float safe_progress(int start_value, int end_value, int current_value) {
    if (end_value == start_value) return (current_value >= end_value) ? 1.0f : 0.0f;
    float t = (float)(current_value - start_value) / (float)(end_value - start_value);
    return clampf(t, 0.0f, 1.0f);
}

static inline void color_to_rgba_f(ALLEGRO_COLOR c, float* r, float* g, float* b, float* a) {
    /* Allegro stores floats internally; al_unmap_rgba_f gives exact floats */
    al_unmap_rgba_f(c, r, g, b, a);
}

/* Draw a filled rect in screen coordinates (x,y,w,h) using OpenGL */
static void gl_fill_rect(float x, float y, float w, float h, ALLEGRO_COLOR col) {
    float r, g, b, a;
    color_to_rgba_f(col, &r, &g, &b, &a);

    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

/* Draw a rectangle outline in screen coordinates (x,y,w,h) using OpenGL */
static void gl_rect_outline(float x, float y, float w, float h, float thickness, ALLEGRO_COLOR col) {
    /* Simple approach: draw 4 filled thin rectangles (top, bottom, left, right) */
    gl_fill_rect(x, y, w, thickness, col);                  // top
    gl_fill_rect(x, y + h - thickness, w, thickness, col);  // bottom
    gl_fill_rect(x, y, thickness, h, col);                  // left
    gl_fill_rect(x + w - thickness, y, thickness, h, col);  // right
}

/* - sentence: text to display
   - font: the font used by al_draw_text
   - x, y: top-left position of the whole widget
   - padding: space between border and content (text/bg/bar)
   - bg_color: background behind the text line
   - border_color: surrounding rectangle color
   - bar_color: progress bar fill color
   - start_value/end_value/current_value: define progress */
void draw_text_box_with_progress_gl(
    const char* sentence,
    ALLEGRO_FONT* font,
    float x,
    float y,
    float padding,

    ALLEGRO_COLOR text_color,
    ALLEGRO_COLOR bg_color,
    ALLEGRO_COLOR border_color,
    ALLEGRO_COLOR bar_color,

    int start_value,
    int end_value,
    int current_value) {
    if (!sentence || !font) return;

    /* Measure text */
    const int text_w = al_get_text_width(font, sentence);
    const int text_h = al_get_font_line_height(font);

    /* Layout constants */
    const float border_thickness = 2.0f;
    const float bar_height = 10.0f;
    const float bar_gap = 6.0f;  // space between text bg and bar

    /* Content area sizes */
    const float bg_w = (float)text_w + padding * 2.0f;
    const float bg_h = (float)text_h + padding * 2.0f;
    const float bar_w = bg_w;  // same width as text background

    /* Inner content origin (inside border) */
    const float cx = x + border_thickness - text_w / 2;
    const float cy = y + border_thickness;

    /* Background rect (behind text only) */
    const float bg_x = cx;
    const float bg_y = cy;

    /* Progress bar rect */
    const float bar_x = cx;
    const float bar_y = cy + bg_h + bar_gap;

    /* Make OpenGL draw in 2D screen space like Allegro */
    /* Allegro usually manages transforms, but when mixing raw GL calls,
       set up a simple orthographic projection matching the backbuffer */
    ALLEGRO_DISPLAY* disp = al_get_current_display();
    if (!disp) return;

    const int dw = al_get_display_width(disp);
    const int dh = al_get_display_height(disp);

    /* Save GL state we touch (minimal) */
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    /* Left=0, Right=dw, Top=0, Bottom=dh (y grows downward) */
    glOrtho(0.0, (double)dw, (double)dh, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* clear screen */
    al_clear_to_color(al_map_rgb(0,0,0));

    /* Draw background behind text */
    gl_fill_rect(bg_x, bg_y, bg_w, bg_h, bg_color);

    /* Draw progress bar fill */
    const float t = safe_progress(start_value, end_value, current_value);
    gl_fill_rect(bar_x, bar_y, bar_w, bar_height, bg_color);
    gl_fill_rect(bar_x, bar_y, bar_w * t, bar_height, bar_color);

    /* Draw surrounding border around text+bg+bar */
    gl_rect_outline(bg_x, bg_y, bg_w, bg_h, border_thickness, border_color);

    /* Restore matrices first (so Allegro text uses its expected state) */
    glPopMatrix();  // modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();

    /* Draw text on top using Allegro */
    /* Place baseline with padding inside the text background. */
    const float text_x = bg_x + padding;
    const float text_y = bg_y + padding;
    al_draw_text(font, text_color, text_x, text_y, 0, sentence);
    al_flip_display();
}

#ifdef __cplusplus
}
#endif

#endif
