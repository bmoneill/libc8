# chip8 (CHIP-8 Interpreter)

This is a fully functional CHIP-8 and SCHIP interpreter with an
integrated debug mode, utilizing libc8 with SDL2.

## Table of Contents

- [Usage](#usage)
- [Options](#options)
- [Keyboard Layout](#keyboard-layout)
- [Fonts](#fonts)
- [Quirks](#quirks)
- [Debug mode](#debug-mode)

## Usage

```bash
chip8 [-dsvV] [-c tickspeed] [-f small,big] [-p file] [-P colors] [-q quirks] file
```

### Options

| Option | Description                                                                                                                      |
| ------ | -------------------------------------------------------------------------------------------------------------------------------- |
| `-c`   | Sets the number of instructions to be executed per second (**default: 1000**).                                                   |
| `-d`   | Enables debug mode. This can be used to add breakpoints, display the current memory, and step through instructions individually. |
| `-f`   | Loads the specified comma-separated fonts. Big font is optional.                                                                 |
| `-p`   | Loads a color palette from a file containing two newline-separated 24-bit hex codes (prefixed by `0x` or `x`).                   |
| `-P`   | Sets the color palette from a string containing two comma-separated 24-bit hex codes (prefixed by `0x` or `x`).                  |
| `-q`   | Sets the quirks to enable from string with non-separated quirk identifiers                                                       |
| `-s`   | Enables SCHIP mode.                                                                                                              |
| `-v`   | Enables verbose mode. This will print each instruction that is executed.                                                         |
| `-V`   | Prints the version number.                                                                                                       |

## Keyboard Layout

```shell
  Key       CHIP-8 keycode
1 2 3 4        1 2 3 C
q w e r  ==>   4 5 6 D
a s d f        7 8 9 E
z x c v        A 0 B F
```

## Fonts

Same as Octo.

| Font      | Small | Big |
| --------- | ----- | --- |
| octo      | ☑    | ☑  |
| schip     | ☑    | ☑  |
| vip       | ☑    | ❌  |
| dream6800 | ☑    | ❌  |
| eti660    | ☑    | ❌  |
| fish      | ☑    | ☑  |

## Quirks

The following quirks are available:

| Quirk | Definition                                                              | CHIP-8 Default | SCHIP Default |
| ----- | ----------------------------------------------------------------------- | -------------- | ------------- |
| `c`   | Clip sprites at bottom edge of screen instead of wrapping.              | ☑             | ☑            |
| `j`   | For `JP V0, nnn`: Jump to `nnn + V[(nnn>>8)&0xF]` instead.              | ❌             | ☑            |
| `m`   | After `LD [I], Vx` and `LD Vx, [I]`: Set `I` to address `I + x + 1`.    | ☑             | ❌            |
| `r`   | `DRW` halts execution until the next frame.                             | ☑             | ☑            |
| `s`   | Before `SHL Vx, Vy` and `SHR Vx, Vy`: Shift `Vx` in place, ignore `Vy`. | ❌             | ☑            |
| `v`   | After `AND`, `OR`, and `XOR`: Clear `VF`.                               | ☑             | ❌            |

## Debug mode

Debug mode can be enabled via the `-d` command-line argument or by pressing P at
any time during execution. It can also be disabled at any time by pressing M.

The following commands and attributes are supported in debug mode:

| Command               | Description                                                           |
| --------------------- | --------------------------------------------------------------------- |
| `break [ADDRESS]`     | Add breakpoint to `PC` or `ADDRESS`, if given.                        |
| `rmbreak [ADDRESS]`   | Remove breakpoint at `PC` or `ADDRESS`, if given and exists.          |
| `continue`            | Exit debug mode until next breakpoint or until execution is complete. |
| `help`                | Print a help string.                                                  |
| `load PATH`           | Load program state from `PATH`.                                       |
| `loadflags PATH`      | Load flag registers from `PATH`.                                      |
| `next`                | Step to the next instruction.                                         |
| `print [ATTRIBUTE]`   | Print current value of the given attribute.                           |
| `quit`                | Terminate the program.                                                |
| `save PATH`           | Save program state to `PATH`.                                         |
| `saveflags PATH`      | Save flag registers to `PATH`.                                        |
| `set ATTRIBUTE VALUE` | Set the given attribute to the given value.                           |

| Attribute    | Description                                           |
| ------------ | ----------------------------------------------------- |
| `PC`         | Program counter                                       |
| `SP`         | Stack pointer                                         |
| `DT`         | Delay timer                                           |
| `ST`         | Sound timer                                           |
| `I`          | I value                                               |
| `K`          | Register to store next keypress                       |
| `V[x]`       | All V register values or value of Vx, if given        |
| `R[x]`       | All R (flag) register values or value of Rx, if given |
| `bfont`      | Big font name                                         |
| `sfont`      | Small font name                                       |
| `stack`      | All stack values                                      |
| `bg`         | Background color                                      |
| `fg`         | Foreground color                                      |
| `$[address]` | Value at given address                                |

> ![NOTE]
> If no argument is given to `print`, it will print all of the above attributes
> except for address values.
