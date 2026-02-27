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

/**
 * @brief Width of the low-resolution display.
 */
#define C8_LOW_DISPLAY_WIDTH 64

/**
 * @brief Height of the low-resolution display.
 */
#define C8_LOW_DISPLAY_HEIGHT 32

/**
 * @brief Width of the high-resolution display.
 */
#define C8_HIGH_DISPLAY_WIDTH 128

/**
 * @brief Height of the high-resolution display.
 */
#define C8_HIGH_DISPLAY_HEIGHT 64

/**
 * @brief Default window width.
 */
#define C8_DEFAULT_WINDOW_WIDTH 800

/**
 * @brief Default window height.
 */
#define C8_DEFAULT_WINDOW_HEIGHT 400

/**
 * @brief Window scale factor in x direction.
 */
#define C8_WINDOW_SCALE_X (C8_DEFAULT_WINDOW_WIDTH / C8_LOW_DISPLAY_WIDTH)

/**
 * @brief Window scale factor in y direction.
 */
#define C8_WINDOW_SCALE_Y (C8_DEFAULT_WINDOW_HEIGHT / C8_LOW_DISPLAY_HEIGHT)

/**
 * @brief Low resolution display mode.
 */
#define C8_DISPLAYMODE_LOW 0

/**
 * @brief High resolution display mode.
 */
#define C8_DISPLAYMODE_HIGH 1

/**
  * @struct C8_Display
  * @brief Represents a graphics display.
  *
  */
typedef struct {
    uint8_t p[C8_HIGH_DISPLAY_WIDTH * C8_HIGH_DISPLAY_HEIGHT]; //!< Pixels (1D array)
    uint8_t mode; //!< Display mode (`C8_DISPLAYMODE_LOW` or `C8_DISPLAYMODE_HIGH`)
    uint8_t x; //!< Offset (for `C8_DISPLAYMODE_LOW`)
    uint8_t y; //!< Offset (for `C8_DISPLAYMODE_HIGH`)
} C8_Display;

uint8_t*       c8_get_pixel(C8_Display*, int, int);

extern void    c8_end_sound(void);
extern void    c8_start_sound(void);
extern void    c8_deinit_graphics(void);
extern uint8_t c8_init_graphics(void);
extern void    c8_render(C8_Display*, int*);
extern int     c8_tick(int*);

#endif
