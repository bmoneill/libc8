/**
 * @file c8/graphics.h
 *
 * Function declarations for graphics display are here.
 *
 * Only `get_pixel` is strongly defined in graphics.c. Declarations are library
 * agnostic so a different graphics backend can be used.
 */

#ifndef C8_GRAPHICS_H
#define C8_GRAPHICS_H

#include <stdint.h>

#define C8_LOW_DISPLAY_WIDTH   64
#define C8_LOW_DISPLAY_HEIGHT  32
#define C8_HIGH_DISPLAY_WIDTH  128
#define C8_HIGH_DISPLAY_HEIGHT 64

#define C8_DEFAULT_WINDOW_WIDTH  800
#define C8_DEFAULT_WINDOW_HEIGHT 400
#define C8_WINDOW_SCALE_X        (C8_DEFAULT_WINDOW_WIDTH / C8_LOW_DISPLAY_WIDTH)
#define C8_WINDOW_SCALE_Y        (C8_DEFAULT_WINDOW_HEIGHT / C8_LOW_DISPLAY_HEIGHT)

#define C8_DISPLAYMODE_LOW  0
#define C8_DISPLAYMODE_HIGH 1

/**
  * @struct display_t
  * @brief Represents a graphics display.
  *
  * @param p pixels (1D array)
  * @param mode display mode (`DISPLAY_STANDARD` or `DISPLAY_EXTENDED`)
  * @param x x offset (for `DISPLAY_EXTENDED`)
  * @param y y offset (for `DISPLAY_EXTENDED`)
  */
typedef struct {
    uint8_t p[C8_HIGH_DISPLAY_WIDTH * C8_HIGH_DISPLAY_HEIGHT];
    uint8_t mode;
    uint8_t x, y;
} c8_display_t;

uint8_t*       c8_get_pixel(c8_display_t*, int, int);

extern void    c8_beep(void);
extern void    c8_deinit_graphics(void);
extern uint8_t c8_init_graphics(void);
extern void    c8_render(c8_display_t*, int*);
extern int     c8_tick(int*);

#endif
