/**\file ttfe_vbo.h
 *  3D VBO helpers
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 11/12/2025
 */

#ifndef TTFE_VBO_HEADER_FOR_HACKS
#define TTFE_VBO_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <string.h>

typedef struct {
    ALLEGRO_VERTEX_BUFFER* vb;
    int capacity;
} TTFE_VBO;

TTFE_VBO g_ttfe_stream_vbo;

/* init once after al_create_display */
static inline void ttfe_vbo_init(TTFE_VBO* vbo, int initial_cap) {
    if (initial_cap < 1) initial_cap = 1;
    vbo->capacity = initial_cap;
    vbo->vb = al_create_vertex_buffer(
        NULL, /* layout standard ALLEGRO_VERTEX */
        NULL,
        initial_cap,
        ALLEGRO_PRIM_BUFFER_DYNAMIC);
}

/* shutdown at the end */
static inline void ttfe_vbo_destroy(TTFE_VBO* vbo) {
    if (vbo->vb) al_destroy_vertex_buffer(vbo->vb);
    vbo->vb = NULL;
    vbo->capacity = 0;
}

/* check vbo cpacity */
static inline void ttfe_vbo_ensure(TTFE_VBO* vbo, int needed) {
    if (needed <= vbo->capacity) return;

    int newcap = vbo->capacity;
    while (newcap < needed) newcap *= 2;

    al_destroy_vertex_buffer(vbo->vb);
    vbo->vb = al_create_vertex_buffer(NULL, NULL, newcap,
                                      ALLEGRO_PRIM_BUFFER_DYNAMIC);
    vbo->capacity = newcap;
}

/* draw from a VertexArray */
static inline void ttfe_vbo_draw(
    TTFE_VBO* vbo,
    const ALLEGRO_VERTEX* verts,
    int count,
    int prim_type) {
    if (!verts || count <= 0) return;

    ttfe_vbo_ensure(vbo, count);

    void* dst = al_lock_vertex_buffer(
        vbo->vb, 0, count, ALLEGRO_LOCK_WRITEONLY);
    if (!dst) return;

    memcpy(dst, verts, sizeof(ALLEGRO_VERTEX) * count);
    al_unlock_vertex_buffer(vbo->vb);

    al_draw_vertex_buffer(vbo->vb, NULL, 0, count, prim_type);
}

#ifdef __cplusplus
}
#endif

#endif
