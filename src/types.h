#ifndef TEXT_EDITOR_TYPES_H
#define TEXT_EDITOR_TYPES_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

// mapping a key to corresponding Ctrl+key command
#define CTRL_KEY(k) ((k) & 0x1f)
#define APPEND_BUFFER_INIT {NULL, 0}
#define EDITOR_VERSION "0.0.1"
#define TAB_STOP 4
#define QUIT_TIMES 2
#define HIGHLIGHT_NUMBERS (1 << 0)

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

struct editor_syntax {
    char* filetype;
    char** filematch;
    int flags;
};

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
    struct editor_syntax* syntax;
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
    HL_NUMBER,
    HL_MATCH
};
#endif //TEXT_EDITOR_TYPES_H
