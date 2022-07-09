#ifndef TEXT_EDITOR_SYSTEM_FUNCTIONS_H
#define TEXT_EDITOR_SYSTEM_FUNCTIONS_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "stdlib.h"
#include "stdio.h"
#include "termios.h"
#include "unistd.h"
#include "ctype.h"
#include "errno.h"
#include "string.h"
#include "stdbool.h"
#include "time.h"
#include "stdarg.h"
#include "sys/ioctl.h"
#include "sys/types.h"

#include "input_output.h"

// mapping a key to corresponding Ctrl+key command
#define CTRL_KEY(k) ((k) & 0x1f)
#define APPEND_BUFFER_INIT {NULL, 0}
#define EDITOR_VERSION "0.0.1"
#define TAB_STOP 8

typedef struct erow {
    int size;
    int rsize;
    char* chars;
    char* render;
} erow;

struct editor_config {
    int cx, cy;
    int rx;
    int screenrows;
    int screencols;
    int num_rows;
    int rowoff;
    int coloff;
    char* filename;
    char status_message[80];
    time_t status_message_time;
    erow *row;
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
    ARROW_RIGHT = 1003,
    PAGE_UP = 1004,
    PAGE_DOWN = 1005,
    HOME_KEY = 1006,
    END_KEY = 1007,
    DEL_KEY = 1008
};

void enable_raw_mode();
void disable_raw_mode();

int get_window_size(int* rows, int* cols);
int get_cursor_position(int* rows, int* cols);

void ab_append(struct append_buffer* ab, const char* s, int len);
void ab_free(struct append_buffer* ab);
void editor_append_row(char* s, size_t len);
void editor_update_row(erow* row);

void editor_init();
void editor_open(char* filename);

void editor_execute_keypress();
void editor_move_cursor(int key);
void editor_scroll();

void editor_set_status_bar_message(const char* format, ...);
int editor_row_cx_to_rx(erow* row, int cx);
void kill(const char* error_message);

#endif //TEXT_EDITOR_SYSTEM_FUNCTIONS_H
