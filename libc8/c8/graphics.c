/**
 * @file c8/graphics.c
 *
 * Backend-agnostic graphics-related functions
 */

#include "graphics.h"

#include <stdio.h>

/**
 * @brief Play sound
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) void c8_beep(void) {
    fprintf(stderr, "c8_beep() not implemented.");
}

/**
 * @brief Deinitialize graphics system
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) void c8_deinit_graphics(void) {
    fprintf(stderr, "c8_deinit_graphics() not implemented.");
}

/**
 * @brief Initialize graphics system
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) uint8_t c8_init_graphics(void) {
    fprintf(stderr, "c8_init_graphics() not implemented.\n");
    return -1;
}

/**
 * @brief Render graphics
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) void c8_render(c8_display_t* display, int* colors) {
    fprintf(stderr, "c8_render() not implemented.");
}

/**
 * @brief Grab current keypresses and delay execution to match clockspeed
 *
 * This function is weak and is overridden by internal/graphics_sdl.c by
 * default. This can also be overridden by the user when compiling without
 * SDL2 support.
 */
__attribute__((weak)) int c8_tick(int* key) {
    fprintf(stderr, "tick() not implemented.\n");
    return -1;
}

/**
 * @brief Get the value of (x,y) from `display`
 *
 * @param display `display_t` to get pixel from
 * @param x the x value
 * @param y the y value
 *
 * @return pointer to value of (x,y) in `display`
 */
uint8_t* c8_get_pixel(c8_display_t* display, int x, int y) {
    if (display->mode == C8_DISPLAYMODE_HIGH) {
        x += display->x;
        y += display->y;
    }
    return &display->p[y * C8_LOW_DISPLAY_WIDTH + x];
}
