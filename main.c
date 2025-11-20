#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>

#define MAX_LINES 10000
#define MAX_LEN 2048

char lines[MAX_LINES][MAX_LEN];
int num_lines = 0;

char *filename;

//  |================================|
//  |              FILE              |
//  |================================|

void load_file() {
    FILE *f = fopen(filename, "r");
    if (!f) return;

    while (fgets(lines[num_lines], MAX_LEN, f)) {
        num_lines++;
    }

    fclose(f);
}

void save_file() {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Error saving file");
        return;
    }

    for (int i = 0; i < num_lines; i++) {
        fputs(lines[i], f);

        // если строка НЕ заканчивается \n — добавим
        if (lines[i][strlen(lines[i]) - 1] != '\n') {
            fputc('\n', f);
        }
    }

    fclose(f);
}

//  |================================|
//  |             SCREEN             |
//  |================================|

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


int main(int argc, char *argv[]){
    if (argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }

    int cursor_row = 0;
    int cursor_col = 0;
    int row_offset = 0;
    
    filename = argv[1];
    load_file(); 
    
    enter_alternate_screen();
    enable_raw_mode();
    
    char key;
    while (1) {
        draw_screen(cursor_row, cursor_col, row_offset);

        read(STDIN_FILENO, &key, 1); // Read single key
        
        // Hot Keys
        if (key == 17) break;   // Ctrl+Q
        if (key == 19) save_file();   // Ctrl+S
        if (key == 24) {save_file(); break;}   // Ctrl+X

        // Moving
        if (key == 27) { // ESC
            char seq[3] = {0};
            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;

            if (seq[0] == '[') {
                // Arrow keys: A=up, B=down, C=right, D=left
                if (seq[1] >= 'A' && seq[1] <= 'D') {
                    struct winsize w;
                    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                    int visible_rows = w.ws_row - 2; // top + bottom panels

                    switch (seq[1]) {
                        case 'A': // up
                            if (cursor_row > 0) {
                                cursor_row--;
                                if (cursor_col > strlen(lines[cursor_row]))
                                    cursor_col = strlen(lines[cursor_row]);
                                if (cursor_row < row_offset)
                                    row_offset = cursor_row;
                            }
                            break;

                        case 'B': // down
                            if (cursor_row < num_lines - 1) {
                                cursor_row++;
                                if (cursor_col > strlen(lines[cursor_row]))
                                    cursor_col = strlen(lines[cursor_row]);
                                if (cursor_row >= row_offset + visible_rows)
                                    row_offset = cursor_row - visible_rows + 1;
                            }
                            break;

                        case 'C': // right
                            if (cursor_col < strlen(lines[cursor_row])) {
                                cursor_col++;
                            } else if (cursor_row < num_lines - 1) {
                                cursor_row++;
                                cursor_col = 0;
                                if (cursor_row >= row_offset + visible_rows)
                                    row_offset = cursor_row - visible_rows + 1;
                            }
                            break;

                        case 'D': // left
                            if (cursor_col > 0) {
                                cursor_col--;
                            } else if (cursor_row > 0) {
                                cursor_row--;
                                cursor_col = strlen(lines[cursor_row]);
                                if (cursor_row < row_offset)
                                    row_offset = cursor_row;
                            }
                            break;
                    }
                } 
                // Delete key: ESC [ 3 ~
                else if (seq[1] == '3') {
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) continue;
                    if (seq[2] == '~') {
                        int len = strlen(lines[cursor_row]);
                        if (cursor_col < len) {
                            // delete character under cursor
                            memmove(&lines[cursor_row][cursor_col],
                                    &lines[cursor_row][cursor_col + 1],
                                    len - cursor_col);
                        } else if (cursor_row < num_lines - 1) {
                            // merge with next line
                            strcat(lines[cursor_row], lines[cursor_row + 1]);
                            for (int i = cursor_row + 1; i < num_lines - 1; i++) {
                                strcpy(lines[i], lines[i + 1]);
                            }
                            num_lines--;
                        }
                    }
                }
            }
            continue;
        }

        // Normal symbols
        if (key >= 32 && key <= 126) {
            int len = strlen(lines[cursor_row]);

            if (cursor_col < len) {
                // Move string to the right
                memmove(&lines[cursor_row][cursor_col + 1],
                        &lines[cursor_row][cursor_col],
                        len - cursor_col + 1);
            }

            // Paste symbol
            lines[cursor_row][cursor_col] = key;
            cursor_col++;
        }

        // Backspace
        if (key == 127) {
            if (cursor_col > 0) {
                int len = strlen(lines[cursor_row]);
                memmove(&lines[cursor_row][cursor_col - 1],
                        &lines[cursor_row][cursor_col],
                        len - cursor_col + 1);
                cursor_col--;
            } else if (cursor_row > 0) {
                int prev_len = strlen(lines[cursor_row - 1]);
                strcat(lines[cursor_row - 1], lines[cursor_row]);
                for (int i = cursor_row; i < num_lines - 1; i++) {
                    strcpy(lines[i], lines[i + 1]);
                }
                num_lines--;
                cursor_row--;
                cursor_col = prev_len;
            }
        }

        // Enter
        if (key == '\n') {
            // Move string down
            for (int i = num_lines; i > cursor_row + 1; i--) {
                strcpy(lines[i], lines[i - 1]);
            }
            num_lines++;

            strcpy(lines[cursor_row + 1], &lines[cursor_row][cursor_col]);
            lines[cursor_row][cursor_col] = '\0';

            cursor_row++;
            cursor_col = 0;
        }
    }
    
    exit_alternate_screen();
    
    return 0;
}
