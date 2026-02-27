# libc8

[![CI Status](https://github.com/bmoneill/libc8/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)](https://github.com/bmoneill/libc8/actions/workflows/cmake-single-platform.yml)
[![Doxygen Status](https://github.com/bmoneill/libc8/actions/workflows/doxygen.yml/badge.svg?branch=main)](https://bmoneill.github.io/libc8)
[![Clang-format status](https://github.com/bmoneill/libc8/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/bmoneill/libc8/actions/workflows/clang-format.yml)

This is a C library for interpreting, assembling, and disassembling CHIP-8 and
SCHIP code, optionally utilizing the [SDL2](https://www.libsdl.org/) library
for graphics.

An example [assembler](docs/chip8as.md), [disassembler](docs/chip8dis.md), and
[interpreter](docs/chip8.md) is located in `tools/`.

## Building

### Linux

```shell
cmake -S . -B build
cmake --build build
sudo cmake --install build
```

This will build libc8 as well as the example tools.

### SDL2

SDL2 support is enabled by default. To disable it to use another graphics
library, run `cmake` with `-DSDL2=OFF`. If `simulate()` is ever called in your
code, you must implement these functions with your preferred graphics library:

- [`void c8_deinit_graphics(void)`](https://bmoneill.github.io/libc8/graphics_8h.html#a2e8bbd8d2fd84b5deada3dd3bdc03ab5)
- `void c8_end_sound(void)`
- [`int c8_init_graphics(void)`](https://bmoneill.github.io/libc8/graphics_8h.html#a10c02b36be48214fec64cc6a9d4f20e4)
- [`void c8_render(c8_display_t *, int *)`](https://bmoneill.github.io/libc8/graphics_8h.html#a57897d69496a19a080b3af70ce26c010)
- `void c8_start_sound(void)`
- [`int c8_tick(int *, int)`](https://bmoneill.github.io/libc8/graphics_8h.html#a020c1df5341d906fb19266b94235f884)

**Note**: the `all` and `tools` targets require `SDL2` to be `ON`.

## Testing

Testing is done using
[ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) and
[Unity](https://github.com/ThrowTheSwitch/Unity).

```shell
git submodule update
cmake -S . -B build -DTEST=ON -DSDL2=OFF
cmake --build build
cd build
ctest --verbose
```

**Note**: When built with tests enabled (`-DTEST`),
`libc8` will not halt execution after encountering an error, potentially leading
to undefined behavior.

## Showcase

The libc8 CHIP-8 interpreter running [Outlaw by John Earnest](https://johnearnest.github.io/chip8Archive/play.html?p=outlaw):

![libc8 running outlaw](https://oneill.sh/img/libc8-outlaw.gif)

## Documentation

[Library documentation for libc8](https://bmoneill.github.io/libc8) is
available on the GitHub pages site.

## Further Reading

- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [S-CHIP 1.1 Reference](http://devernay.free.fr/hacks/chip8/schip.txt)
- [Octo](https://github.com/JohnEarnest/Octo)
- [CHIP-8 Extensions and Compatibility](https://chip-8.github.io/extensions/)

## Bugs

If you find a bug, submit an issue, PR, or email me with a description and/or patch.

## License

Copyright (c) 2019-2026 Ben O'Neill <ben@oneill.sh>. This work is released under
the terms of the MIT License. See [LICENSE](LICENSE) for the license terms.
