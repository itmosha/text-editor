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
            // IMPLEMENTATION HERE
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

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            // IMPLEMENTATION HERE
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
        editor_append_row(line, linelen);
    }

    free(line);
    fclose(file);
    E.unsaved = 0;
}

void editor_append_row(char* s, size_t len) {
    E.row = realloc(E.row, sizeof(erow) * (E.num_rows + 1));

    int at = E.num_rows;
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
    if (E.filename == NULL) return;

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
