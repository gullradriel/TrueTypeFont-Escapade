/**\file ttfe_stars.c
 *  Stars helpers - now uses unified entity system
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 18/12/2025
 */

#include "ttfe_stars.h"

/* Generate starfield into entity pool */
void generate_starfield(EntityPool* pool, int count, float min_r, float max_r) {
    pool_clear(pool);

    for (int i = 0; i < count && i < pool->capacity; ++i) {
        float x, y, z;
        do {
            x = frandf(-1.0f, 1.0f);
            y = frandf(-1.0f, 1.0f);
            z = frandf(-1.0f, 1.0f);
        } while (x * x + y * y + z * z < 0.1f || x * x + y * y + z * z > 1.0f);

        Vec3 dir = v_normalize(v_make(x, y, z));
        float r = frandf(min_r, max_r);
        Vec3 pos = v_scale(dir, r);

        GameEntity* star = pool_alloc(pool);
        if (star) {
            entity_init_star(star, pos, frandf(2.0f, 5.0f), al_map_rgb(0x36, 0x01, 0x3f));
        }
    }
}

/* Render stars with twinkling effect */
void render_starfield(const EntityPool* pool, VertexArray* va, float light_phase) {
    va_clear(va);

    for (int i = 0; i < pool->capacity; ++i) {
        const GameEntity* star = &pool->entities[i];
        if (!entity_is_active(star)) continue;

        unsigned char r, g, b;
        al_unmap_rgb(star->color, &r, &g, &b);

        /* twinkling */
        float k = 0.5f + 0.5f * sinf(light_phase * 2.0f + star->phase);
        float s = 0.6f + 0.8f * k;

        int ri = (int)(r * s);
        if (ri > 255) ri = 255;
        int gi = (int)(g * s);
        if (gi > 255) gi = 255;
        int bi = (int)(b * s);
        if (bi > 255) bi = 255;

        ALLEGRO_COLOR c = al_map_rgb(ri, gi, bi);

        float size = star->size;
        float x = star->pos.x;
        float y = star->pos.y;
        float z = star->pos.z;

        float x0 = x - size, x1 = x + size;
        float y0 = y - size, y1 = y + size;

        va_reserve(va, 6);
        ALLEGRO_VERTEX* v = va->v + va->count;

        v[0] = (ALLEGRO_VERTEX){x0, y0, z, 0, 0, c};
        v[1] = (ALLEGRO_VERTEX){x1, y0, z, 0, 0, c};
        v[2] = (ALLEGRO_VERTEX){x1, y1, z, 0, 0, c};
        v[3] = (ALLEGRO_VERTEX){x0, y0, z, 0, 0, c};
        v[4] = (ALLEGRO_VERTEX){x1, y1, z, 0, 0, c};
        v[5] = (ALLEGRO_VERTEX){x0, y1, z, 0, 0, c};

        va->count += 6;
    }
}

/* Render pink lights with pulsing effect */
void render_pink_lights(const EntityPool* pool, VertexArray* va, Vec3 cam_right, Vec3 cam_up, float light_phase) {
    va_clear(va);

    for (int i = 0; i < pool->capacity; ++i) {
        const GameEntity* light = &pool->entities[i];
        if (!entity_is_active(light)) continue;

        float k = 0.5f + 0.5f * sinf(light_phase * 3.0f + light->phase);
        float size = light->size * (0.6f + 0.4f * k);

        Vec3 right = v_scale(cam_right, size);
        Vec3 up = v_scale(cam_up, size);

        Vec3 p0 = v_add(light->pos, v_add(v_scale(right, -1.0f), v_scale(up, -1.0f)));
        Vec3 p1 = v_add(light->pos, v_add(right, v_scale(up, -1.0f)));
        Vec3 p2 = v_add(light->pos, v_add(right, up));
        Vec3 p3 = v_add(light->pos, v_add(v_scale(right, -1.0f), up));

        va_reserve(va, 6);
        ALLEGRO_VERTEX* v = va->v + va->count;

        v[0] = (ALLEGRO_VERTEX){p0.x, p0.y, p0.z, 0, 0, light->color};
        v[1] = (ALLEGRO_VERTEX){p1.x, p1.y, p1.z, 0, 0, light->color};
        v[2] = (ALLEGRO_VERTEX){p2.x, p2.y, p2.z, 0, 0, light->color};
        v[3] = (ALLEGRO_VERTEX){p0.x, p0.y, p0.z, 0, 0, light->color};
        v[4] = (ALLEGRO_VERTEX){p2.x, p2.y, p2.z, 0, 0, light->color};
        v[5] = (ALLEGRO_VERTEX){p3.x, p3.y, p3.z, 0, 0, light->color};

        va->count += 6;
    }
}
