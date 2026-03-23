<h1 align="center">
  <b>libc8</b>
</h1>

<h4 align="center">
  A C library and TUI toolkit for interpreting, assembling, and disassembling
  CHIP-8 and SCHIP code, optionally utilizing the [SDL2](https://www.libsdl.org/)
  library for graphics.
</h4>

[![CI Status](https://github.com/bmoneill/libc8/actions/workflows/cmake-single-platform.yml/badge.svg?branch=main)](https://github.com/bmoneill/libc8/actions/workflows/cmake-single-platform.yml)
[![Doxygen Status](https://github.com/bmoneill/libc8/actions/workflows/doxygen.yml/badge.svg?branch=main)](https://bmoneill.github.io/libc8)
[![Clang-format status](https://github.com/bmoneill/libc8/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/bmoneill/libc8/actions/workflows/clang-format.yml)
[![Coverage](https://oneill.sh/coverage/libc8)](https://github.com/bmoneill/libc8/actions/workflows/cmake-single-platform.yml)

## Table of contents

- [Overview](#overview)
  - [Important Terms](#important-terms)
- [Features](#features)
  - [Interpreter](#interpreter)
    - [Debug mode](#debug-mode)
  - [Assembler](#assembler)
  - [Disassembler](#disassembler)
- [Building](#building)
  - [SDL2](#sdl2)
- [Testing](#testing)
- [Showcase](#showcase)
- [Documentation](#documentation)
- [Further reading](#further-reading)
- [Bugs](#bugs)
- [License](#license)

## Overview

`libc8` provides tools for interpreting, assembling, and disassembling
CHIP-8 and SCHIP programs. It is also possible to utilize this library
in other software (e.g. an arcade program with support for multiple
architectures).

### Important Terms

Throughout the code and documentation, jargon is used to refer to specific
components and attributes related to CHIP-8. The table below describes some
of these terms in detail:

| Term | Definition                                      |
| ---- | ----------------------------------------------- |
| A    | The first nibble (`0xF000`) in an instruction   |
| X    | The second nibble (`0x0F00`) in an instruction  |
| Y    | The third nibble (`0x00F0`) in an instruction   |
| B    | The fourth nibble (`0x000F`) in an instruction  |
| KK   | The second byte (`0x00FF`) in an instruction    |
| NNN  | The last 3 nibbles (`0x0FFF`) in an instruction |
| I    | The explicit value of the `I` register          |
| [I]  | A pointer to memory starting at address `I`     |
| K    | Register number to store next keypress          |
| F    | Address where font data begins                  |
| HF   | Address where high-resolution font data begins  |
| R[X] | HP-48 flag register                             |

> [!IMPORTANT]
> You should also take a look at the quirks section in the [interpreter docs](docs/chip8.md)
> for quirk identifiers and what they mean.

## Features

> [!TIP]
> In-depth overviews of the [interpreter](docs/chip8.md), [assembler](docs/chip8as.md),
> and [disassembler](docs/chip8dis.md) are available in [docs/](docs/). Library
> documentation is available on the [GitHub Pages site](https://bmoneill.github.io/libc8).

### Interpreter

- CHIP-8 and SCHIP 1.1 instructions fully implemented
- Full sound support
- User-configurable quirk support (common quirks are enabled by default)
- Custom color palette support
- Support for various fonts (all [Octo](https://github.com/JohnEarnest/octo) fonts)

The interpreter passes all of [Timendus's CHIP-8 and SCHIP tests](https://github.com/Timendus/chip8-test-suite)
except for the `DISP WAIT` quirk (identified by `r` in libc8).

#### Debug mode

- Step
- Continue
- Load/save program state and flag registers
- Print relevant attributes

### Assembler

- Strings, 16-bit words, and bytes
- Support for binary and hex encoded values
- Labels

### Disassembler

- Address printing
- Label generation

## Building

Building is only officially supported on Linux, but it should be possible to
build on Windows and Mac with minimal difficulty.

You must have CMake installed with a minimum version of 3.31.6.

```bash
cmake -S . -B build
cmake --build build
sudo cmake --install build
```

This will build and install libc8 as well as the example tools.

### SDL2

SDL2 support is enabled by default. To disable it to use another graphics
library, run `cmake` with `-DSDL2=OFF`. If `c8_simulate()` is ever called in your
code, you must implement these functions with your preferred graphics library:

- [`void c8_deinit_graphics(void)`](https://bmoneill.github.io/libc8/graphics_8h.html#a2e8bbd8d2fd84b5deada3dd3bdc03ab5)
- [`int c8_init_graphics(void)`](https://bmoneill.github.io/libc8/graphics_8h.html#a10c02b36be48214fec64cc6a9d4f20e4)
- [`void c8_render(c8_display_t *, int *)`](https://bmoneill.github.io/libc8/graphics_8h.html#a57897d69496a19a080b3af70ce26c010)
- [`void c8_sound_play(void)`](https://bmoneill.github.io/libc8/graphics_8c.html#aeabec1bbe4ff6953cbd3ad4eb0ef069e)
- [`void c8_sound_stop(void)`](https://bmoneill.github.io/libc8/graphics_8c.html#ab4a5d4072d61da0f397cf9e8cac3d7c0)
- [`int c8_tick(int *, int)`](https://bmoneill.github.io/libc8/graphics_8h.html#a020c1df5341d906fb19266b94235f884)

**Note**: the `all` and `tools` targets require `SDL2` to be `ON`.

## Testing

Testing is done using
[ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) and
[Unity](https://github.com/ThrowTheSwitch/Unity).

```bash
git submodule update
cmake -S . -B build -DTEST=ON -DSDL2=OFF
cmake --build build
cd build
ctest --verbose
```

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
- [Timendus's CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite)

## Bugs

If you find a bug, submit an issue, PR, or email me with a description and/or patch.

## License

Copyright (c) 2019-2026 Ben O'Neill <ben@oneill.sh>. This work is released under
the terms of the MIT License. See [LICENSE](LICENSE) for the license terms.
