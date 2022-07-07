#include "system_functions.h"

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        kill("tcgetattr");
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
        kill("tcsetattr");
    }
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        kill("tcsetattr");
}

void kill(const char* error_message) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(error_message);
    exit(1);
}

void editor_execute_keypress() {
    int c = editor_read_key();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);

            exit(0);

        case HOME_KEY:
            E.cx = 0;
            break;

        case END_KEY:
            E.cx = E.screencols - 1;

        case PAGE_UP:
        case PAGE_DOWN: {
            int times = E.screenrows;
            while (times--) {
                editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
        } break;

        case ARROW_LEFT:
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            editor_move_cursor(c);
            break;
        default:
            break;
    }
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
    E.num_rows = 0;
    E.rowoff = 0;
    E.row = NULL;

   if (get_window_size(&E.screenrows, &E.screencols) == -1)
        kill("Unable to get window size");
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

void ab_append(struct append_buffer* ab, const char* s, int len) {
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
    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) E.cx--;
            break;
        case ARROW_DOWN:
            if (E.cy != 0) E.cy--;
            break;
        case ARROW_UP:
            if (E.cy != E.screenrows - 1) E.cy++;
            break;
        case ARROW_RIGHT:
            if (E.cx != E.screencols - 1) E.cx++;
            break;
        default:
            break;
    }
}

void editor_open(char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) kill("Unable to open file");

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
}

void editor_append_row(char* s, size_t len) {
    E.row = realloc(E.row, sizeof(erow) * (E.num_rows + 1));

    int at = E.num_rows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.num_rows += 1;
}
