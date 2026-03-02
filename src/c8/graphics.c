/**
 * @file c8/graphics.c
 *
 * Backend-agnostic graphics-related functions
 */

#include "graphics.h"
#include "private/exception.h"

#include <stdio.h>

/**
 * @brief Stop sound
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) void c8_sound_play(void) {
    C8_EXCEPTION(C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION, "%s not implemented.", __func__);
}

/**
 * @brief Play sound
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) void c8_sound_stop(void) {
    C8_EXCEPTION(C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION, "%s not implemented.", __func__);
}

/**
 * @brief Deinitialize graphics system
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) void c8_deinit_graphics(void) {
    C8_EXCEPTION(C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION, "%s not implemented.", __func__);
}

/**
 * @brief Initialize graphics system
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) uint8_t c8_init_graphics(void) {
    C8_EXCEPTION(C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION, "%s not implemented.", __func__);
    return C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION;
}

/**
 * @brief Render graphics
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) void c8_render(C8_Display* display, int* colors) {
    C8_EXCEPTION(C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION, "%s not implemented.", __func__);
}

/**
 * @brief Grab current keypresses and delay execution to match clockspeed
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) int c8_tick(int* key) {
    C8_EXCEPTION(C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION, "%s not implemented.", __func__);
    return C8_FAILED_GRAPHICS_INITIALIZATION_EXCEPTION;
}

/**
 * @brief Get the value of (x,y) from `display`
 *
 * @param display `C8_Display` to get pixel from
 * @param x the x value
 * @param y the y value
 *
 * @return pointer to value of (x,y) in `display`
 */
uint8_t* c8_get_pixel(C8_Display* display, int x, int y) {
    if (display->mode == C8_DISPLAYMODE_HIGH) {
        x += display->x;
        y += display->y;
    }
    return &display->p[y * C8_LOW_DISPLAY_WIDTH + x];
}
