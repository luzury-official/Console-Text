#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include "../include/screen.h"
#include "../include/globals.h"

void enter_alternate_screen() {
    write(STDOUT_FILENO, "\033[?1049h", 8); // Enable alternate screen 
    fflush(stdout); 
}

void exit_alternate_screen() {
    write(STDOUT_FILENO, "\033[?1049l", 8); // Return to normal screen 
    fflush(stdout); 
}
// Setting up a terminal to read keys in real time 
struct termios orig_termios; 

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios); // Restore terminal settings 
} 

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); raw.c_cc[VMIN] = 1;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &raw); // Apply raw mode
}

void print_centered(const char *text, int row, const char *fg_color) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int cols = w.ws_col;

    int len = strlen(text);
    int col = (cols - len) / 2;

    int fg, bg;

    if (strcmp(fg_color, "black") == 0) {
        fg = 30;
        bg = 47;
    } else {
        fg = 37;
        bg = 40;
    }

    printf("\033[%d;%dH\033[%d;%dm%s\033[0m", row, col + 1, fg, bg, text);
    fflush(stdout);
}

// how to use it: print_centered("Hello worls", 3, "white");

int get_last_row() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

void draw_screen(int cursor_row, int cursor_col, int row_offset) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int rows = w.ws_row;
    int cols = w.ws_col;

    // Clear screen
    write(STDOUT_FILENO, "\033[H\033[J", 6);

    // File name
    print_centered(filename, 1, "black");

    // Text
    int visible_rows = rows - 2;
    for (int i = 0; i < visible_rows; i++) {
        int line_index = row_offset + i;
        if (line_index >= num_lines) break;

        printf("\033[%d;1H%s", i + 2, lines[line_index]);
    }

    // Hotkeys
    print_centered("Ctrl+Q=Quit  Ctrl+S=Save  Ctrl+X=Save & Exit",
                   rows,
                   "black");

    // Cursor
    printf("\033[%d;%dH", cursor_row - row_offset + 2, cursor_col + 1);

    fflush(stdout);
}
