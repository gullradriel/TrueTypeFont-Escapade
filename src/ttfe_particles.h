/**\file ttfe_particles.h
 *  particles / projectiles helpers
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 18/12/2025
 */

#ifndef TTFE_PARTICLES_HEADER_FOR_HACKS
#define TTFE_PARTICLES_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include "ttfe_game_context.h"
#include <allegro5/allegro_audio.h>

/* Particles, projectiles and box helpes */
void spawn_box_hit_particles(GameContext* ctx, Vec3 pos, int count, float size_scale);
/* spawn particles when hitting a wall */
void spawn_wall_hit_particles(GameContext* ctx, Vec3 pos, int count);
/* spawn 'you win level' particles */
void spawn_celebration_particles(GameContext* ctx);


/* fire a projectile */
void fire_projectile(GameContext* ctx, ALLEGRO_SAMPLE* sfx_shoot, bool audio_ok, double bullet_speed);
/* update all the projectiles actives in the list, check collisions */
void update_projectiles(GameContext* ctx, float dt, ALLEGRO_SAMPLE* sfx_hit_level, ALLEGRO_SAMPLE* sfx_hit_bonus, bool audio_ok, int* level_boxes_hit, int* level_time_bonus_boxes, int* level_speed_bonus_boxes,float speed_bonus_increment, float speed_max_limit);


/* update particles position */
void update_particles(GameContext* ctx, float gravity,float dt);
/* update pink lights position */
void update_pink_lights(GameContext* ctx, float dt);

/* render bonus boxes */
void render_boxes(GameContext* ctx);
/* render particles */
void render_particles(GameContext* ctx, Vec3 cam_right, Vec3 cam_up);
/* render projectiles */
void render_projectiles(GameContext* ctx);
/* render snow */
void render_intro_snow(GameContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
