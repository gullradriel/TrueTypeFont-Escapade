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
Vec3 v_add(Vec3 a, Vec3 b);
Vec3 v_sub(Vec3 a, Vec3 b);
Vec3 v_scale(Vec3 a, float s);
float v_dot(Vec3 a, Vec3 b);
Vec3 v_cross(Vec3 a, Vec3 b);
float v_norm(Vec3 a);
Vec3 v_normalize(Vec3 a);
Vec3 v_zero(void);
Vec3 v_make(float x, float y, float z);

/*
 * UTILITY FUNCTIONS
 */

float frandf(float min_val, float max_val);
float clampf(float val, float min_val, float max_val);

/*
 * CAMERA
 */

typedef struct {
    Vec3 position;
    float yaw;
    float pitch;
    float vertical_fov; /* radians */
} Camera;

Vec3 camera_forward(const Camera* cam);
Vec3 camera_right(const Camera* cam);
Vec3 camera_up(const Camera* cam);

/* Projection similar to Allegro ex_camera.c */
void setup_3d_projection(float vertical_fov, float z_near, float z_far);

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

void world_to_grid(const VoxelField* vf, float x, float z, int* gx, int* gy);
int is_solid(const VoxelField* vf, int gx, int gy);
/* Capsule (vertical cylinder) vs voxel grid collision */
bool capsule_collides(const VoxelField* vf, Vec3 pos, float radius, float half_height);
/* Capsule (vertical cylinder) vs AABB collision */
bool capsule_aabb_collides(Vec3 pos, float radius, float half_height, Vec3 box_pos, float b_half);

/*
 * VERTEX ARRAY (Dynamic)
 */

typedef struct {
    ALLEGRO_VERTEX* v;
    int count;
    int capacity;
} VertexArray;

void va_init(VertexArray* va, int initial_capacity);
void va_free(VertexArray* va);
void va_clear(VertexArray* va);
void va_reserve(VertexArray* va, int extra);
void va_push_vertex(VertexArray* va, float x, float y, float z, float u, float v, ALLEGRO_COLOR color);

void va_add_quad(VertexArray* va,
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
                 ALLEGRO_COLOR color);
void vbo_draw(TTFE_VBO* vbo, const VertexArray* va, int type);

#ifdef __cplusplus
}
#endif

#endif
