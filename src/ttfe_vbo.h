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

/* init once after al_create_display */
void ttfe_vbo_init(TTFE_VBO* vbo, int initial_cap);
/* shutdown at the end */
void ttfe_vbo_destroy(TTFE_VBO* vbo);
/* check vbo cpacity */
void ttfe_vbo_ensure(TTFE_VBO* vbo, int needed);
/* draw from a VertexArray */
void ttfe_vbo_draw(TTFE_VBO* vbo, const ALLEGRO_VERTEX* verts, int count, int prim_type);

#ifdef __cplusplus
}
#endif

#endif
