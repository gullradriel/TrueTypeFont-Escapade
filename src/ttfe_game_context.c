/**
 *\file ttfe_game_context.c
 *  Game context helpers implementation
 *\author Castagnier Mickael
 *\version 1.0
 *\date 16/12/2025
 */

#include <math.h>
#include <string.h>

#include "ttfe_game_context.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void game_context_init(GameContext* ctx, float base_move_speed) {
    memset(ctx, 0, sizeof(GameContext));

    /* Initialize entity pools */
    pool_init(&ctx->stars, STAR_COUNT);
    pool_init(&ctx->boxes, MAX_BOXES + MAX_HITTING_BOXES);
    pool_init(&ctx->projectiles, MAX_PROJECTILES);
    pool_init(&ctx->particles, MAX_PARTICLES);
    pool_init(&ctx->pink_lights, PINK_LIGHT_MAX);
    pool_init(&ctx->intro_snow, INTRO_SNOW_COUNT);

    /* Initialize vertex arrays */
    va_init(&ctx->va_stars, STAR_COUNT * 6);
    va_init(&ctx->va_boxes, (MAX_BOXES + MAX_HITTING_BOXES) * 36);
    va_init(&ctx->va_particles, MAX_PARTICLES * 6);
    va_init(&ctx->va_pink_lights, PINK_LIGHT_MAX * 6);
    va_init(&ctx->va_projectiles, MAX_PROJECTILES * 6);
    va_init(&ctx->va_level, 4096);
    va_init(&ctx->va_overlay_letters, 4096);
    va_init(&ctx->va_overlay_goals, 1024);

    /* Default values */
    ctx->state = STATE_PLAY;
    ctx->party_result = PARTY_UNDECIDED;
    ctx->gravity_enabled = true;
    ctx->on_ground = true;
    ctx->move_speed = base_move_speed;
    ctx->max_speed = base_move_speed;
    ctx->move_forward = 0.0f;
    ctx->move_lateral = 0.0f;
    ctx->mouse_locked = true;
    ctx->time_remaining = 60.0f;

    ctx->cam_radius = 3.0f * 0.4f;
    ctx->cam_half_height = 40.0f * 0.4f;
    ctx->cam.vertical_fov = (float)(60.0 * M_PI / 180.0);

    /* Render state */
    ctx->render_state = al_malloc(sizeof(ALLEGRO_STATE));

    /* Display */
    ctx->display = NULL;
    ctx->pending_w = 0;
    ctx->pending_h = 0;
    ctx->pending_resize = false;
    ctx->cheat_code_used = false;
}

void game_context_free(GameContext* ctx) {
    pool_free(&ctx->stars);
    pool_free(&ctx->boxes);
    pool_free(&ctx->projectiles);
    pool_free(&ctx->particles);
    pool_free(&ctx->pink_lights);
    pool_free(&ctx->intro_snow);

    va_free(&ctx->va_stars);
    va_free(&ctx->va_boxes);
    va_free(&ctx->va_particles);
    va_free(&ctx->va_pink_lights);
    va_free(&ctx->va_projectiles);
    va_free(&ctx->va_level);
    va_free(&ctx->va_overlay_letters);
    va_free(&ctx->va_overlay_goals);

    if (ctx->vf.solid) free(ctx->vf.solid);
    if (ctx->vf.is_goal) free(ctx->vf.is_goal);
    free(ctx->render_state);
}

void game_context_reset_level(GameContext* ctx) {
    pool_clear(&ctx->boxes);
    pool_clear(&ctx->projectiles);
    pool_clear(&ctx->particles);
    pool_clear(&ctx->pink_lights);

    ctx->state = STATE_PLAY;
    ctx->gravity_enabled = true;
    ctx->on_ground = true;
    ctx->vertical_vel = 0.0f;
    ctx->score = 0;
    ctx->time_remaining = 60.0f;
    ctx->paused = false;
}
