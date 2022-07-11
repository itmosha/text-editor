#include "input_output.h"

int editor_read_key() {
    ssize_t read_;
    char c;

    while ((read_ = read(STDIN_FILENO, &c, 1)) != 1) {
        if (read_ == -1 && errno != EAGAIN)
            kill("Unable to read input in editor_read_key() function");
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
    editor_draw_status_bar(&ab);
    editor_draw_message_bar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
    ab_append(&ab, buf, strlen(buf));

    ab_append(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

void editor_draw_rows(append_buffer* ab) {
    for (int y = 0; y < E.screenrows; y++) {
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
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0) len = 0;
            if (len > E.screencols) len = E.screencols;
            char* c = &E.row[filerow].render[E.coloff];
            unsigned char* highlight = &E.row[filerow].highlight[E.coloff];
            int current_color = -1;

            for (int j = 0; j < len; j++) {
                if (highlight[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        ab_append(ab, "\x1b[39m", 5);
                        current_color = -1;
                    }
                    ab_append(ab, &c[j], 1);
                } else {
                    int color = editor_syntax_to_color(highlight[j]);
                    if (color != current_color) {
                        current_color = color;
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
                        ab_append(ab, buf, clen);
                    }
                    ab_append(ab, &c[j], 1);
                }
            }
            ab_append(ab, "\x1b[39m", 5);
        }
        ab_append(ab, "\x1b[K", 3);
        ab_append(ab, "\r\n", 2);
    }
}

void editor_draw_message_bar(append_buffer *ab) {
    ab_append(ab, "\x1b[K", 3);
    int message_len = strlen(E.status_message);

    if (message_len > E.screencols) message_len = E.screencols;

    if (message_len && time(NULL) - E.status_message_time < 5)
        ab_append(ab, E.status_message, message_len);
}

void editor_draw_status_bar(append_buffer *ab) {
    ab_append(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];

    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", E.filename ? E.filename : "[NO NAME]", E.num_rows, E.unsaved ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.num_rows);

    if (len > E.screencols)
        len = E.screencols;
    ab_append(ab, status, len);

    while (len < E.screencols) {
        if (E.screencols - len == rlen) {
            ab_append(ab, rstatus, rlen);
            break;
        } else {
            ab_append(ab, " ", 1);
            len++;
        }
    }
    ab_append(ab, "\x1b[m", 3);
    ab_append(ab, "\r\n", 2);
}

void editor_row_insert_char(erow* row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size;

    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    editor_update_row(row);

    E.unsaved++;
}

void editor_insert_char(char c) {
    if (E.cy == E.num_rows)
        editor_insert_row(E.num_rows, "", 0);

    editor_row_insert_char(&E.row[E.cy], E.cx, c);
    E.cx++;
}

char* editor_prompt(char* prompt, void (*callback)(char*, int)) {
    size_t bufsize = 128;
    char* buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (true) {
        editor_set_status_bar_message(prompt, buf);
        editor_refresh_screen();

        int c = editor_read_key();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
            if (buflen != 0) buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            editor_set_status_bar_message("");
            if (callback) callback(buf, c);
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                editor_set_status_bar_message("");
                if (callback) callback(buf, c);
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }
        if (callback) callback(buf, c);
    }
}
