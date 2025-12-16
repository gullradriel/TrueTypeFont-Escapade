/**\file ttfe_color.h
 *  Colors helpers
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef TTFE_COLOR_HEADER_FOR_HACKS
#define TTFE_COLOR_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

/* simple rainbow helper for color-cycling (t in radians/time) */
static inline ALLEGRO_COLOR rainbow_color(float t, float alpha) {
    float r = 0.5f + 0.5f * sinf(t);
    float g = 0.5f + 0.5f * sinf(t + 2.0f * (float)M_PI / 3.0f);
    float b = 0.5f + 0.5f * sinf(t + 4.0f * (float)M_PI / 3.0f);
    if (r < 0.0f)
        r = 0.0f;
    if (r > 1.0f)
        r = 1.0f;
    if (g < 0.0f)
        g = 0.0f;
    if (g > 1.0f)
        g = 1.0f;
    if (b < 0.0f)
        b = 0.0f;
    if (b > 1.0f)
        b = 1.0f;
    return al_map_rgba_f(r, g, b, alpha);
}

/* simple directional shading */
static inline ALLEGRO_COLOR shade_color(ALLEGRO_COLOR base, float nx, float ny, float nz) {
    float lx = 0.4f, ly = 1.0f, lz = 0.3f;
    float ln = sqrtf(lx * lx + ly * ly + lz * lz);
    lx /= ln;
    ly /= ln;
    lz /= ln;

    float d = nx * lx + ny * ly + nz * lz;
    if (d < 0.0f) d = 0.0f;
    float k = 0.25f + 0.75f * d;

    unsigned char r, g, b;
    al_unmap_rgb(base, &r, &g, &b);
    int ri = (int)(r * k);
    int gi = (int)(g * k);
    int bi = (int)(b * k);
    if (ri > 255) ri = 255;
    if (gi > 255) gi = 255;
    if (bi > 255) bi = 255;
    return al_map_rgb(ri, gi, bi);
}

#ifdef __cplusplus
}
#endif

#endif
