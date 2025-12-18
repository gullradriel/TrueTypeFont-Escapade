/**\file ttfe_vector3d.h
 *  3D level builder, camera and projections
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 18/12/2025
 */

#include "ttfe_level.h"
#include "ttfe_color.h"
#include "ttfe_loading.h"
#include "nilorea/n_log.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
void wasm_yield(void) {
    emscripten_sleep(0);
}
#else
void wasm_yield(void) {
    (void)0;
}
#endif

/* build a level from a font */
int build_level_geometry(GameContext* ctx, ALLEGRO_FONT* level_font, ALLEGRO_FONT *gui_font, const char* phrase, int phrase_len,int level_font_size) {
    int text_w = al_get_text_width(level_font, phrase);
    int text_h = al_get_font_line_height(level_font);

    int margin = level_font_size / 4;
    int bmp_w = text_w + margin * 2;
    int bmp_h = text_h + margin * 2;

    ALLEGRO_BITMAP* text_bmp = al_create_bitmap(bmp_w, bmp_h);
    if (!text_bmp) {
        n_log(LOG_ERR, "Failed to create text bitmap");
        return FALSE;
    }

    al_store_state(ctx->render_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER);
    al_set_target_bitmap(text_bmp);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0));
    al_draw_text(level_font, al_map_rgb(255, 255, 255),
                 bmp_w / 2.0f, (bmp_h - text_h) / 2.0f,
                 ALLEGRO_ALIGN_CENTRE, phrase);
    al_restore_state(ctx->render_state);

    const int STEP = 4;
    ctx->vf.gw = (bmp_w + STEP - 1) / STEP;
    ctx->vf.gh = (bmp_h + STEP - 1) / STEP;
    ctx->vf.cell_size = 3.0f;
    ctx->vf.extrude_h = 40.0f;
    ctx->vf.origin_x = -(float)ctx->vf.gw * ctx->vf.cell_size * 0.5f;
    ctx->vf.origin_z = -(float)ctx->vf.gh * ctx->vf.cell_size * 0.5f;
    ctx->vf.solid = (int*)calloc(ctx->vf.gw * ctx->vf.gh, sizeof(int));
    ctx->vf.is_goal = (int*)calloc(ctx->vf.gw * ctx->vf.gh, sizeof(int));

    /* Identify goal ranges */
    typedef struct {
        float x0, x1;
    } GoalRange;
    GoalRange goal_ranges[64];
    int goal_range_count = 0;

    float text_start_x = (float)(bmp_w - text_w) * 0.5f;
    char temp[2048];

    for (int i = 0; i < phrase_len && goal_range_count < 64; ++i) {
        if (i == (phrase_len - 1)) {
            int w_prev = 0;
            if (i > 0) {
                memcpy(temp, phrase, (size_t)i);
                temp[i] = '\0';
                w_prev = al_get_text_width(level_font, temp);
            }
            memcpy(temp, phrase, (size_t)(i + 1));
            temp[i + 1] = '\0';
            int w_curr = al_get_text_width(level_font, temp);

            goal_ranges[goal_range_count].x0 = text_start_x + (float)w_prev;
            goal_ranges[goal_range_count].x1 = text_start_x + (float)w_curr;
            goal_range_count++;
        }
        draw_text_box_with_progress("Loading assets...", gui_font, ctx->dw / 2, ctx->dh / 2 - 100,
                                    al_map_rgb(255, 255, 255),    /* text */
                                    al_map_rgba(20, 20, 20, 220), /* bg */
                                    al_map_rgb(255, 255, 255),    /* border */
                                    al_map_rgb(80, 200, 120),     /* bar */
                                    0, phrase_len, i);
    }

    /* Fill solid & is_goal */
    for (int gy = 0; gy < ctx->vf.gh; gy++) {
        for (int gx = 0; gx < ctx->vf.gw; gx++) {
            int px = gx * STEP + STEP / 2;
            int py = gy * STEP + STEP / 2;
            px = clampf(px, 0, bmp_w - 1);
            py = clampf(py, 0, bmp_h - 1);

            ALLEGRO_COLOR c = al_get_pixel(text_bmp, px, py);
            unsigned char r, g, b, a;
            al_unmap_rgba(c, &r, &g, &b, &a);

            if (a > 20) {
                int idx = gy * ctx->vf.gw + gx;
                ctx->vf.solid[idx] = 1;

                for (int dr = 0; dr < goal_range_count; ++dr) {
                    if ((float)px >= goal_ranges[dr].x0 && (float)px < goal_ranges[dr].x1) {
                        ctx->vf.is_goal[idx] = 1;
                        break;
                    }
                }
            }
        }
        draw_text_box_with_progress("Fill glyphs...", gui_font, ctx->dw / 2, ctx->dh / 2 - 100,
                                    al_map_rgb(255, 255, 255),    /* text */
                                    al_map_rgba(20, 20, 20, 220), /* bg */
                                    al_map_rgb(255, 255, 255),    /* border */
                                    al_map_rgb(80, 200, 120),     /* bar */
                                    0, ctx->vf.gh, gy);

        wasm_yield();
    }

    /* Build vertex arrays */
    va_clear(&ctx->va_level);
    va_clear(&ctx->va_overlay_letters);
    va_clear(&ctx->va_overlay_goals);

    ALLEGRO_COLOR base_letter = al_map_rgb(0x36, 0x01, 0x3f);
    ALLEGRO_COLOR base_goal = al_map_rgb(0x00, 0xff, 0x00);
    ALLEGRO_COLOR dummy = al_map_rgba(0, 0, 0, 0);

    for (int gy = 0; gy < ctx->vf.gh; gy++) {
        for (int gx = 0; gx < ctx->vf.gw; gx++) {
            if (!is_solid(&ctx->vf, gx, gy)) continue;

            int idx = gy * ctx->vf.gw + gx;
            bool isgoal = ctx->vf.is_goal[idx] != 0;
            ALLEGRO_COLOR base = isgoal ? base_goal : base_letter;

            float x0 = ctx->vf.origin_x + gx * ctx->vf.cell_size;
            float x1 = x0 + ctx->vf.cell_size;
            float z0 = ctx->vf.origin_z + gy * ctx->vf.cell_size;
            float z1 = z0 + ctx->vf.cell_size;
            float y0 = 0.0f;
            float y1 = ctx->vf.extrude_h;

            ALLEGRO_COLOR ctop = shade_color(base, 0.0f, 1.0f, 0.0f);
            ALLEGRO_COLOR cbottom = shade_color(base, 0.0f, -1.0f, 0.0f);

            /* Base geometry */
            va_add_quad(&ctx->va_level, x0, y1, z0, x1, y1, z0, x1, y1, z1, x0, y1, z1, ctop);
            va_add_quad(&ctx->va_level, x0, y0, z1, x1, y0, z1, x1, y0, z0, x0, y0, z0, cbottom);

            /* Overlay geometry */
            VertexArray* overlay = isgoal ? &ctx->va_overlay_goals : &ctx->va_overlay_letters;
            va_add_quad(overlay, x0, y1, z0, x1, y1, z0, x1, y1, z1, x0, y1, z1, dummy);
            va_add_quad(overlay, x0, y0, z1, x1, y0, z1, x1, y0, z0, x0, y0, z0, dummy);

            /* Side faces */
            if (!is_solid(&ctx->vf, gx + 1, gy)) {
                ALLEGRO_COLOR c = shade_color(base, 1.0f, 0.0f, 0.0f);
                va_add_quad(&ctx->va_level, x1, y0, z0, x1, y0, z1, x1, y1, z1, x1, y1, z0, c);
                va_add_quad(overlay, x1, y0, z0, x1, y0, z1, x1, y1, z1, x1, y1, z0, dummy);
            }
            if (!is_solid(&ctx->vf, gx - 1, gy)) {
                ALLEGRO_COLOR c = shade_color(base, -1.0f, 0.0f, 0.0f);
                va_add_quad(&ctx->va_level, x0, y0, z1, x0, y0, z0, x0, y1, z0, x0, y1, z1, c);
                va_add_quad(overlay, x0, y0, z1, x0, y0, z0, x0, y1, z0, x0, y1, z1, dummy);
            }
            if (!is_solid(&ctx->vf, gx, gy + 1)) {
                ALLEGRO_COLOR c = shade_color(base, 0.0f, 0.0f, 1.0f);
                va_add_quad(&ctx->va_level, x0, y0, z1, x1, y0, z1, x1, y1, z1, x0, y1, z1, c);
                va_add_quad(overlay, x0, y0, z1, x1, y0, z1, x1, y1, z1, x0, y1, z1, dummy);
            }
            if (!is_solid(&ctx->vf, gx, gy - 1)) {
                ALLEGRO_COLOR c = shade_color(base, 0.0f, 0.0f, -1.0f);
                va_add_quad(&ctx->va_level, x1, y0, z0, x0, y0, z0, x0, y1, z0, x1, y1, z0, c);
                va_add_quad(overlay, x1, y0, z0, x0, y0, z0, x0, y1, z0, x1, y1, z0, dummy);
            }
        }
        draw_text_box_with_progress("Build vertex arrays...", gui_font, ctx->dw / 2, ctx->dh / 2 - 100,
                                    al_map_rgb(255, 255, 255),    /* text */
                                    al_map_rgba(20, 20, 20, 220), /* bg */
                                    al_map_rgb(255, 255, 255),    /* border */
                                    al_map_rgb(80, 200, 120),     /* bar */
                                    0, ctx->vf.gh, gy);

        wasm_yield();
    }

    al_destroy_bitmap(text_bmp);
    return TRUE;
}


/* place bonus boxes and 'lights' */
void place_boxes_and_lights(GameContext* ctx) {
    /* Collect walkable cells */
    int max_cells = ctx->vf.gw * ctx->vf.gh;
    WalkCell* cells = (WalkCell*)malloc(sizeof(WalkCell) * max_cells);
    int cell_count = 0;

    for (int gy = 0; gy < ctx->vf.gh; ++gy) {
        for (int gx = 0; gx < ctx->vf.gw; ++gx) {
            if (is_solid(&ctx->vf, gx, gy)) {
                cells[cell_count].gx = gx;
                cells[cell_count].gy = gy;
                cell_count++;
            }
        }
    }

    if (cell_count > 0) {
        /* Shuffle cells */
        for (int i = cell_count - 1; i > 0; --i) {
            int j = rand() % (i + 1);
            WalkCell tmp = cells[i];
            cells[i] = cells[j];
            cells[j] = tmp;
        }

        int desired_lights = cell_count / 6;
        desired_lights = desired_lights < 4 ? 4 : desired_lights;
        desired_lights = desired_lights > PINK_LIGHT_MAX ? PINK_LIGHT_MAX : desired_lights;

        int desired_boxes = cell_count / 4;
        desired_boxes = desired_boxes < 8 ? 8 : desired_boxes;
        desired_boxes = desired_boxes > MAX_BOXES ? MAX_BOXES : desired_boxes;

        /* Place pink lights */
        for (int i = 0; i < desired_lights; ++i) {
            WalkCell c = cells[i];
            float cx = ctx->vf.origin_x + (c.gx + 0.5f) * ctx->vf.cell_size;
            float cz = ctx->vf.origin_z + (c.gy + 0.5f) * ctx->vf.cell_size;
            float cy = ctx->vf.extrude_h + frandf(0.5f * ctx->vf.extrude_h, 3.0f * ctx->vf.extrude_h);

            GameEntity* light = pool_alloc(&ctx->pink_lights);
            if (light) {
                entity_init_pink_light(light, v_make(cx, cy, cz),
                                       ctx->vf.cell_size * frandf(0.6f, 1.5f));
            }
        }

        /* Place boxes */
        int offset = desired_lights;
        for (int i = 0; i < desired_boxes && (offset + i) < cell_count; ++i) {
            WalkCell c = cells[offset + i];
            float cx = ctx->vf.origin_x + (c.gx + 0.5f) * ctx->vf.cell_size;
            float cz = ctx->vf.origin_z + (c.gy + 0.5f) * ctx->vf.cell_size;
            float half_size = ctx->vf.cell_size * 0.4f;

            GameEntity* box = pool_alloc(&ctx->boxes);
            if (box) {
                int r = rand() % 4;
                uint32_t bonus_flags = 0;
                if (r == 0)
                    bonus_flags = ENTITY_FLAG_TIME_BONUS;
                else if (r == 1)
                    bonus_flags = ENTITY_FLAG_SPEED_BONUS;

                entity_init_box(box, v_make(cx, ctx->vf.extrude_h + half_size, cz),
                                half_size, bonus_flags);
            }
        }
    }

    free(cells);
}

/* set the camera orientation to point to the end of the level */
void setup_camera_start(GameContext* ctx) {
    int gx_first = ctx->vf.gw;
    int gy_min_col = ctx->vf.gh;
    int gy_max_col = -1;

    for (int gx = 0; gx < ctx->vf.gw; ++gx) {
        int local_min = ctx->vf.gh;
        int local_max = -1;
        for (int gy = 0; gy < ctx->vf.gh; ++gy) {
            if (is_solid(&ctx->vf, gx, gy)) {
                if (gy < local_min) local_min = gy;
                if (gy > local_max) local_max = gy;
            }
        }
        if (local_max >= 0) {
            gx_first = gx;
            gy_min_col = local_min;
            gy_max_col = local_max;
            break;
        }
    }

    if (gx_first == ctx->vf.gw) {
        gx_first = ctx->vf.gw / 2;
        gy_min_col = ctx->vf.gh / 3;
        gy_max_col = ctx->vf.gh * 2 / 3;
    }

    float gx_center_f = (float)gx_first + 0.5f;
    float gy_center_f = (float)(gy_min_col + gy_max_col) * 0.5f + 0.5f;

    ctx->cam.position.x = ctx->vf.origin_x + gx_center_f * ctx->vf.cell_size;
    ctx->cam.position.z = ctx->vf.origin_z + gy_center_f * ctx->vf.cell_size;
    ctx->cam.position.y = ctx->vf.extrude_h + ctx->cam_half_height + 0.1f;

    float dx = -ctx->cam.position.x;
    float dz = -ctx->cam.position.z;

    if (fabsf(dx) > 1e-3f || fabsf(dz) > 1e-3f) {
        ctx->cam.yaw = atan2f(dx, dz);
    } else {
        ctx->cam.yaw = 0.0f;
    }
    ctx->cam.pitch = 0.0f;
}



