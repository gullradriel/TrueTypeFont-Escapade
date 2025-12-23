/**\file ttfe_entities.h
 *  Game entities: unified structures for game objects
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef TTFE_ENTITIES_HEADER
#define TTFE_ENTITIES_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "ttfe_vector3d.h"

/* ENTITY FLAGS (bit flags for entity types/states) */

typedef enum {
    ENTITY_FLAG_NONE = 0,
    ENTITY_FLAG_ACTIVE = (1 << 0),
    ENTITY_FLAG_TIME_BONUS = (1 << 1),
    ENTITY_FLAG_SPEED_BONUS = (1 << 2),
    ENTITY_FLAG_SCORE_BONUS = (1 << 3),
    ENTITY_FLAG_OBSTACLE = (1 << 4),
} EntityFlags;

/* UNIFIED GAME ENTITY, used for: Stars, Particles, Projectiles, Boxes, Pink Lights, Snowflakes */

typedef struct {
    Vec3 pos;      /* current position */
    Vec3 vel;      /* velocity (or prev_pos for projectiles) */
    Vec3 prev_pos; /* previous position (for collision interpolation) */
    ALLEGRO_COLOR color;
    float size;     /* visual size / half_size for boxes */
    float lifetime; /* lifetime remaining (or phase for lights/stars) */
    float phase;    /* animation phase offset */
    int hp;         /* entity life */
    int max_hp;     /* max life (helper to compute hp/max_hp) */
    uint32_t flags; /* EntityFlags bitfield */
} GameEntity;

/* ENTITY POOL - Generic pool management */

typedef struct {
    GameEntity* entities;
    int count;    /* number of entities in use */
    int capacity; /* maximum capacity */
} EntityPool;

/* init pool of entities */
void pool_init(EntityPool* pool, int capacity);
/* free pool of entities */
void pool_free(EntityPool* pool);
/* empty/clear a pool of entities */
void pool_clear(EntityPool* pool);
/* get entity at index in pool */
GameEntity* pool_get(EntityPool* pool, int index);
/* return true if entity is active */
bool entity_is_active(const GameEntity* e);
/* activate an entity in the pool */
void entity_activate(GameEntity* e);
/* deactivate an entity in the pool */
void entity_deactivate(GameEntity* e);
/* Find first inactive entity in pool, returns NULL if full */
GameEntity* pool_alloc(EntityPool* pool);
/* Count active entities */
int pool_active_count(const EntityPool* pool);

/* ENTITY FACTORY FUNCTIONS */

/* Create a star entity */
void entity_init_star(GameEntity* e, Vec3 pos, float size, ALLEGRO_COLOR color);
/* Create a particle entity */
void entity_init_particle(GameEntity* e, Vec3 pos, Vec3 vel, float lifetime, float size, ALLEGRO_COLOR color);
/* Create a projectile entity */
void entity_init_projectile(GameEntity* e, Vec3 pos, Vec3 vel, float lifetime);
/* Create a box entity */
void entity_init_box(GameEntity* e, Vec3 pos, float half_size, uint32_t bonus_flags);
/* Create a pink light entity */
void entity_init_pink_light(GameEntity* e, Vec3 pos, float radius);
/* Create a snowflake entity (2D screen space) */
void entity_init_snowflake(GameEntity* e, float x, float y, float vy, float size);
/* Create a moving obstacle box */
void entity_init_obstacle(GameEntity* e, Vec3 pos, Vec3 vel, float size);

/* ENTITY UPDATE FUNCTIONS */

/* Update particle with gravity */
bool entity_update_particle(GameEntity* e, float dt, float gravity);
/* Update projectile */
bool entity_update_projectile(GameEntity* e, float dt);
/* Update snowflake (2D) */
void entity_update_snowflake(GameEntity* e, float dt, float screen_height);

/*  ENTITY RENDERING HELPERS */

/* Add billboard quad for entity to vertex array */
void entity_add_billboard(const GameEntity* e, VertexArray* va, Vec3 cam_right, Vec3 cam_up);
/* Add box (cube) for entity to vertex array */
void entity_add_box(const GameEntity* e, VertexArray* va, ALLEGRO_COLOR shade_top);

/* COLLISION HELPERS */

/* Check if point is inside box entity */
bool entity_box_contains_point(const GameEntity* box, Vec3 point);

#ifdef __cplusplus
}
#endif

#endif
