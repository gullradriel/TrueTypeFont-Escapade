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
char** load_text_file_lines(const char* filename, int* out_count);

#ifdef __cplusplus
}
#endif

#endif
