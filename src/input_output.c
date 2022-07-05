#include "input_output.h"

char editor_read_key() {
    ssize_t read_;
    char c;

    while ((read_ = read(STDIN_FILENO, &c, 1)) != 1) {
        if (read_ == -1 && errno != EAGAIN)
            kill("Unable to read input");
    }
    return c;
}

void editor_refresh_screen() {
    struct append_buffer ab = APPEND_BUFFER_INIT;

    ab_append(&ab, "\x1b[?25l", 6);
    ab_append(&ab, "\x1b[H", 3);
    editor_draw_rows(&ab);

    ab_append(&ab, "\x1b[H", 3);
    ab_append(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

void editor_draw_rows(struct append_buffer* ab) {
    for (size_t y = 0; y < E.screenrows; y++) {
        if (y == E.screenrows / 3) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome), "Text editor -- version %s", EDITOR_VERSION);

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

        ab_append(ab, "\x1b[K", 3);
        if (y < E.screenrows - 1)
            ab_append(ab, "\r\n", 2);
    }
}