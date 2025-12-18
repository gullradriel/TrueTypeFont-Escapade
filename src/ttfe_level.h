/**\file ttfe_vector3d.h
 *  3D level builder, camera and projections
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 18/12/2025
 */

#ifndef TTFE_LEVEL_HEADER_FOR_HACKS
#define TTFE_LEVEL_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif


#include "ttfe_game_context.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

/* LEVEL BUILDING */

typedef struct {
    int gx, gy;
} WalkCell;

/* build a level from a font */
int build_level_geometry(GameContext* ctx, ALLEGRO_FONT* level_font, ALLEGRO_FONT *gui_font, const char* phrase, int phrase_len,int level_font_size);
/* place bonus boxes and 'lights' */
void place_boxes_and_lights(GameContext* ctx);
/* set the camera orientation to point to the end of the level */
void setup_camera_start(GameContext* ctx);


#ifdef __cplusplus
}
#endif

#endif
