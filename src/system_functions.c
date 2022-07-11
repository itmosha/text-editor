#include "system_functions.h"

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        kill("tcgetattr error in enable_raw_mode() function");
    atexit(disable_raw_mode);

    struct termios raw = E.orig_termios;
    // ECHO to disable echoing the input
    // ICANON to read input byte by byte
    // IEXTEN to disable Ctrl+V (multiple input)
    // ISIG to disable Ctrl+C and Ctrl+Z (terminating + suspending)
    // BRKINT to send SIGINT in case of a break condition
    // ICRNL to disable auto translating \r into \n
    // INPCK to enable parity checking
    // ISTRIP to set every 8th bit of every byte to zero
    // IXON to disable Ctrl+S and Ctrl+Q (input stopping)
    // OPOST to disable auto translating \n into \r\n
    // CS8 to set character size to 8 bits
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        kill("tcsetattr error in enable_raw_mode() function");
    }
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        kill("tcsetattr error in disable_raw_mode() function");
}

void kill(const char* error_message) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(error_message);
    exit(1);
}

void editor_execute_keypress() {
    static int quit_times = QUIT_TIMES;
    int c = editor_read_key();

    switch (c) {
        case '\r':
            editor_insert_new_line();
            break;

        case CTRL_KEY('q'):
            if (E.unsaved && quit_times > 0) {
                editor_set_status_bar_message("WARNING! File nas unsaved changes. "
                                              "Press Ctrl+q %d more times to quit", quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);


        case CTRL_KEY('s'):
            editor_save();
            break;

        case HOME_KEY:
            E.cx = 0;
            break;

        case END_KEY:
            if (E.cy < E.num_rows)
                E.cx = E.row[E.cy].size;
            break;

        case CTRL_KEY('f'):
            editor_find();
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY) editor_move_cursor(ARROW_RIGHT);
            editor_delete_char();
            break;

        case PAGE_UP:
        case PAGE_DOWN: {
            if (c == PAGE_UP)
                E.cy = E.rowoff;
            else {
                E.cy = E.rowoff + E.screenrows - 1;
                if (E.cy > E.num_rows) E.cy = E.num_rows;
            }
            int times = E.screenrows;
            while (times--)
                editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        } break;

        case ARROW_LEFT:
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            editor_move_cursor(c);
            break;

        case CTRL_KEY('l'):
        case '\x1b':
            break;

        default:
            editor_insert_char(c);
            break;
    }
    quit_times = QUIT_TIMES;
}

int get_window_size(int* rows, int* cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {

        // move to the right-bottom corner to find out the size of the window
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return get_cursor_position(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void editor_init() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.num_rows = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.row = NULL;
    E.filename = NULL;
    E.status_message_time = 0;
    E.status_message[0] = '\0';
    E.unsaved = 0;

   if (get_window_size(&E.screenrows, &E.screencols) == -1)
        kill("Unable to get window size in editor_init() function");
   E.screenrows -= 2; // for status bar
}

int get_cursor_position(int* rows, int* cols) {
    char buf[32];
    size_t i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

void ab_append(append_buffer* ab, const char* s, int len) {
    char* new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void ab_free(struct append_buffer* ab) {
    free(ab->b);
}

void editor_move_cursor(int key) {
    erow* row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];

    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) E.cx--;
            else if (E.cy > 0) {
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) E.cx++;
            else if (row && E.cx == row->size) {
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0) E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy < E.num_rows) E.cy++;
            break;
        default:
            break;
    }

    row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen)
        E.cx = rowlen;
}

void editor_open(char* filename) {
    free(E.filename);
    E.filename = strdup(filename);

    FILE* file = fopen(filename, "r");
    if (!file) kill("Unable to open file in editor_open(char*) function");

    char* line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&line, &linecap, file)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        editor_insert_row(E.num_rows, line, linelen);
    }

    free(line);
    fclose(file);
    E.unsaved = 0;
}

void editor_insert_row(int at, char* s, size_t len) {
    if (at < 0 || at > E.num_rows) return;
    E.row = realloc(E.row, sizeof(erow) * (E.num_rows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.num_rows - at));

    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editor_update_row(&E.row[at]);

    E.num_rows++;
    E.unsaved++;
}

void editor_scroll() {
    E.rx = 0;
    if (E.cy < E.num_rows)
        E.rx = editor_row_cx_to_rx(&E.row[E.cy], E.cx);

    if (E.cy < E.rowoff)
        E.rowoff = E.cy;
    if (E.cy >= E.rowoff + E.screenrows)
        E.rowoff = E.cy - E.screenrows + 1;
    if (E.rx < E.coloff)
        E.coloff = E.rx;
    if (E.rx >= E.coloff + E.screencols)
        E.coloff = E.rx - E.screencols + 1;
}

void editor_update_row(erow* row) {
    int tabs = 0;
    for (int i = 0; i < row->size; ++i)
        if (row->chars[i] == '\t') tabs++;

    free(row->render);
    row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

    int index = 0;
    for (int i = 0; i < row->size; ++i) {
        if (row->chars[i] == '\t') {
            row->render[index++] = ' ';
            while (index % TAB_STOP != 0) row->render[index++] = ' ';
        } else {
            row->render[index++] = row->chars[i];
        }
    }
    row->render[index] = '\0';
    row->rsize = index;
}

int editor_row_cx_to_rx(erow* row, int cx) {
    int rx = 0;
    for (int i = 0; i < cx; ++i) {
        if (row->chars[i] == '\t')
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}


void editor_set_status_bar_message(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    vsnprintf(E.status_message, sizeof(E.status_message), format, ap);
    va_end(ap);

    E.status_message_time = time(NULL);
}

char* editor_rows_to_string(int* buflen) {
    int total_len = 0;
    for (int i = 0; i < E.num_rows; i++)
        total_len += E.row[i].size + 1;
    *buflen = total_len;

    char* buf = malloc(total_len);
    char* p = buf;
    for (int i = 0; i < E.num_rows; i++) {
        memcpy(p, E.row[i].chars, E.row[i].size);
        p += E.row[i].size;
        *p = '\n';
        p++;
    }

    return buf;
}

void editor_save() {
    if (E.filename == NULL) {
        E.filename = editor_prompt("Save file as (ESC to cancel): %s", NULL);

        if (E.filename == NULL) {
            editor_set_status_bar_message("Save cancelled");
            return;
        }
    }

    int len;
    char* buf = editor_rows_to_string(&len);

    int file = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (file != -1) {
        if (ftruncate(file, len) != -1) {
            if (write(file, buf, len) == len) {
                close(file);
                free(buf);
                E.unsaved = 0;
                editor_set_status_bar_message("%d bytes written to disk", len);
                return;
            }
        }
        close(file);
    }
    free(buf);
    editor_set_status_bar_message("Cannot save! I/O error: %s", strerror(errno));
}

void editor_row_delete_char(erow* row, int at) {
    if (at < 0 || at > row->size) return;

    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editor_update_row(row);
    E.unsaved++;
}

void editor_delete_char() {
    if (E.cy == E.num_rows) return;
    if (E.cx == 0 && E.cy == 0) return;

    erow* row = &E.row[E.cy];
    if (E.cx > 0) {
        editor_row_delete_char(row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.row[E.cy - 1].size;
        editor_row_append_string(&E.row[E.cy - 1], row->chars, row->size);
        editor_delete_row(E.cy);
        E.cy--;
    }
}

void editor_free_row(erow* row) {
    free(row->render);
    free(row->chars);
}

void editor_delete_row(int at) {
    if (at < 0 || at >= E.num_rows) return;

    editor_free_row(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.num_rows - at - 1));
    E.num_rows--;
    E.unsaved++;
}

void editor_row_append_string(erow* row, char* s, size_t len) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);

    row->size += len;
    row->chars[row->size] = '\0';
    editor_update_row(row);
    E.unsaved++;
}

void editor_insert_new_line() {
    if (E.cx == 0) {
        editor_insert_row(E.cy, "", 0);
    } else {
        erow* row = &E.row[E.cy];
        editor_insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editor_update_row(row);
    }
    E.cy++;
    E.cx = 0;
}

void editor_find() {
    int save_cx = E.cx, save_cy = E.cy;
    int save_coloff = E.coloff, save_rowoff = E.rowoff;

    char* query = editor_prompt("Search (ESC to cancel): %s", editor_find_callback);
    if (query) {
        free(query);
    } else {
        E.cx = save_cx;
        E.cy = save_cy;
        E.coloff = save_coloff;
        E.rowoff = save_rowoff;
    }
}

void editor_find_callback(char* query, int key) {
    static int last_match = -1;
    static int direction = 1;

    if (key == '\r' || key == '\x1b') {
        last_match = -1;
        direction = 1;
        return;
    } else if (key == ARROW_RIGHT || key == ARROW_DOWN) {
        direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1) direction = 1;
    int current = last_match;

    for (int i = 0; i < E.num_rows; i++) {
        current += direction;
        if (current == -1) current = E.num_rows - 1;
        else if (current == E.num_rows) current = 0;

        erow* row = &E.row[current];
        char* match = strstr(row->render, query);
        if (match) {
            last_match = current;
            E.cy = current;
            E.cx = editor_row_rx_to_cx(row, match - row->render);
            E.rowoff = E.num_rows;
            break;
        }
    }
}

int editor_row_rx_to_cx(erow* row, int rx) {
    int cur_rx = 0;
    int cx;
    for (cx = 0; cx < row->size; cx++) {
        if (row->chars[cx] == '\t')
            cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
        cur_rx++;

        if (cur_rx > rx) return cx;
    }
    return cx;
}
