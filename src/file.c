#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include "../include/file.h"
#include "../include/globals.h"

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

        if (lines[i][strlen(lines[i]) - 1] != '\n') {
            fputc('\n', f);
        }
    }

    fclose(f);
}

void ensure_nonempty() {
    if(num_lines == 0) {
        lines[0][0] = '\0';
        num_lines = 1;
    }
}