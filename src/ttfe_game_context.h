/**
 *\file ttfe_game_context.h
 *  Game context structures and helpers
 *\author Castagnier Mickael
 *\version 1.0
 *\date 16/12/2025
 */

#ifndef TTFE_GAME_CONTEXT_H
#define TTFE_GAME_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ttfe_vector3d.h"
#include "ttfe_entities.h"

#define STAR_COUNT 16384
#define MAX_BOXES 64
#define MAX_PROJECTILES 128
#define MAX_PARTICLES 2048
#define INTRO_SNOW_COUNT 400
#define PINK_LIGHT_MAX 256

/* Game states */
typedef enum {
    STATE_PLAY = 0,
    STATE_LEVEL_END,
    STATE_PARTY_END
} GameState;

typedef enum {
    PARTY_UNDECIDED = 0,
    PARTY_SUCCESS,
    PARTY_FAILED
} PartyResult;

typedef struct {
    /* Entity pools */
    EntityPool stars;
    EntityPool boxes;
    EntityPool projectiles;
    EntityPool particles;
    EntityPool pink_lights;
    EntityPool intro_snow;

    /* Vertex arrays for rendering */
    VertexArray va_stars;
    VertexArray va_boxes;
    VertexArray va_particles;
    VertexArray va_pink_lights;
    VertexArray va_projectiles;
    VertexArray va_level;
    VertexArray va_overlay_letters;
    VertexArray va_overlay_goals;

    /* Game state */
    GameState state;
    PartyResult party_result;
    Camera cam;

    int score;
    int total_score;
    float time_remaining;
    float move_speed;
    float max_speed;
    float move_forward;
    float move_lateral;

    bool gravity_enabled;
    bool on_ground;
    float vertical_vel;

    bool paused;
    bool mouse_locked;
    bool cheat_code_used;

    /* Level info */
    VoxelField vf;
    int level_index;
    int level_count;

    /* Physics constants */
    float cam_radius;
    float cam_half_height;

    /* Display info */
    int dw, dh;
    int center_x, center_y;

    /* Render state storage */
    ALLEGRO_STATE* render_state;

    /* Display */
    ALLEGRO_DISPLAY* display;
    int pending_w;
    int pending_h;
    bool pending_resize;

} GameContext;

void game_context_init(GameContext* ctx, float base_move_speed);
void game_context_free(GameContext* ctx);
void game_context_reset_level(GameContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
