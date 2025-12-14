/**\file states_management.c
 *  Config file management
 *\author Castagnier Mickaël aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#include "ttfe_app_config.h"
#include "cJSON.h"
#include "nilorea/n_str.h"

int load_app_config(char* state_filename, long int* WIDTH, long int* HEIGHT, bool* fullscreen, char** intro_sample, char** win_sample, char** loose_sample, char** shoot_sample, char** jump_sample, char** hit_level_sample, char** hit_bonus_sample, char** game_over_sample, double* fps, char** level_font_file, int* level_font_size, char** gui_font_file, int* gui_font_size, char** levels_file, float* gravity, float* jump_vel, float* config_base_speed, float* speed_bonus_increment, float* speed_max_limit, float* mouse_sensitivity, float* bullet_speed, int* bullet_delta_divider) {
    __n_assert(state_filename, return FALSE);

    cJSON* monitor_json = NULL;
    N_STR* data = NULL;

    if (access(state_filename, F_OK) != 0) {
        n_log(LOG_ERR, "no app state %s to load !", state_filename);
        goto error_exit;
    }

    data = file_to_nstr(state_filename);
    if (!data) {
        n_log(LOG_ERR, "Error reading file %s, defaults will be used",
              state_filename);
        goto error_exit;
    }

    monitor_json = cJSON_Parse(_nstr(data));
    if (monitor_json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        n_log(LOG_ERR, "%s: Error before: %s",
              state_filename, _str(error_ptr));
        goto error_exit;
    }

    cJSON* value = NULL;
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "width");
    if (cJSON_IsNumber(value)) {
        (*WIDTH) = value->valueint;
    } else {
        n_log(LOG_ERR, "width is not a number");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "height");
    if (cJSON_IsNumber(value)) {
        (*HEIGHT) = value->valueint;
    } else {
        n_log(LOG_ERR, "height is not a number");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "fullscreen");
    if (cJSON_IsNumber(value)) {
        (*fullscreen) = value->valueint;
    } else {
        n_log(LOG_ERR, "fullscreen is not a number");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "intro-sample");
    if (cJSON_IsString(value)) {
        (*intro_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "intro-sample is not a string");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "win-sample");
    if (cJSON_IsString(value)) {
        (*win_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "win-sample is not a string");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "loose-sample");
    if (cJSON_IsString(value)) {
        (*loose_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "loose-sample is not a string");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "shoot-sample");
    if (cJSON_IsString(value)) {
        (*shoot_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "shoot-sample is not a string");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "jump-sample");
    if (cJSON_IsString(value)) {
        (*jump_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "jump-sample is not a string");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "hit-level-sample");
    if (cJSON_IsString(value)) {
        (*hit_level_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "hit-level-sample is not a string");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "hit-bonus-sample");
    if (cJSON_IsString(value)) {
        (*hit_bonus_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "hit-bonus-sample is not a string");
        goto error_exit;
    }
    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "game-over-sample");
    if (cJSON_IsString(value)) {
        (*game_over_sample) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "game-over-sample is not a string");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "gravity");
    if (cJSON_IsNumber(value)) {
        (*gravity) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "gravity is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "jump-vel");
    if (cJSON_IsNumber(value)) {
        (*jump_vel) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "jump-vel is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "config-base-speed");
    if (cJSON_IsNumber(value)) {
        (*config_base_speed) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "config-base-speed is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "speed-bonus-increment");
    if (cJSON_IsNumber(value)) {
        (*speed_bonus_increment) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "speed-bonus-increment is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "speed-max-limit");
    if (cJSON_IsNumber(value)) {
        (*speed_max_limit) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "speed-max-limit is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "mouse-sensitivity");
    if (cJSON_IsNumber(value)) {
        (*mouse_sensitivity) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "mouse-sensitivity is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "bullet-speed");
    if (cJSON_IsNumber(value)) {
        (*bullet_speed) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "bullet-speed is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "bullet-delta-divider");
    if (cJSON_IsNumber(value)) {
        (*bullet_delta_divider) = value->valueint;
    } else {
        n_log(LOG_ERR, "bullet-delta-divider is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "fps");
    if (cJSON_IsNumber(value)) {
        (*fps) = value->valuedouble;
    } else {
        n_log(LOG_ERR, "fps is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "level-font-size");
    if (cJSON_IsNumber(value)) {
        (*level_font_size) = value->valueint;
    } else {
        n_log(LOG_ERR, "level-font-size is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "gui-font-size");
    if (cJSON_IsNumber(value)) {
        (*gui_font_size) = value->valueint;
    } else {
        n_log(LOG_ERR, "gui-font-size is not a number");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "level-font-file");
    if (cJSON_IsString(value)) {
        (*level_font_file) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "level-font-file is not a string");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "levels-file");
    if (cJSON_IsString(value)) {
        (*levels_file) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "levels-file is not a string");
        goto error_exit;
    }

    value = cJSON_GetObjectItemCaseSensitive(monitor_json, "gui-font-file");
    if (cJSON_IsString(value)) {
        (*gui_font_file) = strdup(value->valuestring);
    } else {
        n_log(LOG_ERR, "gui-font-file is not a string");
        goto error_exit;
    }

    cJSON_Delete(monitor_json);
    free_nstr(&data);

    return TRUE;

error_exit:

    if (monitor_json) {
        cJSON_Delete(monitor_json);
    }
    if (data) {
        free_nstr(&data);
    }
    return FALSE;
}
