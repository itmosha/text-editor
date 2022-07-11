#include "highlighting.h"

char* C_highlight_extensions[] = {".c", ".h", ".cpp", NULL};
struct editor_syntax highlight_database[] = {
        {
                "c",
                C_highlight_extensions,
                HIGHLIGHT_NUMBERS
                },
};

#define HIGHLIGHT_DATABASE_ENTRIES (sizeof(highlight_database) / sizeof(highlight_database[0]))

//--------------------------------------------------------------------------------------------


void editor_update_syntax(erow* row) {
    row->highlight = realloc(row->highlight, row->rsize);
    memset(row->highlight, HL_NORMAL, row->rsize);

    if (E.syntax == NULL) return;

    int prev_separator = 1;

    int i = 0;
    while (i < row->rsize) {
        char c = row->render[i];
        unsigned char prev_highlight = (i > 0) ? row->highlight[i - 1] : HL_NORMAL;

        if (E.syntax->flags & HIGHLIGHT_NUMBERS) {
            if ((isdigit(c) && (prev_separator || prev_highlight == HL_NUMBER)) ||
                (c == '.' && prev_highlight == HL_NUMBER)) {
                row->highlight[i] = HL_NUMBER;
                i++;
                prev_separator = 0;
                continue;
            }
        }
        prev_separator = is_separator(c);
        i++;
    }
}

int editor_syntax_to_color(int highlight) {
    switch (highlight) {
        case HL_NUMBER:
            return 31;
        case HL_MATCH:
            return 34;
        default:
            return 37;
    }
}

int is_separator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editor_select_syntax_highlight() {
    E.syntax = NULL;
    if (E.filename == NULL) return;

    char* extension = strrchr(E.filename, '.');
    for (int i = 0; i < HIGHLIGHT_DATABASE_ENTRIES; i++) {
        struct editor_syntax* s = &highlight_database[i];

        unsigned int j = 0;
        while (s->filematch[j]) {
            int is_extension = (s->filematch[j][0] == '.');

            if ((is_extension && extension && !strcmp(extension, s->filematch[j])) || (!is_extension && strstr(E.filename, s->filematch[j]))) {
                E.syntax = s;

                for (int filerow = 0; filerow < E.num_rows; filerow++)
                    editor_update_syntax(&E.row[filerow]);
                return;
            }
            j++;
        }
    }
}
