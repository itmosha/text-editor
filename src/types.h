#ifndef TEXT_EDITOR_TYPES_H
#define TEXT_EDITOR_TYPES_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "time.h"
#include "stdlib.h"
#include "stdio.h"
#include "termios.h"
#include "sys/types.h"
#include "ctype.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "stdbool.h"
#include "stdarg.h"
#include "fcntl.h"
#include "sys/ioctl.h"

typedef struct erow {
    int size;
    int rsize;
    char* chars;
    char* render;
    unsigned char* highlight;
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
    ARROW_UP,
    ARROW_DOWN,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DEL_KEY
};

enum editor_highlight {
    HL_NORMAL = 0,
    HL_NUMBER
};
#endif //TEXT_EDITOR_TYPES_H
