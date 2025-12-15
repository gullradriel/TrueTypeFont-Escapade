/**\file ttfe_app_config.h
 *  Config file management
 *\author Castagnier Micka�l aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef TTFE_STATES_HEADER_FOR_HACKS
#define TTFE_STATES_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

int load_app_config(char* state_filename, long int* WIDTH, long int* HEIGHT, bool* fullscreen, char** intro_sample, char** win_sample, char** lose_sample, char** shoot_sample, char** jump_sample, char** hit_level_sample, char** hit_bonus_sample, char** game_over_sample, double* fps, char** level_font_file, int* level_font_size, char** gui_font_file, int* gui_font_size, char** levels_file, float* gravity, float* jump_vel, float* config_base_speed, float* SPEED_BONUS_INCREMENT, float* SPEED_MAX_LIMIT, float* mouse_sensitivity, float* bullet_speed, int* bullet_delta_divider);

#ifdef __cplusplus
}
#endif

#endif
