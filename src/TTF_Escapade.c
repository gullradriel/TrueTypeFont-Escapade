/**\file TTF_Escapade.c
 *  Main game file - refactored to use unified entity system
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 02/12/2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "nilorea/n_common.h"
#include "nilorea/n_log.h"
#include "nilorea/n_str.h"
#include "ttfe_app_config.h"
#include "ttfe_vector3d.h"
#include "ttfe_entities.h"
#include "ttfe_color.h"
#include "ttfe_text.h"
#include "ttfe_stars.h"
#include "ttfe_loading.h"
#include "ttfe_game_context.h"
#include "ttfe_particles.h"
#include "ttfe_level.h"

/* GAME CONFIGURATION */

/* Tweakable Christmas effects */
int COLOR_CYCLE_GOAL = 1;
int PULSE_TEXT = 1;
int BLEND_TEXT = 1;

/* shared phase for animations */
float light_phase = 0.0f;

/* depth viewport for the camera */
const float Z_NEAR = 1.0f;
const float Z_FAR = 5000.0f;

/* GLOBAL CONFIGURATION (loaded from config file) */

long int WIDTH = 1280, HEIGHT = 800;
bool fullscreen = 0;
double fps = 60.0;
double logic = 120.0;

float pending_mdx = 0.0f;
float pending_mdy = 0.0f;
float mouse_sensitivity = 0.003f;

char* level_font_file = NULL;
char* override_level_font_file = NULL;
int level_font_size = 128;
ALLEGRO_FONT* level_font = NULL;
char* gui_font_file = NULL;
char* override_gui_font_file = NULL;
int gui_font_size = 22;
ALLEGRO_FONT* gui_font = NULL;

const char* songs_file = "DATA/songs.txt";
const char* intro_file = "DATA/intro.txt";

char* levels_file = NULL;
char* override_levels_file = NULL;

char* intro_sample = NULL;
char* win_sample = NULL;
char* falling_sample = NULL;
char* shoot_sample = NULL;
char* jump_sample = NULL;
char* hit_level_sample = NULL;
char* hit_bonus_sample = NULL;
char* game_over_sample = NULL;

ALLEGRO_SAMPLE* music_intro = NULL;
ALLEGRO_SAMPLE* music_win = NULL;
ALLEGRO_SAMPLE* sfx_falling = NULL;
ALLEGRO_SAMPLE* sfx_shoot = NULL;
ALLEGRO_SAMPLE* sfx_jump = NULL;
ALLEGRO_SAMPLE* sfx_hit_level = NULL;
ALLEGRO_SAMPLE* sfx_hit_bonus = NULL;
ALLEGRO_SAMPLE* sfx_game_over = NULL;

float gravity = -70.0f;
float jump_vel = 50.0f;
float base_speed = 0.8f;
float speed_bonus_increment = 0.2f;
float speed_max_limit = 3.0f;
float bullet_speed = 150.0f;
int bullet_delta_divider = 4;
float obstacle_spawn_timer = 0.0f;
float obstacle_spawn_delay = 4.0f; /* Une boite toutes les 2 secondes */

int getoptret = 0;

/* default log level if not set by user */
/* int log_level = LOG_DEBUG; */
int log_level = LOG_ERR;

bool do_draw = 1, do_logic = 1;

void usage(int log_level, char* progname) {
    n_log(log_level,
          "\n    %s usage:\n"
          "    -h => print help\n"
          "    -v => print version\n"
          "    -V LOGLEVEL => choose log level\n"
          "    -L logfile  => log to file\n"
          "    -f level_font_file\n"
          "    -g gui_font_file\n"
          "    -l levels_file\n",
          progname);
}

#include "ttfe_emscripten_mouse.h"
#include "ttfe_emscripten_fullscreen.h"

/*
 * MAIN FUNCTION
 */

int main(int argc, char** argv) {
    set_log_level(LOG_INFO);

    char ver_str[128] = "";

    while ((getoptret = getopt(argc, argv, "hvV:L:f:l:g:")) != EOF) {
        switch (getoptret) {
            case 'h':
                usage(LOG_INFO, argv[0]);
                exit(0);
            case 'v':
                sprintf(ver_str, "%s %s", __DATE__, __TIME__);
                exit(TRUE);
                break;
            case 'V':
                if (!strncmp("INFO", optarg, 6))
                    log_level = LOG_INFO;
                else if (!strncmp("NOTICE", optarg, 6))
                    log_level = LOG_NOTICE;
                else if (!strncmp("VERBOSE", optarg, 7))
                    log_level = LOG_NOTICE;
                else if (!strncmp("ERROR", optarg, 5))
                    log_level = LOG_ERR;
                else if (!strncmp("DEBUG", optarg, 5))
                    log_level = LOG_DEBUG;
                else {
                    n_log(LOG_ERR, "%s is not a valid log level", optarg);
                    exit(FALSE);
                }
                n_log(LOG_NOTICE, "LOG LEVEL UP TO: %d", log_level);
                set_log_level(log_level);
                break;
            case 'L':
                n_log(LOG_NOTICE, "LOG FILE: %s", optarg);
                set_log_file(optarg);
                break;
            case 'l':
                n_log(LOG_NOTICE, "LEVEL FILE: %s", optarg);
                override_levels_file = strdup(optarg);
                break;
            case 'f':
                n_log(LOG_NOTICE, "LEVEL FONT FILE: %s", optarg);
                override_level_font_file = strdup(optarg);
                break;
            case 'g':
                n_log(LOG_NOTICE, "GUI FONT FILE: %s", optarg);
                override_gui_font_file = strdup(optarg);
                break;
            case '?':
                if (optopt == 'V') {
                    n_log(LOG_ERR, "\nPlease specify a log level after -V.");
                } else if (optopt == 'L') {
                    n_log(LOG_ERR, "\nPlease specify a log file after -L");
                }
                __attribute__((fallthrough));
            default:
                usage(LOG_ERR, argv[0]);
                exit(FALSE);
        }
    }

    set_log_level(log_level);
#ifdef __EMSCRIPTEN__
    set_log_file_fd(stdout);
#endif

    /* Load config */
    if (load_app_config("DATA/app_config.json", &WIDTH, &HEIGHT, &fullscreen,
                        &intro_sample, &win_sample, &falling_sample, &shoot_sample, &jump_sample,
                        &hit_level_sample, &hit_bonus_sample, &game_over_sample,
                        &fps, &logic,
                        &level_font_file, &level_font_size, &gui_font_file, &gui_font_size, &levels_file,
                        &gravity, &jump_vel, &base_speed, &speed_bonus_increment, &speed_max_limit,
                        &mouse_sensitivity, &bullet_speed, &bullet_delta_divider) != TRUE) {
        n_log(LOG_ERR, "couldn't load app_config.json!");
        exit(1);
    }

    if (override_level_font_file) {
        Free(level_font_file);
        level_font_file = override_level_font_file;
    }
    if (override_gui_font_file) {
        Free(gui_font_file);
        gui_font_file = override_gui_font_file;
    }
    if (override_levels_file) {
        Free(levels_file);
        levels_file = override_levels_file;
    }

    srand((unsigned int)time(NULL));

    /* Allegro init */
    if (!al_init()) {
        fprintf(stderr, "al_init() failed\n");
        return FALSE;
    }

    al_install_keyboard();
    al_install_mouse();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();

    bool audio_ok = false;
    if (al_install_audio() && al_init_acodec_addon()) {
        if (al_reserve_samples(32)) {
            audio_ok = true;
        } else {
            n_log(LOG_ERR, "Failed to reserve 32 audio samples");
        }
    } else {
        n_log(LOG_ERR, "Failed to al_install_audio && al_init_acodec_addon");
    }

    al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);
    al_set_new_display_flags(ALLEGRO_RESIZABLE);
    al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);

    ALLEGRO_DISPLAY* display = al_create_display(WIDTH, HEIGHT);
    if (!display) {
        n_log(LOG_ERR, "Failed to create display");
        return FALSE;
    }

    if (fullscreen) {
        al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
        al_acknowledge_resize(display);
    }

    /* Initialize game context */
    GameContext ctx;
    game_context_init(&ctx, base_speed);
    ctx.display = display;
    ttfe_vbo_init(&ctx.g_ttfe_stream_vbo, 16382);

    al_set_window_title(display, "TrueTypeFont Escapade");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* fps_timer = al_create_timer(1.0 / fps);
    ALLEGRO_TIMER* logic_timer = al_create_timer(1.0 / logic);
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_timer_event_source(fps_timer));
    al_register_event_source(queue, al_get_timer_event_source(logic_timer));

#ifdef __EMSCRIPTEN__
    /* fullscreen state callback */
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, (void*)&ctx, EM_TRUE, on_fullscreen_change);

    /* Install browser callbacks (pointer lock + movement deltas) */
    web_init_pointer_lock(&ctx);
#endif

    ctx.dw = al_get_display_width(display);
    ctx.dh = al_get_display_height(display);
    ctx.center_x = ctx.dw / 2;
    ctx.center_y = ctx.dh / 2;

#ifndef __EMSCRIPTEN__
    al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
#else
    /* On Web, hiding/grabbing is handled by pointer lock. You can keep cursor visible until lock is active. */
    al_show_mouse_cursor(display);
#endif

    /* Load intro text */
    int intro_count = 0;
    char** intro_lines = load_text_file_lines(intro_file, &intro_count);

    /* Load levels */
    int level_count = 0;
    char** levels = load_text_file_lines(levels_file, &level_count);
    if (!levels) {
        game_context_free(&ctx);
        al_destroy_timer(fps_timer);
        al_destroy_timer(logic_timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return FALSE;
    }
    ctx.level_count = level_count;

    /* Load songs */
    int songs_count = 0;
    char** songs = NULL;
    if (audio_ok) {
        songs = load_text_file_lines(songs_file, &songs_count);
    }

    /* Load fonts */
    level_font = al_load_ttf_font(level_font_file, level_font_size, 0);
    if (!level_font) {
        n_log(LOG_ERR, "Failed to load level font");
        game_context_free(&ctx);
        return FALSE;
    }

    gui_font = al_load_ttf_font(gui_font_file, gui_font_size, 0);
    if (!gui_font) {
        gui_font = al_create_builtin_font();
    }

    /* Initialize intro snow */
    for (int i = 0; i < INTRO_SNOW_COUNT; ++i) {
        GameEntity* snow = pool_alloc(&ctx.intro_snow);
        if (snow) {
            entity_init_snowflake(snow,
                                  frandf(0.0f, (float)ctx.dw),
                                  frandf(-(float)ctx.dh, 0.0f),
                                  frandf(30.0f, 80.0f),
                                  frandf(2.0f, 6.0f));
        }
    }

    /* Load audio samples */
    if (audio_ok) {
        if (!(sfx_shoot = al_load_sample(shoot_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", shoot_sample, strerror(al_get_errno()));
        }
        if (!(sfx_jump = al_load_sample(jump_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", jump_sample, strerror(al_get_errno()));
        }
        if (!(sfx_hit_level = al_load_sample(hit_level_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", hit_level_sample, strerror(al_get_errno()));
        }
        if (!(sfx_hit_bonus = al_load_sample(hit_bonus_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", hit_bonus_sample, strerror(al_get_errno()));
        }
        if (!(sfx_falling = al_load_sample(falling_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", falling_sample, strerror(al_get_errno()));
        }
        if (!(sfx_game_over = al_load_sample(game_over_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", game_over_sample, strerror(al_get_errno()));
        }
        if (!(music_intro = al_load_sample(intro_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", intro_sample, strerror(al_get_errno()));
        }
        if (!(music_win = al_load_sample(win_sample))) {
            n_log(LOG_ERR, "could not load %s, %s", win_sample, strerror(al_get_errno()));
        }
    } else {
        n_log(LOG_ERR, "not loading musics and samples as audio is not correctly initialized");
    }

    ALLEGRO_SAMPLE_INSTANCE* music_intro_instance = NULL;
    ALLEGRO_SAMPLE_INSTANCE* music_win_instance = NULL;
    ALLEGRO_SAMPLE* current_sample = NULL;
    ALLEGRO_SAMPLE_INSTANCE* current_sample_instance = NULL;

    al_start_timer(fps_timer);
    al_start_timer(logic_timer);

    /*  INTRO SCREEN  */
    if (intro_count > 0 && audio_ok && music_intro) {
        music_intro_instance = al_create_sample_instance(music_intro);
        if (music_intro_instance) {
            al_set_sample_instance_playmode(music_intro_instance, ALLEGRO_PLAYMODE_LOOP);
            al_attach_sample_instance_to_mixer(music_intro_instance, al_get_default_mixer());
            al_play_sample_instance(music_intro_instance);
        }
    }

    bool in_intro = (intro_count > 0);
    while (in_intro) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (al_get_timer_event_source(fps_timer) == ev.any.source) {
                do_draw = 1;
            } else if (al_get_timer_event_source(logic_timer) == ev.any.source) {
                do_logic = 1;
            }
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                in_intro = false;
                ctx.party_result = PARTY_FAILED;
                goto cleanup;
            } else if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                in_intro = false;
            }
        } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            in_intro = false;
            ctx.party_result = PARTY_FAILED;
            goto cleanup;
        } else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            al_acknowledge_resize(display);
            ctx.dw = al_get_display_width(display);
            ctx.dh = al_get_display_height(display);
            ctx.center_x = ctx.dw / 2;
            ctx.center_y = ctx.dh / 2;
#ifndef __EMSCRIPTEN__
            if (ctx.mouse_locked) {
                al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
            }
#endif
        }

        if (do_logic) {
            const float dt = 1.0f / logic;
            light_phase += dt;

            /* Update intro snow */
            for (int i = 0; i < ctx.intro_snow.capacity; ++i) {
                entity_update_snowflake(&ctx.intro_snow.entities[i], dt, (float)ctx.dh);
            }
            do_logic = 0;
        }

        if (do_draw) {
            al_set_render_state(ALLEGRO_DEPTH_TEST, 0);
            al_clear_to_color(al_map_rgb(0, 0, 0));

            render_intro_snow(&ctx);

            /* Draw intro text */
            if (intro_lines) {
                int line_h = gui_font_size + 4;
                int total_h = line_h * intro_count;
                int y0 = (ctx.dh - 120 - total_h) / 2;
                if (y0 < 0) y0 = 0;

                for (int i = 0; i < intro_count; ++i) {
                    al_draw_text(gui_font, al_map_rgb(255, 255, 255),
                                 10, y0 + i * line_h,
                                 ALLEGRO_ALIGN_LEFT, intro_lines[i]);
                }
            }

            al_draw_text(gui_font, al_map_rgb(255, 255, 0),
                         ctx.dw / 2, ctx.dh - 120,
                         ALLEGRO_ALIGN_CENTRE, "Press ENTER to start");
            al_draw_text(gui_font, al_map_rgb(150, 150, 150),
                         ctx.dw / 2, ctx.dh - 60,
                         ALLEGRO_ALIGN_CENTRE, "ESC to quit");

            al_flip_display();
            do_draw = 0;
        }
    }

    if (music_intro_instance) {
        al_stop_sample_instance(music_intro_instance);
        al_destroy_sample_instance(music_intro_instance);
        music_intro_instance = NULL;
    }

    /*  MAIN GAME LOOP  */
    for (ctx.level_index = 0; ctx.level_index < level_count; ++ctx.level_index) {
        bool score_counted = false;

        /* Parse level config */
        char** level_split = split(levels[ctx.level_index], " ", 0);
        if (!level_split || split_count(level_split) < 4) {
            n_log(LOG_ERR, "error splitting level line");
            ctx.party_result = PARTY_FAILED;
            break;
        }

        char* phrase = strdup(level_split[0]);
        int phrase_len = (int)strlen(phrase);

        int tmpval;
        if (str_to_int(level_split[1], &tmpval, 10) == TRUE && (tmpval == 0 || tmpval == 1))
            COLOR_CYCLE_GOAL = tmpval;
        if (str_to_int(level_split[2], &tmpval, 10) == TRUE && (tmpval == 0 || tmpval == 1))
            PULSE_TEXT = tmpval;
        if (str_to_int(level_split[3], &tmpval, 10) == TRUE && (tmpval == 0 || tmpval == 1))
            BLEND_TEXT = tmpval;

        free_split_result(&level_split);

        /* Reset level state */
        n_log(LOG_DEBUG, "Level %d: game_context_reset_level...", ctx.level_index + 1);
        game_context_reset_level(&ctx);

        /* Build level */
        n_log(LOG_DEBUG, "Level %d: build_level_geometry for: %s", ctx.level_index + 1, phrase);
        if (!build_level_geometry(&ctx, level_font, gui_font, phrase, phrase_len, level_font_size)) {
            goto cleanup;
        }

        /* Generate starfield */
        n_log(LOG_DEBUG, "Level %d: generate_starfield...", ctx.level_index + 1);
        float level_w = ctx.vf.gw * ctx.vf.cell_size;
        float level_d = ctx.vf.gh * ctx.vf.cell_size;
        float level_radius = 0.5f * sqrtf(level_w * level_w + level_d * level_d);
        float min_r = level_radius + 50.0f;
        float max_r = level_radius + 250.0f;

        int star_count = 128 + 120 * phrase_len;
        if (star_count > STAR_COUNT) star_count = STAR_COUNT;
        generate_starfield(&ctx.stars, star_count, min_r, max_r);

        /* Place boxes and lights */
        n_log(LOG_DEBUG, "Level %d: place_boxes_and_lights...", ctx.level_index + 1);
        place_boxes_and_lights(&ctx);

        /* Setup camera */
        n_log(LOG_DEBUG, "Level %d: setup_camera_start...", ctx.level_index + 1);
        setup_camera_start(&ctx);

#ifndef __EMSCRIPTEN__
        al_hide_mouse_cursor(display);
        al_grab_mouse(display);
        if (ctx.mouse_locked) {
            al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
        }
#endif

        /* Start level music */
        if (audio_ok && songs && ctx.level_index < songs_count) {
            current_sample = al_load_sample(songs[ctx.level_index]);
            if (current_sample) {
                current_sample_instance = al_create_sample_instance(current_sample);
                if (current_sample_instance) {
                    al_set_sample_instance_playmode(current_sample_instance, ALLEGRO_PLAYMODE_LOOP);
                    al_attach_sample_instance_to_mixer(current_sample_instance, al_get_default_mixer());
                    al_play_sample_instance(current_sample_instance);
                }
            } else {
                n_log(LOG_ERR, "unable to load song %s", songs[ctx.level_index]);
            }
        }

        /* Level state variables */
        int level_boxes_hit = 0;
        int level_time_bonus_boxes = 0;
        int level_speed_bonus_boxes = 0;
        bool time_over = false;
        bool fell_out = false;
        bool game_over_played = false;
        bool winning_music_started = false;
        bool was_above_top = true;
        bool save_jump_available = false;

        const float TOP_Y = ctx.vf.extrude_h;
        const float FALL_DEATH_Y = -10.0f;
        const float SAVE_JUMP_MIN_Y = -5.0f;

        int keys[ALLEGRO_KEY_MAX] = {0};
        bool leaving_level = false;
        bool restart_level = false;

        n_log(LOG_DEBUG, "Starting level %d: %s", ctx.level_index + 1, phrase);
        Free(phrase);

        /* flush queue to eliminate all unecessary events accumulated during loading phases */
        ALLEGRO_EVENT flush_ev;
        while (al_get_next_event(queue, &flush_ev)) {
            if (flush_ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
                al_acknowledge_resize(display);
                ctx.dw = al_get_display_width(display);
                ctx.dh = al_get_display_height(display);
                ctx.center_x = ctx.dw / 2;
                ctx.center_y = ctx.dh / 2;
#ifndef __EMSCRIPTEN__
                if (ctx.mouse_locked) {
                    al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
                }
#endif
            }
        }
        al_flush_event_queue(queue);

        /* Level event loop */
        while (!leaving_level) {
            ALLEGRO_EVENT ev;
            al_wait_for_event(queue, &ev);
            if (ev.type == ALLEGRO_EVENT_TIMER) {
                if (al_get_timer_event_source(fps_timer) == ev.any.source) {
                    do_draw = 1;
                } else if (al_get_timer_event_source(logic_timer) == ev.any.source) {
                    do_logic = 1;
                }
            } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                int kc = ev.keyboard.keycode;

                if (kc == ALLEGRO_KEY_ESCAPE) {
                    ctx.state = STATE_PARTY_END;
                    ctx.party_result = PARTY_FAILED;
                    leaving_level = true;
                    break;
                } else if (kc == ALLEGRO_KEY_F1) {
                    ctx.paused = !ctx.paused;

                    if (ctx.paused) {
                        ctx.mouse_locked = false;

#ifndef __EMSCRIPTEN__
                        al_ungrab_mouse();
                        al_show_mouse_cursor(display);
#else
                        /* Web: release browser pointer lock so the user can interact with the page. */
                        web_exit_pointer_lock();
                        al_show_mouse_cursor(display);
#endif
                    } else {
                        ctx.mouse_locked = true;

#ifndef __EMSCRIPTEN__
                        al_grab_mouse(display);
                        al_hide_mouse_cursor(display);
                        al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
#else
                        /*
Web: requesting pointer lock here usually works because this is executed
in direct response to a key event (user gesture).
If the browser refuses, the click callback will acquire it on next click.
*/
                        web_request_pointer_lock();
                        al_hide_mouse_cursor(display);
#endif
                    }
                } else if (kc == ALLEGRO_KEY_F3) {
                    ctx.gravity_enabled = !ctx.gravity_enabled;
                    if (ctx.gravity_enabled) {
                        ctx.vertical_vel = 0.0f;
                        ctx.on_ground = false;
                        ctx.cheat_code_used = true;
                    }
                    n_log(LOG_DEBUG, "CHEATCODE gravity_enabled = %d", ctx.gravity_enabled);
                } else if (kc == ALLEGRO_KEY_1) {
                    /* Toggle goal color cycling */
                    COLOR_CYCLE_GOAL = !COLOR_CYCLE_GOAL;
                    n_log(LOG_DEBUG, "CHEATCODE COLOR_CYCLE_GOAL = %d", COLOR_CYCLE_GOAL);
                } else if (kc == ALLEGRO_KEY_2) {
                    /* Toggle text pulsing */
                    PULSE_TEXT = !PULSE_TEXT;
                    n_log(LOG_DEBUG, "CHEATCODE PULSE_TEXT = %d", PULSE_TEXT);
                } else if (kc == ALLEGRO_KEY_3) {
                    /* Toggle blend mode */
                    BLEND_TEXT = !BLEND_TEXT;
                    n_log(LOG_DEBUG, "CHEATCODE BLEND_TEXT = %d", BLEND_TEXT);
                } else if (kc == ALLEGRO_KEY_T) {
                    /* Time bonus cheat */
                    n_log(LOG_DEBUG, "CHEATCODE TIME +30s !!");
                    ctx.time_remaining += 30.0f;
                    ctx.cheat_code_used = true;
                } else if (kc == ALLEGRO_KEY_V) {
                    /* Speed bonus cheat */
                    ctx.move_speed += speed_bonus_increment;
                    if (ctx.move_speed > speed_max_limit)
                        ctx.move_speed = speed_max_limit;
                    /* save max achieved move speed */
                    if (ctx.move_speed > ctx.max_speed)
                        ctx.max_speed = ctx.move_speed;
                    n_log(LOG_DEBUG, "CHEATCODE SPEED %f, total: %f !!", speed_bonus_increment, ctx.move_speed);
                    ctx.cheat_code_used = true;
                } else if (kc == ALLEGRO_KEY_SPACE) {
                    if (!ctx.paused && ctx.state == STATE_PLAY) {
                        if (ctx.gravity_enabled) {
                            if (ctx.on_ground) {
                                ctx.vertical_vel = jump_vel;
                                ctx.on_ground = false;
                                if (audio_ok && sfx_jump) {
                                    al_play_sample(sfx_jump, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
                                }
                            } else if (save_jump_available) {
                                float bottom = ctx.cam.position.y - ctx.cam_half_height;
                                if (bottom > SAVE_JUMP_MIN_Y) {
                                    ctx.vertical_vel = jump_vel;
                                }
                                save_jump_available = false;
                                if (audio_ok && sfx_jump) {
                                    al_play_sample(sfx_jump, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
                                }
                            }
                        }
                        keys[ALLEGRO_KEY_SPACE] = 1;
                    }
                } else if (kc == ALLEGRO_KEY_ENTER) {
                    if (ctx.state == STATE_LEVEL_END) {
                        leaving_level = true;
                    } else if (ctx.state == STATE_PARTY_END) {
                        if (ctx.party_result == PARTY_FAILED) {
                            restart_level = true;
                        }
                        leaving_level = true;
                    }
                } else if (kc == ALLEGRO_KEY_F11) {
#ifndef __EMSCRIPTEN__
                    uint32_t flags = al_get_display_flags(display);
                    bool is_fullscreen = (flags & ALLEGRO_FULLSCREEN_WINDOW) != 0;
                    al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, !is_fullscreen);
                    al_acknowledge_resize(display);
                    ctx.dw = al_get_display_width(display);
                    ctx.dh = al_get_display_height(display);
                    ctx.center_x = ctx.dw / 2;
                    ctx.center_y = ctx.dh / 2;
                    if (ctx.mouse_locked) {
                        al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
                    }
#endif
                } else if (kc < ALLEGRO_KEY_MAX) {
                    keys[kc] = 1;
                }
            } else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
                if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) {
                    keys[ev.keyboard.keycode] = 0;
                }
            } else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                if (ctx.paused) {
                    ctx.paused = false;
                    ctx.mouse_locked = true;

#ifndef __EMSCRIPTEN__
                    al_grab_mouse(display);
                    al_hide_mouse_cursor(display);
                    if (ctx.mouse_locked) {
                        al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
                    }
#else
                    web_request_pointer_lock(); /* this is a user gesture, should succeed */
                    al_hide_mouse_cursor(display);
#endif
                } else if (ev.mouse.button == 1 && ctx.state == STATE_PLAY) {
                    fire_projectile(&ctx, sfx_shoot, audio_ok, bullet_speed);
                }
            } else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
#ifndef __EMSCRIPTEN__
                if (ctx.mouse_locked && ctx.state == STATE_PLAY && !ctx.paused) {
                    pending_mdx += (float)ev.mouse.dx;
                    pending_mdy += (float)ev.mouse.dy;
                }
#else
                /* Web: deltas come from Emscripten mousemove callback (movementX/Y) under pointer lock */
                (void)ev;
#endif
            } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                ctx.state = STATE_PARTY_END;
                ctx.party_result = PARTY_FAILED;
                leaving_level = true;
                break;
            } else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
                al_acknowledge_resize(display);
                ctx.dw = al_get_display_width(display);
                ctx.dh = al_get_display_height(display);
                ctx.center_x = ctx.dw / 2;
                ctx.center_y = ctx.dh / 2;
#ifndef __EMSCRIPTEN__
                if (ctx.mouse_locked) {
                    al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
                }
#endif
            }

            if (do_logic) {
                const float dt = 1.0f / logic;

                /* Update timer */
                if (ctx.state == STATE_PLAY && !ctx.paused) {
                    ctx.time_remaining -= dt;
                    if (ctx.time_remaining <= 0.0f && !time_over) {
                        ctx.time_remaining = 0.0f;
                        ctx.state = STATE_PARTY_END;
                        time_over = true;
                        ctx.party_result = PARTY_FAILED;

                        if (!game_over_played && audio_ok && sfx_game_over) {
                            if (current_sample_instance) {
                                al_stop_sample_instance(current_sample_instance);
                                al_destroy_sample_instance(current_sample_instance);
                                current_sample_instance = NULL;
                            }
                            if (current_sample) {
                                al_destroy_sample(current_sample);
                                current_sample = NULL;
                            }
                            al_play_sample(sfx_game_over, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
                            game_over_played = true;
                        }
                    }
                }

                /* Mouse look */
#ifndef __EMSCRIPTEN__
                if (ctx.mouse_locked && ctx.state == STATE_PLAY && !ctx.paused)
#else
                if (mouse_capture_active(&ctx))
#endif
                {
                    float dx = pending_mdx;
                    float dy = pending_mdy;
                    pending_mdx = pending_mdy = 0.0f;

                    if (dx != 0.0f || dy != 0.0f) {
                        ctx.cam.yaw -= dx * mouse_sensitivity;
                        ctx.cam.pitch -= dy * mouse_sensitivity;

                        float limit = (float)(M_PI / 2.0f - 0.1f);
                        ctx.cam.pitch = clampf(ctx.cam.pitch, -limit, limit);

#ifndef __EMSCRIPTEN__
                        al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
#endif
                    }
                }

                /* Movement */
                if (ctx.state == STATE_PLAY && !ctx.paused) {
                    Vec3 forward3 = camera_forward(&ctx.cam);
                    Vec3 right3 = camera_right(&ctx.cam);

                    ctx.move_forward = ctx.move_lateral = 0.0f;
                    if (keys[ALLEGRO_KEY_W] || keys[ALLEGRO_KEY_UP] || keys[ALLEGRO_KEY_Z])
                        ctx.move_forward += ctx.move_speed;
                    if (keys[ALLEGRO_KEY_S] || keys[ALLEGRO_KEY_DOWN])
                        ctx.move_forward -= ctx.move_speed;
                    if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT])
                        ctx.move_lateral += ctx.move_speed;
                    if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT] || keys[ALLEGRO_KEY_Q])
                        ctx.move_lateral -= ctx.move_speed;

                    bool prev_on_ground = ctx.on_ground;
                    Vec3 disp = v_zero();

                    if (ctx.gravity_enabled) {
                        Vec3 forward_flat = v_normalize(v_make(forward3.x, 0.0f, forward3.z));
                        Vec3 right_flat = v_normalize(v_make(right3.x, 0.0f, right3.z));

                        disp = v_add(disp, v_scale(forward_flat, ctx.move_forward));
                        disp = v_add(disp, v_scale(right_flat, ctx.move_lateral));

                        ctx.vertical_vel += gravity * dt;
                        disp.y += ctx.vertical_vel * dt;
                    } else {
                        disp = v_add(disp, v_scale(forward3, ctx.move_forward));
                        disp = v_add(disp, v_scale(right3, ctx.move_lateral));

                        if (keys[ALLEGRO_KEY_SPACE])
                            disp.y += ctx.move_speed;
                    }

                    /* Update obstacles */
                    Vec3 hit_move = v_make(0.0f, 0.0f, 0.0f);
                    if (ctx.state == STATE_PLAY && !ctx.paused) {
                        /* Spawn Obstacles */
                        obstacle_spawn_timer += dt;
                        if (obstacle_spawn_timer >= obstacle_spawn_delay) {
                            obstacle_spawn_timer = 0.0f;

                            /* Spawn at the end of the level geometry */
                            float end_x = ctx.vf.origin_x + ctx.vf.gw * ctx.vf.cell_size;
                            float z_span = ctx.vf.gh * ctx.vf.cell_size;

                            /* Random Z position within level width */
                            float spawn_z = ctx.vf.origin_z + frandf(0.0f, z_span);

                            /* Random Size */
                            float size = frandf(2.5f, 6.0f);

                            GameEntity* obs = pool_alloc(&ctx.boxes);
                            if (obs) {
                                /* speed (negative X) */
                                Vec3 vel = v_make(-frandf(20.0f, 60.0f), 0.0f, 0.0f);
                                /* Y over the surface of the letters */
                                Vec3 pos = v_make(end_x, ctx.vf.extrude_h + size, spawn_z);
                                entity_init_obstacle(obs, pos, vel, size);
                            }
                        }

                        /* Update boxes and boxes collisions */
                        for (int i = 0; i < ctx.boxes.capacity; ++i) {
                            GameEntity* box = &ctx.boxes.entities[i];
                            if (!entity_is_active(box) || !(box->flags & ENTITY_FLAG_OBSTACLE)) continue;

                            /* Box move */
                            box->pos = v_add(box->pos, v_scale(box->vel, dt));

                            /* collision */
                            if (capsule_aabb_collides(ctx.cam.position, ctx.cam_radius, ctx.cam_half_height, box->pos, box->size)) {
                                /* add box move to player */
                                hit_move.x += box->vel.x * dt;
                                /* Feedback effects (optional) */
                                ctx.cam.pitch += frandf(-0.02f, 0.02f);
                                ctx.cam.yaw += frandf(-0.02f, 0.02f);
                            }

                            /* if bump box is out of the map, kill it */
                            if (box->pos.x < ctx.vf.origin_x - 30.0f) {
                                entity_deactivate(box);
                            }
                        }
                    }

                    if (v_norm(disp) > 1e-5f) {
                        Vec3 pos = ctx.cam.position;

                        /* X axis */
                        Vec3 test_pos = pos;
                        test_pos.x += disp.x + hit_move.x;
                        if (!capsule_collides(&ctx.vf, test_pos, ctx.cam_radius, ctx.cam_half_height)) {
                            pos.x = test_pos.x;
                        }

                        /* Z axis */
                        test_pos = pos;
                        test_pos.z += disp.z + hit_move.z;
                        if (!capsule_collides(&ctx.vf, test_pos, ctx.cam_radius, ctx.cam_half_height)) {
                            pos.z = test_pos.z;
                        }

                        /* Y axis */
                        test_pos = pos;
                        test_pos.y += disp.y + hit_move.y;
                        if (!capsule_collides(&ctx.vf, test_pos, ctx.cam_radius, ctx.cam_half_height)) {
                            pos.y = test_pos.y;
                            if (ctx.gravity_enabled)
                                ctx.on_ground = false;
                        } else {
                            if (ctx.gravity_enabled) {
                                if (disp.y < 0.0f)
                                    ctx.on_ground = true;
                                ctx.vertical_vel = 0.0f;
                            }
                        }

                        ctx.cam.position = pos;

                        /* Fall detection */
                        if (ctx.gravity_enabled) {
                            float bottom = ctx.cam.position.y - ctx.cam_half_height;

                            if (prev_on_ground && !ctx.on_ground) {
                                int gx, gy;
                                world_to_grid(&ctx.vf, ctx.cam.position.x, ctx.cam.position.z, &gx, &gy);
                                bool near_solid = false;
                                for (int dy = -1; dy <= 1 && !near_solid; ++dy) {
                                    for (int dx = -1; dx <= 1; ++dx) {
                                        if (is_solid(&ctx.vf, gx + dx, gy + dy)) {
                                            near_solid = true;
                                            break;
                                        }
                                    }
                                }
                                save_jump_available = (near_solid && bottom > SAVE_JUMP_MIN_Y);
                            }

                            if (ctx.on_ground)
                                save_jump_available = false;

                            if (bottom > TOP_Y + 0.1f) {
                                was_above_top = true;
                            } else if (was_above_top && bottom < TOP_Y && ctx.vertical_vel < 0.0f) {
                                was_above_top = false;
                                if (audio_ok && sfx_falling) {
                                    al_play_sample(sfx_falling, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
                                }
                            }

                            if (bottom < FALL_DEATH_Y && !fell_out) {
                                save_jump_available = false;
                                ctx.state = STATE_PARTY_END;
                                ctx.party_result = PARTY_FAILED;
                                fell_out = true;

                                if (!game_over_played && audio_ok && sfx_game_over) {
                                    if (current_sample_instance) {
                                        al_stop_sample_instance(current_sample_instance);
                                        al_destroy_sample_instance(current_sample_instance);
                                        current_sample_instance = NULL;
                                    }
                                    if (current_sample) {
                                        al_destroy_sample(current_sample);
                                        current_sample = NULL;
                                    }
                                    al_play_sample(sfx_game_over, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
                                    game_over_played = true;
                                }
                            }
                        }

                        /* Goal check */
                        if (ctx.gravity_enabled && ctx.on_ground && ctx.state == STATE_PLAY) {
                            int gx, gy;
                            world_to_grid(&ctx.vf, ctx.cam.position.x, ctx.cam.position.z, &gx, &gy);
                            if (gx >= 0 && gx < ctx.vf.gw && gy >= 0 && gy < ctx.vf.gh) {
                                int idx = gy * ctx.vf.gw + gx;
                                if (ctx.vf.is_goal[idx]) {
                                    ctx.state = STATE_PARTY_END;
                                    if (!time_over && !fell_out) {
                                        ctx.party_result = PARTY_SUCCESS;
                                    }

                                    if (!winning_music_started && audio_ok && music_win) {
                                        if (current_sample_instance) {
                                            al_stop_sample_instance(current_sample_instance);
                                            al_destroy_sample_instance(current_sample_instance);
                                            current_sample_instance = NULL;
                                        }
                                        if (current_sample) {
                                            al_destroy_sample(current_sample);
                                            current_sample = NULL;
                                        }
                                        music_win_instance = al_create_sample_instance(music_win);
                                        if (music_win_instance) {
                                            al_set_sample_instance_playmode(music_win_instance, ALLEGRO_PLAYMODE_ONCE);
                                            al_attach_sample_instance_to_mixer(music_win_instance, al_get_default_mixer());
                                            al_play_sample_instance(music_win_instance);
                                        }
                                        winning_music_started = true;
                                    }

                                    if (ctx.level_index != level_count - 1) {
                                        ctx.state = STATE_LEVEL_END;
                                    }

                                    if (!score_counted) {
                                        ctx.total_score += ctx.score;
                                        score_counted = true;
                                    }
                                }
                            }
                        }
                    }
                }

                /* Update projectiles */
                update_projectiles(&ctx, dt, sfx_hit_level, sfx_hit_bonus, audio_ok,
                                   &level_boxes_hit, &level_time_bonus_boxes, &level_speed_bonus_boxes,
                                   speed_bonus_increment, speed_max_limit);

                /* Update moving pink lights */
                update_pink_lights(&ctx, dt);

                /* Celebration particles */
                if (ctx.state == STATE_PARTY_END && ctx.party_result == PARTY_SUCCESS) {
                    spawn_celebration_particles(&ctx);
                }

                /* Update particles */
                if ((ctx.state == STATE_PLAY && !ctx.paused) ||
                    ctx.state == STATE_LEVEL_END ||
                    (ctx.state == STATE_PARTY_END && ctx.party_result == PARTY_SUCCESS)) {
                    update_particles(&ctx, gravity, dt);
                }

                do_logic = 0;
            }

            if (do_draw) {
                const float dt = 1.0f / fps;
                light_phase += dt * 0.75f;

                /*  RENDERING  */

                bool overlay_letters = (PULSE_TEXT != 0);
                bool overlay_goals = (COLOR_CYCLE_GOAL != 0);

#ifdef __EMSCRIPTEN__
                /* triggered by emscripten fullscreen callbacks */
                if (ctx.pending_resize) {
                    /* keep track of fullscreen status */
                    if (!fullscreen)
                        fullscreen = true;
                    else
                        fullscreen = false;

                    ctx.pending_resize = false;

                    /* reset size on bad values or when leaving fullscreen */
                    if (ctx.pending_w <= 0 || ctx.pending_h <= 0 || !fullscreen) {
                        ctx.pending_w = WIDTH;
                        ctx.pending_h = HEIGHT;
                    }

                    al_resize_display(ctx.display, ctx.pending_w, ctx.pending_h);

                    /* For GL/WebGL backends, make sure you are drawing to the display again */
                    al_set_target_backbuffer(ctx.display);

                    ctx.dw = al_get_display_width(ctx.display);
                    ctx.dh = al_get_display_height(ctx.display);
                    ctx.center_x = ctx.dw / 2;
                    ctx.center_y = ctx.dh / 2;
                }
#endif
                al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
                al_set_render_state(ALLEGRO_WRITE_MASK, ALLEGRO_MASK_DEPTH | ALLEGRO_MASK_RGBA);

                setup_3d_projection(ctx.cam.vertical_fov, Z_NEAR, Z_FAR);
                al_clear_depth_buffer(1.0f);
                al_clear_to_color(al_map_rgb(5, 5, 15));

                Vec3 forward3 = camera_forward(&ctx.cam);
                Vec3 target = v_add(ctx.cam.position, forward3);

                ALLEGRO_TRANSFORM view;
                al_build_camera_transform(&view,
                                          ctx.cam.position.x, ctx.cam.position.y, ctx.cam.position.z,
                                          target.x, target.y, target.z,
                                          0.0f, 1.0f, 0.0f);
                al_use_transform(&view);

                /* Stars */
                render_starfield(&ctx.stars, &ctx.va_stars, light_phase);
                vbo_draw(&ctx.g_ttfe_stream_vbo, &ctx.va_stars, ALLEGRO_PRIM_TRIANGLE_LIST);

                /* Level geometry */
                vbo_draw(&ctx.g_ttfe_stream_vbo, &ctx.va_level, ALLEGRO_PRIM_TRIANGLE_LIST);

                /* Glow overlay */
                if (overlay_letters || overlay_goals) {
                    al_store_state(ctx.render_state, ALLEGRO_STATE_BLENDER);

                    int prev_depth_test = al_get_render_state(ALLEGRO_DEPTH_TEST);
                    al_set_render_state(ALLEGRO_DEPTH_TEST, 0);

                    if (overlay_letters && ctx.va_overlay_letters.count > 0) {
                        float s = sinf(light_phase * 4.0f) * 0.5f + 0.5f;
                        float pulse_alpha = 0.6f * s + 0.2f;
                        ALLEGRO_COLOR letter_glow = al_map_rgba_f(0.4f, 0.1f, 0.4f, pulse_alpha);
                        for (int i = 0; i < ctx.va_overlay_letters.count; ++i) {
                            ctx.va_overlay_letters.v[i].color = letter_glow;
                        }
                    }

                    if (overlay_goals && ctx.va_overlay_goals.count > 0) {
                        ALLEGRO_COLOR goal_glow = rainbow_color(light_phase * 2.0f, 1.0f);
                        for (int i = 0; i < ctx.va_overlay_goals.count; ++i) {
                            ctx.va_overlay_goals.v[i].color = goal_glow;
                        }
                    }

                    if (BLEND_TEXT) {
                        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
                    } else {
                        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
                    }

                    if (overlay_goals) vbo_draw(&ctx.g_ttfe_stream_vbo, &ctx.va_overlay_goals, ALLEGRO_PRIM_TRIANGLE_LIST);
                    if (overlay_letters) vbo_draw(&ctx.g_ttfe_stream_vbo, &ctx.va_overlay_letters, ALLEGRO_PRIM_TRIANGLE_LIST);

                    al_set_render_state(ALLEGRO_DEPTH_TEST, prev_depth_test);
                    al_restore_state(ctx.render_state);
                }

                /* Pink lights */
                Vec3 cam_right = camera_right(&ctx.cam);
                Vec3 cam_up = camera_up(&ctx.cam);

                if (pool_active_count(&ctx.pink_lights) > 0) {
                    render_pink_lights(&ctx.pink_lights, &ctx.va_pink_lights, cam_right, cam_up, light_phase);
                    al_store_state(ctx.render_state, ALLEGRO_STATE_BLENDER);
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
                    vbo_draw(&ctx.g_ttfe_stream_vbo, &ctx.va_pink_lights, ALLEGRO_PRIM_TRIANGLE_LIST);
                    al_restore_state(ctx.render_state);
                }

                /* Boxes */
                render_boxes(&ctx);

                /* Particles */
                render_particles(&ctx, cam_right, cam_up);

                /* Projectiles */
                render_projectiles(&ctx);

                /*  HUD  */
                al_set_render_state(ALLEGRO_DEPTH_TEST, 0);

                ALLEGRO_TRANSFORM proj2d, view2d;
                al_identity_transform(&proj2d);
                al_orthographic_transform(&proj2d, 0, 0, -1, ctx.dw, ctx.dh, 1);
                al_use_projection_transform(&proj2d);
                al_identity_transform(&view2d);
                al_use_transform(&view2d);

                char buf[256];
                int ti = (int)ctx.time_remaining;
                if (ti < 0) ti = 0;

                snprintf(buf, sizeof(buf), "Level %d/%d | Level score: %d | Time: %02d:%02d | Speed: %0.2f/%0.2f",
                         ctx.level_index + 1, level_count, ctx.score, ti / 60, ti % 60, ctx.move_speed, speed_max_limit);

                al_draw_text(gui_font, al_map_rgb(255, 255, 255), 10, 10, 0, buf);

                /* Crosshair */
                if (!ctx.paused) {
                    int cx = ctx.dw / 2;
                    int cy = ctx.dh / 2;
                    al_draw_line(cx - 10, cy, cx + 10, cy, al_map_rgb(255, 0, 0), 1.0f);
                    al_draw_line(cx, cy - 10, cx, cy + 10, al_map_rgb(255, 0, 0), 1.0f);
                }

                /* Pause text */
                if (ctx.paused) {
                    ALLEGRO_COLOR pause_col = rainbow_color(light_phase * 1.5f, 1.0f);
                    al_draw_text(gui_font, pause_col, ctx.dw / 2, ctx.dh / 2 - gui_font_size / 2,
                                 ALLEGRO_ALIGN_CENTRE, "PAUSE");
                }

                /* End state messages */
                if (ctx.state == STATE_LEVEL_END) {
                    snprintf(buf, sizeof(buf), "LEVEL COMPLETED! Level score: %d, Total score: %d",
                             ctx.score, ctx.total_score);
                    al_draw_text(gui_font, al_map_rgb(0, 255, 0), ctx.dw / 2, ctx.dh / 2 - 80,
                                 ALLEGRO_ALIGN_CENTRE, buf);
                    if (ctx.cheat_code_used)
                        al_draw_text(gui_font, al_map_rgb(255, 0, 0), ctx.dw / 2, ctx.dh / 2 - 20,
                                     ALLEGRO_ALIGN_CENTRE, "cheat code were used !-)");

                    al_draw_text(gui_font, al_map_rgb(255, 255, 255), ctx.dw / 2, ctx.dh / 2 + 40,
                                 ALLEGRO_ALIGN_CENTRE, "Press ENTER for next level or ESC to quit");
                } else if (ctx.state == STATE_PARTY_END) {
                    if (ctx.party_result == PARTY_SUCCESS && !time_over && !fell_out) {
                        snprintf(buf, sizeof(buf), "YOU WIN! Final score: %d", ctx.total_score);
                        al_draw_text(gui_font, al_map_rgb(0, 255, 0), ctx.dw / 2, ctx.dh / 2 - 80,
                                     ALLEGRO_ALIGN_CENTRE, buf);
                        if (ctx.cheat_code_used)
                            al_draw_text(gui_font, al_map_rgb(255, 0, 0), ctx.dw / 2, ctx.dh / 2 - 20,
                                         ALLEGRO_ALIGN_CENTRE, "(but you used a cheat code...)");

                        al_draw_text(gui_font, al_map_rgb(255, 255, 255), ctx.dw / 2, ctx.dh / 2 + 40,
                                     ALLEGRO_ALIGN_CENTRE, "Press ENTER to go to score or ESC to quit");
                    } else {
                        if (time_over) {
                            snprintf(buf, sizeof(buf), "YOU LOSE! Time over!");
                        } else if (fell_out) {
                            snprintf(buf, sizeof(buf), "YOU LOSE! You fell into the void!");
                        } else {
                            snprintf(buf, sizeof(buf), "YOU LOSE!");
                        }
                        al_draw_text(gui_font, al_map_rgb(255, 0, 0), ctx.dw / 2, ctx.dh / 2 - 40,
                                     ALLEGRO_ALIGN_CENTRE, buf);
                        al_draw_text(gui_font, al_map_rgb(255, 255, 255), ctx.dw / 2, ctx.dh / 2 + 20,
                                     ALLEGRO_ALIGN_CENTRE, "Press ENTER to restart or ESC to quit");
                    }
                }

                al_flip_display();
                do_draw = 0;
            }
        }

        /* Stop level music */
        if (current_sample_instance) {
            al_stop_sample_instance(current_sample_instance);
            al_destroy_sample_instance(current_sample_instance);
            current_sample_instance = NULL;
        }
        if (current_sample) {
            al_destroy_sample(current_sample);
            current_sample = NULL;
        }

        /* Free level resources */
        if (ctx.vf.solid) {
            free(ctx.vf.solid);
            ctx.vf.solid = NULL;
        }
        if (ctx.vf.is_goal) {
            free(ctx.vf.is_goal);
            ctx.vf.is_goal = NULL;
        }

        /* Restart level if requested */
        if (ctx.state == STATE_PARTY_END && ctx.party_result == PARTY_FAILED && restart_level) {
            ctx.party_result = PARTY_UNDECIDED;
            ctx.state = STATE_PLAY;
            ctx.level_index--;
            ctx.move_speed = base_speed;
            continue;
        }

        if (ctx.state == STATE_PARTY_END) {
            break;
        }
    }

    /*  OUTRO SCREEN  */
    if (ctx.party_result == PARTY_SUCCESS) {
        pool_clear(&ctx.particles);
        ctx.total_score += ctx.score;

        bool in_outro = true;
        while (in_outro) {
            ALLEGRO_EVENT ev;
            al_wait_for_event(queue, &ev);

            if (ev.type == ALLEGRO_EVENT_TIMER) {
                if (al_get_timer_event_source(fps_timer) == ev.any.source) {
                    do_draw = 1;
                } else if (al_get_timer_event_source(logic_timer) == ev.any.source) {
                    do_logic = 1;
                }
            } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE ||
                       (ev.type == ALLEGRO_EVENT_KEY_DOWN &&
                        (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE ||
                         ev.keyboard.keycode == ALLEGRO_KEY_ENTER ||
                         ev.keyboard.keycode == ALLEGRO_KEY_SPACE))) {
                in_outro = false;
            } else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
                al_acknowledge_resize(display);
                ctx.dw = al_get_display_width(display);
                ctx.dh = al_get_display_height(display);
                ctx.center_x = ctx.dw / 2;
                ctx.center_y = ctx.dh / 2;
#ifndef __EMSCRIPTEN__
                if (ctx.mouse_locked) {
                    al_set_mouse_xy(display, ctx.center_x, ctx.center_y);
                }
#endif
            }

            if (do_logic) {
                const float dt = 1.0f / 60.0f;

                /* Spawn confetti */
                int bursts = rand() % 3;
                for (int bi = 0; bi < bursts; ++bi) {
                    Vec3 center = v_make(frandf(0.0f, (float)ctx.dw), frandf(-20.0f, 0.0f), 0.0f);
                    int count = 100 + rand() % 100;

                    for (int pi = 0; pi < count; ++pi) {
                        GameEntity* p = pool_alloc(&ctx.particles);
                        if (!p) break;

                        Vec3 vel = v_make(frandf(-30.0f, 30.0f), frandf(50.0f, 120.0f), 0.0f);

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

                        entity_init_particle(p,
                                             v_make(center.x + frandf(-25.0f, 25.0f),
                                                    center.y + frandf(-10.0f, 10.0f), 0.0f),
                                             vel, frandf(1.0f, 4.0f), 3.0f, color);
                    }
                }

                /* Update confetti (2D) */
                for (int i = 0; i < ctx.particles.capacity; ++i) {
                    GameEntity* p = &ctx.particles.entities[i];
                    if (!entity_is_active(p)) continue;

                    p->pos.x += p->vel.x * dt;
                    p->pos.y += p->vel.y * dt;
                    p->lifetime -= dt;

                    if (p->lifetime <= 0.0f || p->pos.y > (float)ctx.dh + 50.0f) {
                        entity_deactivate(p);
                    }
                }

                if (ctx.mouse_locked) {
                    ctx.mouse_locked = false;
                    al_ungrab_mouse();
                    al_show_mouse_cursor(display);
                }
                do_logic = 0;
            }

            if (do_draw) {
                al_set_render_state(ALLEGRO_DEPTH_TEST, 0);
                al_clear_to_color(al_map_rgb(0, 0, 0));

                /* Draw confetti as points */
                ALLEGRO_VERTEX pv[MAX_PARTICLES];
                int pc = 0;
                for (int i = 0; i < ctx.particles.capacity; ++i) {
                    GameEntity* p = &ctx.particles.entities[i];
                    if (!entity_is_active(p)) continue;
                    pv[pc++] = (ALLEGRO_VERTEX){p->pos.x, p->pos.y, 0.0f, 0.0f, 0.0f, p->color};
                }
                if (pc > 0) {
                    al_draw_prim(pv, NULL, NULL, 0, pc, ALLEGRO_PRIM_POINT_LIST);
                }

                char buf[256];
                snprintf(buf, sizeof(buf), "CONGRATULATIONS! Final score: %d", ctx.total_score);
                al_draw_text(gui_font, al_map_rgb(0, 255, 0), ctx.dw / 2, ctx.dh / 2 - 80,
                             ALLEGRO_ALIGN_CENTRE, buf);

                if (ctx.cheat_code_used)
                    al_draw_text(gui_font, al_map_rgb(255, 0, 0), ctx.dw / 2, ctx.dh / 2 - 20,
                                 ALLEGRO_ALIGN_CENTRE, "Next time try without the cheat code !-)");
                snprintf(buf, sizeof(buf), "Max speed reached: %.2f", ctx.max_speed);
                al_draw_text(gui_font, al_map_rgb(200, 200, 255), ctx.dw / 2, ctx.dh / 2 + 40,
                             ALLEGRO_ALIGN_CENTRE, buf);

                al_draw_text(gui_font, al_map_rgb(255, 255, 255), ctx.dw / 2, ctx.dh / 2 + 100,
                             ALLEGRO_ALIGN_CENTRE, "Press ENTER or ESC to quit");

                al_flip_display();
                do_draw = 0;
            }
        }
    }

cleanup:
    /* Cleanup */
    for (int i = 0; i < level_count; ++i) free(levels[i]);
    free(levels);

    if (songs) {
        for (int i = 0; i < songs_count; ++i) free(songs[i]);
        free(songs);
    }

    if (intro_lines) {
        for (int i = 0; i < intro_count; ++i) free(intro_lines[i]);
        free(intro_lines);
    }

    if (audio_ok) {
        if (music_intro_instance) al_destroy_sample_instance(music_intro_instance);
        if (music_win_instance) al_destroy_sample_instance(music_win_instance);
        if (music_intro) al_destroy_sample(music_intro);
        if (music_win) al_destroy_sample(music_win);
        if (sfx_shoot) al_destroy_sample(sfx_shoot);
        if (sfx_jump) al_destroy_sample(sfx_jump);
        if (sfx_falling) al_destroy_sample(sfx_falling);
        if (sfx_hit_level) al_destroy_sample(sfx_hit_level);
        if (sfx_hit_bonus) al_destroy_sample(sfx_hit_bonus);
        if (sfx_game_over) al_destroy_sample(sfx_game_over);
        al_uninstall_audio();
    }

    ttfe_vbo_destroy(&ctx.g_ttfe_stream_vbo);

    game_context_free(&ctx);

    al_destroy_font(level_font);
    al_destroy_font(gui_font);
    al_destroy_timer(fps_timer);
    al_destroy_timer(logic_timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    FreeNoLog(level_font_file);
    FreeNoLog(gui_font_file);
    FreeNoLog(levels_file);

    return 0;
}
