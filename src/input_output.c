#include "input_output.h"

int editor_read_key() {
    int read_;
    char c;

    while ((read_ = read(STDIN_FILENO, &c, 1)) != 1) {
        if (read_ == -1 && errno != EAGAIN)
            kill("Unable to read input");
    }

    if (c == '\x1b') {
        // READ AN ESCAPE SEQUENCE
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }
        return '\x1b';
    } else {
        // FOR REGULAR KEYS

        switch (c) {
            case 'h': return ARROW_LEFT;
            case 'j': return ARROW_DOWN;
            case 'k': return ARROW_UP;
            case 'l': return ARROW_RIGHT;
        }
    }
}

void editor_refresh_screen() {
    editor_scroll();
    struct append_buffer ab = APPEND_BUFFER_INIT;

    ab_append(&ab, "\x1b[?25l", 6);
    ab_append(&ab, "\x1b[H", 3);
    editor_draw_rows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, E.cx + 1);
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    ab_append(&ab, buf, strlen(buf));

    ab_append(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

void editor_draw_rows(struct append_buffer* ab) {
    for (size_t y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;
        if (filerow >= E.num_rows) {
            if (E.num_rows == 0 && y == E.screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "\x1b[1mText editor -- version %s\x1b[0m",
                                          EDITOR_VERSION);
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    ab_append(ab, "~", 1);
                    padding--;
                }
                while (padding--) ab_append(ab, " ", 1);
                if (welcomelen > E.screencols) welcomelen = E.screencols;
                ab_append(ab, welcome, welcomelen);
            } else {
                ab_append(ab, "~", 1);
            }
        } else {
            int len = E.row[filerow].size;
            if (len > E.screencols) len = E.screencols;
            ab_append(ab, E.row[filerow].chars, len);
        }

        ab_append(ab, "\x1b[K", 3);
        if (y < E.screenrows - 1)
            ab_append(ab, "\r\n", 2);
    }
}