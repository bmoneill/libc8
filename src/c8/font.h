/**
 * @file c8/font.h
 *
 * Stuff related to loading fonts.
 */

#ifndef C8_FONT_H
#define C8_FONT_H

#include "chip8.h"

#define C8_FONT_START      0x000
#define C8_HIGH_FONT_START (C8_FONT_START + (0x10 * 5))

extern const char* c8_fontNames[2][5];

void               c8_print_fonts(C8*);
void               c8_set_fonts(C8*, int, int);
int                c8_set_fonts_s(C8*, char*);
int                c8_set_small_font(C8*, const char*);
int                c8_set_big_font(C8*, const char*);

#endif
