/**\file ttfe_vector3d.h
 *  3D vector, voxel and camera helpers
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 04/12/2025
 */

#ifndef TTFE_VECTOR3D_HEADER_FOR_HACKS
#define TTFE_VECTOR3D_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h> /* offsetof */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "ttfe_vbo.h"

/*
 * BASIC 3D VECTOR
 */

typedef struct {
    float x, y, z;
} Vec3;

/* Vector operations */
Vec3 v_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 v_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 v_scale(Vec3 a, float s) {
    return (Vec3){a.x * s, a.y * s, a.z * s};
}

float v_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 v_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

float v_norm(Vec3 a) {
    return sqrtf(v_dot(a, a));
}

Vec3 v_normalize(Vec3 a) {
    float n = v_norm(a);
    if (n <= 1e-6f) return a;
    return v_scale(a, 1.0f / n);
}

Vec3 v_zero(void) {
    return (Vec3){0.0f, 0.0f, 0.0f};
}

Vec3 v_make(float x, float y, float z) {
    return (Vec3){x, y, z};
}

/*
 * UTILITY FUNCTIONS
 */

static inline float frandf(float min_val, float max_val) {
    return min_val + (float)rand() / (float)RAND_MAX * (max_val - min_val);
}

static inline float clampf(float val, float min_val, float max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

/*
 * CAMERA
 */

typedef struct {
    Vec3 position;
    float yaw;
    float pitch;
    float vertical_fov; /* radians */
} Camera;

static inline Vec3 camera_forward(const Camera* cam) {
    return (Vec3){
        sinf(cam->yaw) * cosf(cam->pitch),
        sinf(cam->pitch),
        cosf(cam->yaw) * cosf(cam->pitch)};
}

static inline Vec3 camera_right(const Camera* cam) {
    Vec3 forward = camera_forward(cam);
    Vec3 world_up = {0.0f, 1.0f, 0.0f};
    return v_normalize(v_cross(forward, world_up));
}

static inline Vec3 camera_up(const Camera* cam) {
    Vec3 forward = camera_forward(cam);
    Vec3 right = camera_right(cam);
    return v_normalize(v_cross(right, forward));
}

/* Projection similar to Allegro ex_camera.c */
static inline void setup_3d_projection(float vertical_fov, float z_near, float z_far) {
    ALLEGRO_TRANSFORM projection;
    ALLEGRO_DISPLAY* display = al_get_current_display();
    double dw = al_get_display_width(display);
    double dh = al_get_display_height(display);

    if (dh <= 0 || dw <= 0) return;

    double f = tan(vertical_fov / 2.0);

    al_identity_transform(&projection);
    al_translate_transform_3d(&projection, 0, 0, -z_near);
    al_perspective_transform(
        &projection,
        -1.0 * dw / dh * f,
        f,
        z_near,
        f * dw / dh,
        -f,
        z_far);
    al_use_projection_transform(&projection);
}

/*
 * VOXEL FIELD
 */

typedef struct {
    int gw, gh;               /* grid width / height */
    float cell_size;          /* world size of one cell */
    float extrude_h;          /* height of extrusion */
    float origin_x, origin_z; /* world coord of cell (0,0) left/back corner */
    int* solid;               /* gw*gh, 0 = empty, 1 = solid */
    int* is_goal;             /* gw*gh, 1 = goal character */
} VoxelField;

static inline void world_to_grid(const VoxelField* vf, float x, float z, int* gx, int* gy) {
    float fx = x / vf->cell_size + (float)vf->gw * 0.5f;
    float fz = z / vf->cell_size + (float)vf->gh * 0.5f;
    *gx = (int)floorf(fx);
    *gy = (int)floorf(fz);
}

static inline int is_solid(const VoxelField* vf, int gx, int gy) {
    if (gx < 0 || gx >= vf->gw || gy < 0 || gy >= vf->gh)
        return 0;
    return vf->solid[gy * vf->gw + gx] != 0;
}

/* Capsule (vertical cylinder) vs voxel grid collision */
static inline bool capsule_collides(const VoxelField* vf, Vec3 pos, float radius, float half_height) {
    float bottom = pos.y - half_height;
    float top = pos.y + half_height;

    if (top <= 0.0f || bottom >= vf->extrude_h)
        return false;

    float minx = pos.x - radius;
    float maxx = pos.x + radius;
    float minz = pos.z - radius;
    float maxz = pos.z + radius;

    int gx_min, gy_min, gx_max, gy_max;
    world_to_grid(vf, minx, minz, &gx_min, &gy_min);
    world_to_grid(vf, maxx, maxz, &gx_max, &gy_max);

    if (gx_min > gx_max) {
        int t = gx_min;
        gx_min = gx_max;
        gx_max = t;
    }
    if (gy_min > gy_max) {
        int t = gy_min;
        gy_min = gy_max;
        gy_max = t;
    }

    gx_min = gx_min < 0 ? 0 : gx_min;
    gy_min = gy_min < 0 ? 0 : gy_min;
    gx_max = gx_max >= vf->gw ? vf->gw - 1 : gx_max;
    gy_max = gy_max >= vf->gh ? vf->gh - 1 : gy_max;

    float r2 = radius * radius;

    for (int gy = gy_min; gy <= gy_max; ++gy) {
        for (int gx = gx_min; gx <= gx_max; ++gx) {
            if (!is_solid(vf, gx, gy))
                continue;

            float x0 = vf->origin_x + gx * vf->cell_size;
            float x1 = x0 + vf->cell_size;
            float z0 = vf->origin_z + gy * vf->cell_size;
            float z1 = z0 + vf->cell_size;

            float nx = clampf(pos.x, x0, x1);
            float nz = clampf(pos.z, z0, z1);

            float dx = pos.x - nx;
            float dz = pos.z - nz;
            if (dx * dx + dz * dz <= r2)
                return true;
        }
    }
    return false;
}

/*
 * VERTEX ARRAY (Dynamic)
 */

typedef struct {
    ALLEGRO_VERTEX* v;
    int count;
    int capacity;
} VertexArray;

static inline void va_init(VertexArray* va, int initial_capacity) {
    va->count = 0;
    va->capacity = initial_capacity;
    va->v = (ALLEGRO_VERTEX*)malloc(sizeof(ALLEGRO_VERTEX) * initial_capacity);
}

static inline void va_free(VertexArray* va) {
    free(va->v);
    va->v = NULL;
    va->count = va->capacity = 0;
}

static inline void va_clear(VertexArray* va) {
    va->count = 0;
}

static inline void va_reserve(VertexArray* va, int extra) {
    if (va->count + extra <= va->capacity)
        return;
    int newcap = va->capacity * 2;
    if (newcap < va->count + extra)
        newcap = va->count + extra;
    va->v = (ALLEGRO_VERTEX*)realloc(va->v, sizeof(ALLEGRO_VERTEX) * newcap);
    va->capacity = newcap;
}

/*static inline void va_push_vertex(VertexArray* va, float x, float y, float z, float u, float v, ALLEGRO_COLOR color) {
    va_reserve(va, 1);
    ALLEGRO_VERTEX* vert = &va->v[va->count++];
    vert->x = x;
    vert->y = y;
    vert->z = z;
    vert->u = u;
    vert->v = v;
    vert->color = color;
}*/

static inline void va_add_quad(VertexArray* va,
                               float x1,
                               float y1,
                               float z1,
                               float x2,
                               float y2,
                               float z2,
                               float x3,
                               float y3,
                               float z3,
                               float x4,
                               float y4,
                               float z4,
                               ALLEGRO_COLOR color) {
    va_reserve(va, 6);
    ALLEGRO_VERTEX* v = va->v + va->count;

    v[0] = (ALLEGRO_VERTEX){x1, y1, z1, 0, 0, color};
    v[1] = (ALLEGRO_VERTEX){x2, y2, z2, 0, 0, color};
    v[2] = (ALLEGRO_VERTEX){x3, y3, z3, 0, 0, color};
    v[3] = (ALLEGRO_VERTEX){x1, y1, z1, 0, 0, color};
    v[4] = (ALLEGRO_VERTEX){x3, y3, z3, 0, 0, color};
    v[5] = (ALLEGRO_VERTEX){x4, y4, z4, 0, 0, color};

    va->count += 6;
}

static inline void vbo_draw(const VertexArray* va, int type) {
    if (!va || va->count < 0) return;
    ttfe_vbo_draw(&g_ttfe_stream_vbo, va->v, va->count, type);
}

#ifdef __cplusplus
}
#endif

#endif
