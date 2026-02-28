/**
 * @file c8/private/graphics_sdl2.c
 * @note NOT EXPORTED
 *
 * SDL2 graphics implementation. When `SDL2` is defined, overrides weak
 * graphics utility function definitions in `graphics.c`.
 */

#include "../common.h"
#include "../graphics.h"
#include "exception.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>
#include <stdint.h>

#define C8_RGB_R(i) ((i >> 12) & 0xFF)
#define C8_RGB_G(i) ((i >> 8) & 0xFF)
#define C8_RGB_B(i) (i & 0xFF)

#define C8_AUDIO_CHANNEL     1
#define C8_AUDIO_SAMPLE_RATE 44100
#define C8_AUDIO_WAVE_FREQ   440
#define C8_AUDIO_WAVE_LENGTH C8_AUDIO_SAMPLE_RATE / C8_AUDIO_WAVE_FREQ

C8_STATIC SDL_Window*   c8_window;
C8_STATIC SDL_Renderer* c8_renderer;
C8_STATIC int16_t       samples[C8_AUDIO_WAVE_LENGTH];
C8_STATIC Mix_Chunk*    c8_wave_chunk = NULL;

/**
 * Map of all keys to track.
 *
 * * `c8_keyMap[x][0]` is `SDL_Keycode`
 * * `c8_keyMap[x][1]` is CHIP-8 keycode
 * * `c8_keyMap[16]` enables debug mode / step,
 * * `c8_keyMap[17]` disables debug mode
 */
C8_STATIC const int c8_keyMap[18][2] = {
    { SDLK_1, 1 },   { SDLK_2, 2 },   { SDLK_3, 3 },   { SDLK_4, 0xC }, { SDLK_q, 4 },
    { SDLK_w, 5 },   { SDLK_e, 6 },   { SDLK_r, 0xD }, { SDLK_a, 7 },   { SDLK_s, 8 },
    { SDLK_d, 9 },   { SDLK_f, 0xE }, { SDLK_z, 0xA }, { SDLK_x, 0 },   { SDLK_c, 0xB },
    { SDLK_v, 0xF }, { SDLK_p, 16 }, // Enter debug mode
    { SDLK_m, 17 }, // Leave debug mode
};

C8_STATIC int c8_get_key(SDL_Keycode k);

void          c8_sound_play(void) {
    if (c8_wave_chunk == NULL) {
        printf("Beep\n");
        uint32_t bytes = (uint32_t) (C8_AUDIO_WAVE_LENGTH * sizeof(int16_t));
        c8_wave_chunk  = Mix_QuickLoad_RAW((uint8_t*) samples, bytes);

        if (!c8_wave_chunk) {
            Mix_GetError();
            printf("%s\n", SDL_GetError());
            C8_EXCEPTION(C8_AUDIO_EXCEPTION, "An error occurred while loading the audio chunk.");
        }
        c8_wave_chunk->volume = MIX_MAX_VOLUME / 4;
        Mix_PlayChannel(-1, c8_wave_chunk, -1);
    }
}

void c8_sound_stop(void) {
    Mix_HaltChannel(C8_AUDIO_CHANNEL);
    if (c8_wave_chunk) {
        Mix_FreeChunk(c8_wave_chunk);
        c8_wave_chunk = NULL;
    }
}

/**
 * @brief Deinitialize the graphics library.
 */
void c8_deinit_graphics(void) {
    SDL_DestroyRenderer(c8_renderer);
    SDL_DestroyWindow(c8_window);
    Mix_CloseAudio();
    SDL_Quit();
}

/**
 * @brief Initialize the graphics library.
 *
 * @return 1 if successful, 0 otherwise.
 */
uint8_t c8_init_graphics(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return 0;
    }

    if (!(c8_window = SDL_CreateWindow("CHIP8",
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       C8_DEFAULT_WINDOW_WIDTH,
                                       C8_DEFAULT_WINDOW_HEIGHT,
                                       SDL_WINDOW_RESIZABLE))) {
        return 0;
    }

    if (!(c8_renderer = SDL_CreateRenderer(c8_window, -1, SDL_RENDERER_ACCELERATED))) {
        SDL_DestroyWindow(c8_window);
        return 0;
    }

    if (Mix_OpenAudio(C8_AUDIO_SAMPLE_RATE, AUDIO_S16SYS, 1, 4096)) {
        SDL_DestroyRenderer(c8_renderer);
        SDL_DestroyWindow(c8_window);
        SDL_Quit();
        return 0;
    }

    Mix_AllocateChannels(1);

    for (int i = 0; i < C8_AUDIO_WAVE_LENGTH; i++) {
        samples[i] = i < C8_AUDIO_WAVE_LENGTH / 2 ? INT16_MAX : INT16_MIN;
    }
    printf("Graphics initialized successfully\n");
    return 1;
}

/**
 * Render the given display to the SDL2 window.
 *
 * @param display `C8_Display` to render
 * @param colors colors to render
 */
void c8_render(C8_Display* display, int* colors) {
    SDL_Rect pix = {
        .x = 0,
        .y = 0,
        .w = C8_WINDOW_SCALE_X,
        .h = C8_WINDOW_SCALE_Y,
    };
    SDL_Rect winRect = {
        .x = 0,
        .y = 0,
        .w = C8_LOW_DISPLAY_WIDTH,
        .h = C8_LOW_DISPLAY_HEIGHT,
    };

    int dx = 0;
    int dy = 0;

    SDL_RenderClear(c8_renderer);
    SDL_SetRenderDrawColor(c8_renderer,
                           C8_RGB_R(colors[0]),
                           C8_RGB_G(colors[0]),
                           C8_RGB_B(colors[0]),
                           SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(c8_renderer, &winRect);
    SDL_SetRenderDrawColor(c8_renderer,
                           C8_RGB_R(colors[1]),
                           C8_RGB_G(colors[1]),
                           C8_RGB_B(colors[1]),
                           SDL_ALPHA_OPAQUE);

    if (display->mode == C8_DISPLAYMODE_HIGH) {
        dx = display->x;
        dy = display->y;
    }

    for (int i = 0; i < C8_LOW_DISPLAY_WIDTH; i++) {
        for (int j = 0; j < C8_LOW_DISPLAY_HEIGHT; j++) {
            if (*c8_get_pixel(display, i + dx, j + dy)) {
                pix.x = i * C8_WINDOW_SCALE_X;
                pix.y = j * C8_WINDOW_SCALE_Y;
                SDL_RenderFillRect(c8_renderer, &pix);
            }
        }
    }

    SDL_SetRenderDrawColor(c8_renderer,
                           C8_RGB_R(colors[0]),
                           C8_RGB_G(colors[0]),
                           C8_RGB_B(colors[0]),
                           SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(c8_renderer, &winRect);

    SDL_RenderPresent(c8_renderer);
}

/**
 * @brief Process keypresses and delay based on clockspeed.
 *
 * If a relevant key is pressed or released (see `keyMap` in this file), this
 * function will update `key` accordingly.
 *
 * @param key pointer to int arr of keys
 *
 * @return -2 if quitting, -1 if no key was pressed/released, else returns value
 * of key pressed/released.
 */
int c8_tick(int* key) {
    SDL_Event e;
    int       ret = -1;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            return -2;
        case SDL_KEYDOWN:
            if ((ret = c8_get_key(e.key.keysym.sym)) != -1) {
                key[ret] = 1;
            }
            break;
        case SDL_KEYUP:
            if ((ret = c8_get_key(e.key.keysym.sym)) != -1) {
                key[ret] = 0;
            }
            break;
        }
    }

    return ret > 15 ? -1 : ret;
}

/**
 * @brief Convert the given SDL Keycode to a CHIP-8 keycode.
 *
 * @param k the SDL_Keycode
 *
 * @return the CHIP-8 keycode
 */
C8_STATIC int c8_get_key(SDL_Keycode k) {
    for (int i = 0; i < 18; i++) {
        if (c8_keyMap[i][0] == k) {
            return c8_keyMap[i][1];
        }
    }
    return -1;
}
