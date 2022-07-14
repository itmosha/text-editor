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
#define HIGHLIGHT_STRINGS (1 << 1)

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
    int index;
    int size;
    int rsize;
    char* chars;
    char* render;
    unsigned char* highlight;
    int highlight_open_comment;
} erow;

typedef struct append_buffer {
    char* b;
    size_t len;
} append_buffer;

struct editor_syntax {
    char* filetype;
    char** filematch;
    char** keywords;
    char* singleline_comment_start;
    char* multiline_comment_start;
    char* multiline_comment_end;
    int flags;
};

struct editor_config {
    int mode;
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

enum editor_mode {
    DEFAULT = 1,
    INSERT,
    SEARCH
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
    DEL_KEY,
    EXIT_KEY,
    SAVE_KEY,
    SEARCH_KEY,
    INSERT_KEY
};

enum editor_highlight {
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};
#endif //TEXT_EDITOR_TYPES_H
