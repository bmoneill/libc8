/**
 * @file c8/private/graphics_ncurses.c
 * @note NOT EXPORTED
 *
 * ncurses graphics implementation. When `NCURSES` is defined, overrides weak
 * graphics utility function definitions in `graphics.c`.
 */

#include "../common.h"
#include "../graphics.h"
#include "exception.h"

#include <ncurses.h>
#include <string.h>

#ifdef X11
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#endif

C8_STATIC const int c8_keyMap[18][2] = {
    { '1', 1 },   { '2', 2 },   { '3', 3 },   { '4', 0xC }, { 'q', 4 },  { 'w', 5 },
    { 'e', 6 },   { 'r', 0xD }, { 'a', 7 },   { 's', 8 },   { 'd', 9 },  { 'f', 0xE },
    { 'z', 0xA }, { 'x', 0 },   { 'c', 0xB }, { 'v', 0xF }, { 'p', 16 }, { 'm', 17 },
};

C8_STATIC int cursor_visibility;

#ifdef X11
Display* x11display;

unsigned c8_old_xkb_delay;
unsigned c8_old_xkb_rate;
#endif

C8_STATIC int c8_get_key(char k);

int           c8_init_graphics(void) {
    initscr();
    cbreak();
    noecho();
    noqiflush();
    nodelay(stdscr, TRUE);

#ifdef X11
    x11display = XOpenDisplay(NULL);
    if (!x11display) {
        C8_EXCEPTION(C8_GRAPHICS_EXCEPTION, "Failed to open X11 display");
        endwin();
        return C8_GRAPHICS_EXCEPTION;
    }

    XkbGetAutoRepeatRate(x11display, XkbUseCoreKbd, &c8_old_xkb_rate, &c8_old_xkb_delay);
    Status s = XkbSetAutoRepeatRate(x11display, XkbUseCoreKbd, 10, 10);

    if (s != True) {
        C8_EXCEPTION(C8_GRAPHICS_EXCEPTION, "Failed to set X11 auto-repeat rate");
        endwin();
        return C8_GRAPHICS_EXCEPTION;
    }
    XFlush(x11display);
#endif

    cursor_visibility = curs_set(0);

    start_color();
    init_pair(1, COLOR_CYAN, COLOR_RED);

    return 0;
}

/**
 * @brief Start playing the sound.
 *
 * @return 0 if successful, error code otherwise.
 */
int c8_sound_play(void) {
    // TODO implement
    return 0;
}

/**
 * @brief Stop the sound playing.
 *
 * @return 0 if successful, error code otherwise.
 */
int c8_sound_stop(void) {
    // TODO implement
    return 0;
}

/**
 * @brief Deinitialize the graphics library.
 */
int c8_deinit_graphics(void) {
#ifdef X11
    XkbSetAutoRepeatRate(x11display, XkbUseCoreKbd, c8_old_xkb_rate, c8_old_xkb_delay);
    XFlush(x11display);
    XCloseDisplay(x11display);
#endif

    if (cursor_visibility == ERR) {
        curs_set(1);
    } else {
        curs_set(cursor_visibility);
    }
    echo();
    qiflush();
    endwin();
    return 0;
}

/**
 * Render the given display to the SDL2 window.
 *
 * @param display `C8_Display` to render
 * @param colors colors to render
 * @return 0 on success, non-zero on failure
 */
int c8_render(C8_Display* display, int* colors) {
    int display_width
        = (display->mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_WIDTH : C8_HIGH_DISPLAY_WIDTH;
    int display_height
        = (display->mode == C8_DISPLAYMODE_LOW) ? C8_LOW_DISPLAY_HEIGHT : C8_HIGH_DISPLAY_HEIGHT;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    if (rows < display_height || cols < display_width) {
        C8_EXCEPTION(C8_GRAPHICS_EXCEPTION, "Window too small.")
        return C8_GRAPHICS_EXCEPTION;
    }

    for (int y = 0; y < display_height; y++) {
        for (int x = 0; x < display_width; x++) {
            if (*c8_get_pixel(display, x, y)) {
                attron(A_REVERSE);
                mvaddch(y, x, ' ');
                attroff(A_REVERSE);
            } else {
                mvaddch(y, x, ' ');
            }
        }
    }

    refresh();
    return 0;
}

/**
 * @brief Process keypresses and delay based on clockspeed.
 *
 * If a relevant key is pressed or released (see `keyMap` in this file), this
 * function will update `key` accordingly.
 *
 * @param key pointer to int arr of keys
 *
 * @return -2 if quitting, -1 if no key was released, else returns value
 * of key released.
 */
int c8_tick(int* keys) {
    int released = -1;

    int current_keys[18];
    memset(current_keys, 0, sizeof(current_keys));

    int c = ERR;
    while ((c = getch()) != ERR) {
        int k = c8_get_key(c);

        if (k > -1) {
            if (current_keys[k]) {
                break; // key already processed, exit loop
            }

            current_keys[k] = 1;
        }
    }

    for (int i = 0; i < 18; i++) {
        if (!current_keys[i] && keys[i]) {
            released = i;
        }
    }

    memcpy(keys, current_keys, sizeof(current_keys));

    return released;
}

/**
 * @brief Convert the given SDL Keycode to a CHIP-8 keycode.
 *
 * @param k the SDL_Keycode
 *
 * @return the CHIP-8 keycode, or -1 if no match is found.
 */
C8_STATIC int c8_get_key(char c) {
    for (int i = 0; i < 18; i++) {
        if (c8_keyMap[i][0] == c) {
            return c8_keyMap[i][1];
        }
    }
    return -1;
}
