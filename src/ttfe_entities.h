/**\file ttfe_entities.h
 *  Game entities: unified structures for game objects
 *\author Castagnier Mickael
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef TTFE_ENTITIES_HEADER
#define TTFE_ENTITIES_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "ttfe_vector3d.h"

/*
 * ENTITY FLAGS (bit flags for entity types/states)
 */

typedef enum {
    ENTITY_FLAG_NONE = 0,
    ENTITY_FLAG_ACTIVE = (1 << 0),
    ENTITY_FLAG_TIME_BONUS = (1 << 1),
    ENTITY_FLAG_SPEED_BONUS = (1 << 2),
    ENTITY_FLAG_SCORE_BONUS = (1 << 3),
} EntityFlags;

/*
 * UNIFIED GAME ENTITY
 * Used for: Stars, Particles, Projectiles, Boxes, Pink Lights, Snowflakes
 */

typedef struct {
    Vec3 pos;      /* current position */
    Vec3 vel;      /* velocity (or prev_pos for projectiles) */
    Vec3 prev_pos; /* previous position (for collision interpolation) */
    ALLEGRO_COLOR color;
    float size;     /* visual size / half_size for boxes */
    float life;     /* lifetime remaining (or phase for lights/stars) */
    float phase;    /* animation phase offset */
    uint32_t flags; /* EntityFlags bitfield */
} GameEntity;

/*
 * ENTITY POOL - Generic pool management
 */

typedef struct {
    GameEntity* entities;
    int count;    /* number of entities in use */
    int capacity; /* maximum capacity */
} EntityPool;

static inline void pool_init(EntityPool* pool, int capacity) {
    pool->entities = (GameEntity*)calloc(capacity, sizeof(GameEntity));
    pool->count = 0;
    pool->capacity = capacity;
}

static inline void pool_free(EntityPool* pool) {
    free(pool->entities);
    pool->entities = NULL;
    pool->count = pool->capacity = 0;
}

static inline void pool_clear(EntityPool* pool) {
    for (int i = 0; i < pool->capacity; ++i) {
        pool->entities[i].flags = ENTITY_FLAG_NONE;
    }
    pool->count = 0;
}

static inline GameEntity* pool_get(EntityPool* pool, int index) {
    if (index < 0 || index >= pool->capacity) return NULL;
    return &pool->entities[index];
}

static inline bool entity_is_active(const GameEntity* e) {
    return (e->flags & ENTITY_FLAG_ACTIVE) != 0;
}

static inline void entity_activate(GameEntity* e) {
    e->flags |= ENTITY_FLAG_ACTIVE;
}

static inline void entity_deactivate(GameEntity* e) {
    e->flags &= ~ENTITY_FLAG_ACTIVE;
}

/* Find first inactive entity in pool, returns NULL if full */
static inline GameEntity* pool_alloc(EntityPool* pool) {
    for (int i = 0; i < pool->capacity; ++i) {
        if (!entity_is_active(&pool->entities[i])) {
            pool->count++;
            return &pool->entities[i];
        }
    }
    return NULL;
}

/* Count active entities */
static inline int pool_active_count(const EntityPool* pool) {
    int count = 0;
    for (int i = 0; i < pool->capacity; ++i) {
        if (entity_is_active(&pool->entities[i]))
            count++;
    }
    return count;
}

/*
 * ENTITY FACTORY FUNCTIONS
 */

/* Create a star entity */
static inline void entity_init_star(GameEntity* e, Vec3 pos, float size, ALLEGRO_COLOR color) {
    e->pos = pos;
    e->vel = v_zero();
    e->prev_pos = pos;
    e->color = color;
    e->size = size;
    e->life = 0.0f;
    e->phase = frandf(0.0f, 6.2831853f);
    e->flags = ENTITY_FLAG_ACTIVE;
}

/* Create a particle entity */
static inline void entity_init_particle(GameEntity* e, Vec3 pos, Vec3 vel, float life, float size, ALLEGRO_COLOR color) {
    e->pos = pos;
    e->vel = vel;
    e->prev_pos = pos;
    e->color = color;
    e->size = size;
    e->life = life;
    e->phase = 0.0f;
    e->flags = ENTITY_FLAG_ACTIVE;
}

/* Create a projectile entity */
static inline void entity_init_projectile(GameEntity* e, Vec3 pos, Vec3 vel, float life) {
    e->pos = pos;
    e->vel = vel;
    e->prev_pos = pos;
    e->color = al_map_rgb(255, 200, 200);
    e->size = 0.05f;
    e->life = life;
    e->phase = 0.0f;
    e->flags = ENTITY_FLAG_ACTIVE;
}

/* Create a box entity */
static inline void entity_init_box(GameEntity* e, Vec3 pos, float half_size, uint32_t bonus_flags) {
    e->pos = pos;
    e->vel = v_zero();
    e->prev_pos = pos;
    e->size = half_size;
    e->life = 0.0f;
    e->phase = 0.0f;
    e->flags = ENTITY_FLAG_ACTIVE | bonus_flags;

    /* Set color based on bonus type */
    if (bonus_flags & ENTITY_FLAG_TIME_BONUS)
        e->color = al_map_rgb(0xff, 0xff, 0x40); /* yellow -> time */
    else if (bonus_flags & ENTITY_FLAG_SPEED_BONUS)
        e->color = al_map_rgb(0x40, 0xff, 0xff); /* cyan -> speed */
    else
        e->color = al_map_rgb(0xff, 0xff, 0xff); /* white -> score */
}

/* Create a pink light entity */
static inline void entity_init_pink_light(GameEntity* e, Vec3 pos, float radius) {
    e->pos = pos;
    e->vel = v_zero();
    e->prev_pos = pos;
    e->color = al_map_rgba(0xff, 0x60, 0xff, 180);
    e->size = radius;
    e->life = 0.0f;
    e->phase = frandf(0.0f, 6.2831853f);
    e->flags = ENTITY_FLAG_ACTIVE;
}

/* Create a snowflake entity (2D screen space) */
static inline void entity_init_snowflake(GameEntity* e, float x, float y, float vy, float size) {
    e->pos = v_make(x, y, 0.0f);
    e->vel = v_make(0.0f, vy, 0.0f);
    e->prev_pos = e->pos;
    e->color = al_map_rgb(255, 255, 255);
    e->size = size;
    e->life = 0.0f;
    e->phase = 0.0f;
    e->flags = ENTITY_FLAG_ACTIVE;
}

/*
 * ENTITY UPDATE FUNCTIONS
 */

/* Update particle with gravity */
static inline bool entity_update_particle(GameEntity* e, float dt, float gravity) {
    if (!entity_is_active(e)) return false;

    e->pos = v_add(e->pos, v_scale(e->vel, dt));
    e->vel.y += gravity * 0.5f * dt;
    e->life -= dt;

    if (e->life <= 0.0f) {
        entity_deactivate(e);
        return false;
    }
    return true;
}

/* Update projectile */
static inline bool entity_update_projectile(GameEntity* e, float dt) {
    if (!entity_is_active(e)) return false;

    e->prev_pos = e->pos;
    e->pos = v_add(e->pos, v_scale(e->vel, dt));
    e->life -= dt;

    if (e->life <= 0.0f) {
        entity_deactivate(e);
        return false;
    }
    return true;
}

/* Update snowflake (2D) */
static inline void entity_update_snowflake(GameEntity* e, float dt, float screen_height) {
    if (!entity_is_active(e)) return;

    e->pos.y += e->vel.y * dt;

    if (e->pos.y - e->size > screen_height) {
        e->pos.y = frandf(-screen_height * 0.5f, 0.0f);
        e->pos.x = frandf(0.0f, screen_height * 1.6f); /* approximate width */
        e->vel.y = frandf(30.0f, 80.0f);
        e->size = frandf(2.0f, 6.0f);
    }
}

/*
 * ENTITY RENDERING HELPERS
 */

/* Add billboard quad for entity to vertex array */
static inline void entity_add_billboard(const GameEntity* e, VertexArray* va, Vec3 cam_right, Vec3 cam_up) {
    if (!entity_is_active(e)) return;

    Vec3 right = v_scale(cam_right, e->size);
    Vec3 up = v_scale(cam_up, e->size);

    Vec3 p0 = v_add(e->pos, v_add(v_scale(right, -1.0f), v_scale(up, -1.0f)));
    Vec3 p1 = v_add(e->pos, v_add(right, v_scale(up, -1.0f)));
    Vec3 p2 = v_add(e->pos, v_add(right, up));
    Vec3 p3 = v_add(e->pos, v_add(v_scale(right, -1.0f), up));

    va_reserve(va, 6);
    ALLEGRO_VERTEX* v = va->v + va->count;

    v[0] = (ALLEGRO_VERTEX){p0.x, p0.y, p0.z, 0, 0, e->color};
    v[1] = (ALLEGRO_VERTEX){p1.x, p1.y, p1.z, 0, 0, e->color};
    v[2] = (ALLEGRO_VERTEX){p2.x, p2.y, p2.z, 0, 0, e->color};
    v[3] = (ALLEGRO_VERTEX){p0.x, p0.y, p0.z, 0, 0, e->color};
    v[4] = (ALLEGRO_VERTEX){p2.x, p2.y, p2.z, 0, 0, e->color};
    v[5] = (ALLEGRO_VERTEX){p3.x, p3.y, p3.z, 0, 0, e->color};

    va->count += 6;
}

/* Add box (cube) for entity to vertex array */
static inline void entity_add_box(const GameEntity* e, VertexArray* va, ALLEGRO_COLOR shade_top) {
    if (!entity_is_active(e)) return;

    float hs = e->size;
    float x = e->pos.x;
    float y = e->pos.y;
    float z = e->pos.z;
    ALLEGRO_COLOR c = e->color;

    va_reserve(va, 36);
    ALLEGRO_VERTEX* v = va->v + va->count;
    int idx = 0;

    /* top */
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z - hs, 0, 0, shade_top};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z - hs, 0, 0, shade_top};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z + hs, 0, 0, shade_top};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z - hs, 0, 0, shade_top};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z + hs, 0, 0, shade_top};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z + hs, 0, 0, shade_top};

    /* bottom */
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z - hs, 0, 0, c};

    /* +X */
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z - hs, 0, 0, c};

    /* -X */
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z + hs, 0, 0, c};

    /* +Z */
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z + hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z + hs, 0, 0, c};

    /* -Z */
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y - hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x - hs, y + hs, z - hs, 0, 0, c};
    v[idx++] = (ALLEGRO_VERTEX){x + hs, y + hs, z - hs, 0, 0, c};

    va->count += 36;
}

/*
 * COLLISION HELPERS
 */

/* Check if point is inside box entity */
static inline bool entity_box_contains_point(const GameEntity* box, Vec3 point) {
    if (!entity_is_active(box)) return false;

    float hs = box->size;
    return (point.x >= box->pos.x - hs && point.x <= box->pos.x + hs &&
            point.y >= box->pos.y - hs && point.y <= box->pos.y + hs &&
            point.z >= box->pos.z - hs && point.z <= box->pos.z + hs);
}

#ifdef __cplusplus
}
#endif

#endif
