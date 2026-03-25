<h1 align="center">
    <b>chip8as</b>
</h1>

<h4 align="center">
  This is an assembler for the CHIP-8 and SCHIP, utilizing libc8.
</h4>

## Table of Contents

- [Features](#features)
- [Usage](#usage)
- [Example Code](#example-code)
- [Instructions](#instructions)
- [Notes](#notes)

## Features

- Label support (`mylabel:`)
- Support for strings (`.DS`), 16-bit data words (`.DW`),
  and data bytes (`.DB`)
- Support for hex (`0x` or `x` prefixed), and binary (`0b`) integers

## Usage

```bash
chip8as [-vV] [-o outputfile] src
```

- `-o` sets an output file (default is `a.c8`).
- `-v` prints diagnostic messages and the resulting hex-encoded bytecode to
  standard output.
- `-V` prints the version number.

## Example Code

```asm
.DS "mystring"
JP mylabel

mysprite:
.DW 0x1234

mylabel:
ADD V0, 1
ADD V1, 0xA
LD I, mysprite
DRW V0, V1, 1
```

## Instructions

| Assembly code   | Bytecode  | Notes                                                                     |
| --------------- | --------- | ------------------------------------------------------------------------- |
| `.DB byte`      | `ax`      | Special instruction                                                       |
| `.DW word`      | `axyb`    | Special instruction                                                       |
| `.DS "string"`  | `axyb`... | Special instruction. String will be stored as ASCII-encoded bytes.        |
| `SCD b`         | `00Cb`    | SCHIP instruction                                                         |
| `CLS`           | `00E0`    | SCHIP instruction                                                         |
| `RET`           | `00EE`    | SCHIP instruction                                                         |
| `SCR`           | `00FB`    | SCHIP instruction                                                         |
| `SCL`           | `00FC`    | SCHIP instruction                                                         |
| `EXIT`          | `00FD`    | SCHIP instruction                                                         |
| `LOW`           | `00FE`    | SCHIP instruction                                                         |
| `HIGH`          | `00FF`    | SCHIP instruction                                                         |
| `JP nnn`        | `1nnn`    |                                                                           |
| `CALL nnn`      | `2nnn`    |                                                                           |
| `SE Vx, kk`     | `3xkk`    |                                                                           |
| `SNE Vx kk`     | `4xkk`    |                                                                           |
| `SE Vx, Vy`     | `5xy0`    |                                                                           |
| `LD Vx, kk`     | `6xkk`    |                                                                           |
| `ADD Vx, kk`    | `7xkk`    |                                                                           |
| `LD Vx, Vy`     | `8xy0`    |                                                                           |
| `OR Vx, Vy`     | `8xy1`    |                                                                           |
| `AND Vx, Vy`    | `8xy2`    |                                                                           |
| `XOR Vx, Vy`    | `8xy3`    |                                                                           |
| `ADD Vx, Vy`    | `8xy4`    |                                                                           |
| `SUB Vx, Vy`    | `8xy5`    |                                                                           |
| `SHR Vx {, Vy}` | `8xy5`    | `Vy` is optional - it will be set to 0 in resulting bytecode if undefined |
| `SUBN Vx, Vy`   | `8xy7`    |                                                                           |
| `SHL Vx {, Vy}` | `8xyE`    | `Vy` is optional - it will be set to 0 in resulting bytecode if undefined |
| `SNE Vx, Vy`    | `9xy0`    |                                                                           |
| `LD I, nnn`     | `Annn`    |                                                                           |
| `JP V0, nnn`    | `Bnnn`    |                                                                           |
| `RND Vx, kk`    | `Cxkk`    |                                                                           |
| `DRW Vx, Vy, b` | `Dxyb`    |                                                                           |
| `SKP Vx`        | `Ex9E`    |                                                                           |
| `SKNP Vx`       | `ExA1`    |                                                                           |
| `LD Vx, DT`     | `Fx07`    |                                                                           |
| `LD Vx, K`      | `Fx0A`    |                                                                           |
| `LD DT, Vx`     | `Fx15`    |                                                                           |
| `LD ST, Vx`     | `Fx18`    |                                                                           |
| `ADD I, Vx`     | `Fx1E`    |                                                                           |
| `LD F, Vx`      | `Fx29`    |                                                                           |
| `LD HF, Vx`     | `Fx30`    | SCHIP instruction                                                         |
| `LD B, Vx`      | `Fx33`    |                                                                           |
| `LD [I], Vx`    | `Fx55`    |                                                                           |
| `LD Vx, [I]`    | `Fx65`    |                                                                           |
| `LD R, Vx`      | `Fx75`    | SCHIP instruction                                                         |
| `LD Vx, R`      | `Fx85`    | SCHIP instruction                                                         |

## Notes

- Hex integers, besides V register identifiers, must be formatted with `0x`,
  `x`, or `$` prefixes.
- Binary integers must be formatted with a `0b` prefix.
- Data bytes and strings are **not** padded. If a `.DB` or `.DS` contains an odd
  number of bytes, following bytecode will be offset (the first byte of
  following instructions will be at odd-numbered addresses).
- Commas are optional for instruction parameters.
- This assembler is not case sensitive.
