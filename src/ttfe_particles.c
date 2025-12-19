/**\file ttfe_particles.c
 *  particles / projectiles helpers
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 18/12/2025
 */

#include "ttfe_particles.h"
#include "ttfe_color.h"

/* Particles, projectiles and box helpes */

/* spawn particles when a box is hit helper */
void spawn_box_hit_particles(GameContext* ctx, Vec3 pos, int count, float size_scale) {
    for (int i = 0; i < count; ++i) {
        GameEntity* p = pool_alloc(&ctx->particles);
        if (!p) break;

        Vec3 vel = v_make(
            frandf(-10.0f, 10.0f),
            frandf(5.0f, 15.0f),
            frandf(-10.0f, 10.0f));

        ALLEGRO_COLOR color;
        int ccase = rand() % 4;
        if (ccase == 0)
            color = al_map_rgb(255, 0, 0);
        else if (ccase == 1)
            color = al_map_rgb(0, 255, 0);
        else if (ccase == 2)
            color = al_map_rgb(255, 255, 255);
        else
            color = al_map_rgb(255, 215, 0);

        entity_init_particle(p, pos, vel,
                             frandf(0.5f, 1.5f),
                             size_scale * frandf(0.01f, 0.2f), /* use size_scale */
                             color);
    }
}

/* spawn 'you win level' particles */
void spawn_celebration_particles(GameContext* ctx) {
    int bursts = rand() % 4;
    for (int bi = 0; bi < bursts; ++bi) {
        Vec3 center = v_make(
            ctx->cam.position.x + frandf(-20.0f, 20.0f),
            ctx->vf.extrude_h + frandf(5.0f, 25.0f),
            ctx->cam.position.z + frandf(-20.0f, 20.0f));

        int count = 20 + rand() % 40;
        spawn_box_hit_particles(ctx, center, count, 0.5f); /* smaller particles */
    }
}

/* spawn particles when hitting a wall */
void spawn_wall_hit_particles(GameContext* ctx, Vec3 pos, int count) {
    for (int i = 0; i < count; ++i) {
        GameEntity* p = pool_alloc(&ctx->particles);
        if (!p) break;

        Vec3 vel = v_make(
            frandf(-8.0f, 8.0f),
            frandf(-2.0f, 10.0f),
            frandf(-8.0f, 8.0f));

        ALLEGRO_COLOR color = al_map_rgb(200 + rand() % 55, 20 + rand() % 80, 100 + rand() % 80);

        entity_init_particle(p, pos, vel,
                             frandf(0.3f, 1.0f),
                             ctx->vf.cell_size * frandf(0.01f, 0.2f),
                             color);
    }
}

/* PROJECTILE MANAGEMENT */

/* fire a projectile */
void fire_projectile(GameContext* ctx, ALLEGRO_SAMPLE* sfx_shoot, bool audio_ok, double bullet_speed) {
    GameEntity* proj = pool_alloc(&ctx->projectiles);
    if (!proj) return;

    Vec3 dir = v_normalize(camera_forward(&ctx->cam));
    entity_init_projectile(proj, ctx->cam.position, v_scale(dir, bullet_speed + ctx->move_speed), 6.0f);

    if (audio_ok && sfx_shoot) {
        al_play_sample(sfx_shoot, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
}

/* update all the projectiles actives in the list */
void update_projectiles(GameContext* ctx, float dt, ALLEGRO_SAMPLE* sfx_hit_level, ALLEGRO_SAMPLE* sfx_hit_bonus, bool audio_ok, int* level_boxes_hit, int* level_time_bonus_boxes, int* level_speed_bonus_boxes, float speed_bonus_increment, float speed_max_limit) {
    for (int i = 0; i < ctx->projectiles.capacity; ++i) {
        GameEntity* proj = &ctx->projectiles.entities[i];
        if (!entity_update_projectile(proj, dt)) continue;

        bool hit_something = false;
        bool hit_bonus = false;

        Vec3 from = proj->prev_pos;
        Vec3 to = proj->pos;
        int steps = 4; /* or more if needed */

        /* go along trajectory */
        for (int i = 0; i <= steps; ++i) {
            float t = (float)i / (float)steps;
            Vec3 trajectory_point = v_add(from, v_scale(v_sub(to, from), t));

            /* Check box collisions */
            for (int b = 0; b < ctx->boxes.capacity && !hit_something; ++b) {
                GameEntity* box = &ctx->boxes.entities[b];
                if (!entity_is_active(box)) continue;

                if (entity_box_contains_point(box, trajectory_point)) {
                    entity_deactivate(box);
                    entity_deactivate(proj);
                    hit_something = true;
                    hit_bonus = true;

                    spawn_box_hit_particles(ctx, box->pos, 40, ctx->vf.cell_size);

                    if (box->flags & ENTITY_FLAG_TIME_BONUS) {
                        ctx->time_remaining += 30.0f;
                        (*level_time_bonus_boxes)++;
                        ctx->score += 15;
                    } else if (box->flags & ENTITY_FLAG_SPEED_BONUS) {
                        ctx->move_speed += speed_bonus_increment;
                        if (ctx->move_speed > speed_max_limit)
                            ctx->move_speed = speed_max_limit;
                        /* save max achieved move speed */
                        if (ctx->move_speed > ctx->max_speed)
                            ctx->max_speed = ctx->move_speed;
                        (*level_speed_bonus_boxes)++;
                        ctx->score += 15;
                    } else {
                        (*level_boxes_hit)++;
                        ctx->score += 100;
                    }
                    break;
                }
            }

            /* Check environment collision */
            if (!hit_something) {
                if (trajectory_point.y >= 0.0f && trajectory_point.y <= ctx->vf.extrude_h) {
                    int gx, gy;
                    world_to_grid(&ctx->vf, trajectory_point.x, trajectory_point.z, &gx, &gy);
                    if (is_solid(&ctx->vf, gx, gy)) {
                        entity_deactivate(proj);
                        hit_something = true;
                        hit_bonus = false;
                        spawn_wall_hit_particles(ctx, trajectory_point, 25);
                    }
                }
            }
        }

        if (hit_something && audio_ok) {
            if (hit_bonus && sfx_hit_bonus) {
                al_play_sample(sfx_hit_bonus, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
            } else if (sfx_hit_level) {
                al_play_sample(sfx_hit_level, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
        }
    }
}

/* PARTICLE UPDATE */

/* update particles position */
void update_particles(GameContext* ctx, float gravity, float dt) {
    for (int i = 0; i < ctx->particles.capacity; ++i) {
        entity_update_particle(&ctx->particles.entities[i], dt, gravity);
    }
}

/* update pink lights position */
void update_pink_lights(GameContext* ctx, float dt) {
    const float begin_x = ctx->vf.origin_x;                                /* beginning of level */
    const float end_x = ctx->vf.origin_x + ctx->vf.gw * ctx->vf.cell_size; /* end of level */

    for (int i = 0; i < ctx->pink_lights.capacity; ++i) {
        GameEntity* l = &ctx->pink_lights.entities[i];
        if (!entity_is_active(l)) continue;

        const float speed = l->vel.x;

        /* Move from end -> beginning (toward smaller x). Flip sign if your level direction is opposite. */
        l->pos.x -= speed * dt;

        /* If it went past the beginning, respawn back at the end */
        if (l->pos.x < begin_x - ctx->vf.cell_size) {
            l->pos.x = end_x + frandf(0.0f, 5.0f * ctx->vf.cell_size);
        }
    }
}

/* RENDERING FUNCTIONS */

/* render bonus boxes */
void render_boxes(GameContext* ctx) {
    va_clear(&ctx->va_boxes);

    for (int i = 0; i < ctx->boxes.capacity; ++i) {
        GameEntity* box = &ctx->boxes.entities[i];
        if (!entity_is_active(box)) continue;

        ALLEGRO_COLOR shade_top = shade_color(box->color, 0.0f, 1.0f, 0.0f);
        entity_add_box(box, &ctx->va_boxes, shade_top);
    }
    vbo_draw(&ctx->g_ttfe_stream_vbo, &ctx->va_boxes, ALLEGRO_PRIM_TRIANGLE_LIST);
}

/* render particles */
void render_particles(GameContext* ctx, Vec3 cam_right, Vec3 cam_up) {
    va_clear(&ctx->va_particles);

    for (int i = 0; i < ctx->particles.capacity; ++i) {
        GameEntity* p = &ctx->particles.entities[i];
        if (!entity_is_active(p)) continue;

        if (p->size <= 0.0f)
            p->size = ctx->vf.cell_size * frandf(0.01f, 0.2f);

        entity_add_billboard(p, &ctx->va_particles, cam_right, cam_up);
    }
    vbo_draw(&ctx->g_ttfe_stream_vbo, &ctx->va_particles, ALLEGRO_PRIM_TRIANGLE_LIST);
}

/* render projectiles */
void render_projectiles(GameContext* ctx) {
    Vec3 forward = camera_forward(&ctx->cam);
    Vec3 right = v_make(cosf(ctx->cam.yaw), 0.0f, -sinf(ctx->cam.yaw));
    right = v_normalize(right);
    Vec3 up = v_cross(right, forward);
    up = v_normalize(up);

    const float HALF_SIZE = 0.05f;
    Vec3 right_scaled = v_scale(right, HALF_SIZE);
    Vec3 up_scaled = v_scale(up, HALF_SIZE);

    for (int i = 0; i < ctx->projectiles.capacity; ++i) {
        GameEntity* proj = &ctx->projectiles.entities[i];
        if (!entity_is_active(proj)) continue;

        Vec3 p = proj->pos;
        Vec3 p0 = v_add(v_sub(p, right_scaled), up_scaled);
        Vec3 p1 = v_add(v_add(p, right_scaled), up_scaled);
        Vec3 p2 = v_sub(v_sub(p, right_scaled), up_scaled);
        Vec3 p3 = v_sub(v_add(p, right_scaled), up_scaled);

        ALLEGRO_VERTEX verts[4];
        verts[0] = (ALLEGRO_VERTEX){p0.x, p0.y, p0.z, 0.0f, 0.0f, al_map_rgb(255, 200, 200)};
        verts[1] = (ALLEGRO_VERTEX){p1.x, p1.y, p1.z, 1.0f, 0.0f, al_map_rgb(255, 200, 200)};
        verts[2] = (ALLEGRO_VERTEX){p2.x, p2.y, p2.z, 0.0f, 1.0f, al_map_rgb(255, 150, 150)};
        verts[3] = (ALLEGRO_VERTEX){p3.x, p3.y, p3.z, 1.0f, 1.0f, al_map_rgb(255, 150, 150)};

        al_draw_prim(verts, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);
    }
}

/* render snow */
void render_intro_snow(GameContext* ctx) {
    for (int i = 0; i < ctx->intro_snow.capacity; ++i) {
        GameEntity* snow = &ctx->intro_snow.entities[i];
        if (!entity_is_active(snow)) continue;

        al_draw_filled_circle(snow->pos.x, snow->pos.y, snow->size, snow->color);
    }
}
