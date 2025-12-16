/**\file ttfe_text.h
 *  Text loading functions
 *\author Castagnier Mickael aka Gull Ra Driel
 *\version 1.0
 *\date 08/12/2025
 */

#ifndef TTFE_TEXT_HEADER_FOR_HACKS
#define TTFE_TEXT_HEADER_FOR_HACKS

#ifdef __cplusplus
extern "C" {
#endif

/* Generic text file loading */
static inline char** load_text_file_lines(const char* filename, int* out_count) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        n_log(LOG_ERR, "Cannot open text file '%s'", filename);
        *out_count = 0;
        return NULL;
    }

    int capacity = 8;
    int count = 0;
    char** lines = (char**)malloc(sizeof(char*) * capacity);
    if (!lines) {
        fclose(f);
        n_log(LOG_ERR, "malloc failed for lines of '%s'", filename);
        *out_count = 0;
        return NULL;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), f)) {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[--len] = '\0';
        }
        if (len == 0) {
            continue; /* skip empty lines */
        }
        if (count >= capacity) {
            capacity *= 2;
            char** tmp = (char**)realloc(lines, sizeof(char*) * capacity);
            if (!tmp) {
                n_log(LOG_ERR, "realloc failed for lines of '%s'", filename);
                for (int i = 0; i < count; ++i) free(lines[i]);
                free(lines);
                fclose(f);
                *out_count = 0;
                return NULL;
            }
            lines = tmp;
        }
        char* line = strdup(buffer);
        if (!line) {
            n_log(LOG_ERR, "strdup failed for a line of '%s'", filename);
            for (int i = 0; i < count; ++i) free(lines[i]);
            free(lines);
            fclose(f);
            *out_count = 0;
            return NULL;
        }
        lines[count++] = line;
    }
    fclose(f);

    if (count == 0) {
        n_log(LOG_ERR, "No non-empty lines in '%s'", filename);
        free(lines);
        *out_count = 0;
        return NULL;
    }

    *out_count = count;
    return lines;
}

#ifdef __cplusplus
}
#endif

#endif
