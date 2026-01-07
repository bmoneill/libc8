/**
 * @file c8/font.h
 *
 * Stuff related to loading fonts.
 */

#ifndef C8_FONT_H
#define C8_FONT_H

#include "chip8.h"

/**
 * @brief Font storage address in memory
 */
#define C8_FONT_START 0x000

/**
 * @brief HD font storage address in memory
 */
#define C8_HIGH_FONT_START (C8_FONT_START + (0x10 * 5))

/**
 * @brief Small font IDs
 */
typedef enum {
    C8_SMALLFONT_OCTO      = 0,
    C8_SMALLFONT_VIP       = 1,
    C8_SMALLFONT_DREAM6800 = 2,
    C8_SMALLFONT_ETI660    = 3,
    C8_SMALLFONT_FISH      = 4,
} C8_SmallFont;

/**
 * @brief Big font IDs
 */
typedef enum {
    C8_BIGFONT_OCTO  = 0,
    C8_BIGFONT_SCHIP = 1,
    C8_BIGFONT_FISH  = 2,
} C8_BigFont;

/**
 * @brief Font names
 */
extern const char* c8_fontNames[2][5];

void               c8_print_fonts(C8*);
void               c8_set_fonts(C8*, C8_SmallFont, C8_BigFont);
int                c8_set_fonts_s(C8*, char*);
int                c8_set_small_font(C8*, const char*);
int                c8_set_big_font(C8*, const char*);

#endif
