# ConText

**ConText** (Console-Text) is a minimalist text and code editor for Linux that runs directly in the terminal.

## Features
- Arrow keys (up, down, left, right)
- Insert and delete characters
- Backspace and Delete support
- Jump between lines
- Save file (`Ctrl + S`)
- Quit (`Ctrl + Q`)
- Save and quit (`Ctrl + X`)

## Build

```bash
gcc main.c src/file.c src/screen.c -Iinclude -o ConText
```

Run

```bash
./ConText <file>
```
License
GPL v3