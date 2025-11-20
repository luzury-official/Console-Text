#ifndef SCREEN_H
#define SCREEN_H

void enter_alternate_screen();
void exit_alternate_screen();
void disable_raw_mode();
void enable_raw_mode();
void print_centered(const char *text, int row, const char *fg_color);
int get_last_row();
void draw_screen(int cursor_row, int cursor_col, int row_offset);

#endif