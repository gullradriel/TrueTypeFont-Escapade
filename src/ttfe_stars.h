/**\file ttfe_stars.h
 *  Stars helpers - now uses unified entity system
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef TTFE_STAR_HEADER_FOR_HACKS
#define TTFE_STAR_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include "ttfe_entities.h"

/* Generate starfield into entity pool */
void generate_starfield(EntityPool* pool, int count, float min_r, float max_r);

/* Render stars with twinkling effect */
void render_starfield(const EntityPool* pool, VertexArray* va, float light_phase);

/* Render pink lights with pulsing effect */
void render_pink_lights(const EntityPool* pool, VertexArray* va, Vec3 cam_right, Vec3 cam_up, float light_phase);

#ifdef __cplusplus
}
#endif

#endif
