#ifndef TEXT_EDITOR_SYSTEM_FUNCTIONS_H
#define TEXT_EDITOR_SYSTEM_FUNCTIONS_H

#include "stdlib.h"
#include "stdio.h"
#include "termios.h"
#include "unistd.h"
#include "ctype.h"
#include "errno.h"
#include "string.h"
#include "input_output.h"
#include "sys/ioctl.h"

// mapping a key to corresponding Ctrl+key command
#define CTRL_KEY(k) ((k) & 0x1f)
#define APPEND_BUFFER_INIT {NULL, 0}

struct editor_config {
    int cx, cy;
    int screenrows;
    int screencols;
    struct termios orig_termios;
};
struct editor_config E;

struct append_buffer {
    char* b;
    size_t len;
};

enum editor_key {
    ARROW_LEFT = 1000,
    ARROW_UP = 1001,
    ARROW_DOWN = 1002,
    ARROW_RIGHT = 1003
};

void init_editor();
void enable_raw_mode();
void disable_raw_mode();
int get_window_size(int* rows, int* cols);
int get_cursor_position(int* rows, int* cols);

void ab_append(struct append_buffer* ab, const char* s, int len);
void ab_free(struct append_buffer* ab);

void editor_execute_keypress();
void editor_move_cursor(int key);

void kill(const char* error_message);

#endif //TEXT_EDITOR_SYSTEM_FUNCTIONS_H
