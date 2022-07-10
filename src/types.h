#ifndef TEXT_EDITOR_TYPES_H
#define TEXT_EDITOR_TYPES_H

#include "time.h"
#include "stdlib.h"
#include "stdio.h"
#include "termios.h"
#include "sys/types.h"
#include "ctype.h"

typedef struct erow {
    int size;
    int rsize;
    char* chars;
    char* render;
} erow;

typedef struct append_buffer {
    char* b;
    size_t len;
} append_buffer;

struct editor_config {
    int cx, cy;
    int rx;
    int screenrows;
    int screencols;
    int num_rows;
    int rowoff;
    int coloff;
    int unsaved;
    char* filename;
    char status_message[80];
    time_t status_message_time;
    erow *row;
    struct termios orig_termios;
};


enum editor_key {
    BACKSPACE = 127,
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

#endif //TEXT_EDITOR_TYPES_H
