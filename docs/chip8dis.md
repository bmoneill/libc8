<h1 align="center">
    <b>chip8dis</b>
</h1>

<h4 align="center">
    This is a disassembler for the CHIP-8 and SCHIP, utilizing libc8.
</h4>

## Usage

```bash
chip8dis [-alV] [-o outputfile] rom
```

- `-a` toggles printing of addresses.
- `-l` toggles printing of auto-generated labels.
- `-o` writes the output to `outputfile`.
- `-V` prints the version number.

By default, `c8dis` will write to `stdout`.
