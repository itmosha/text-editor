#include "highlighting.h"

char* C_highlight_extensions[] = {".c", ".h", ".cpp", NULL};
char* C_highlight_keywords[] = {"switch", "if", "while", "for", "break", "continue", "return", "else",
                                "struct", "union", "typedef", "static", "enum", "class", "case",
                                "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", NULL};

struct editor_syntax highlight_database[] = {
        {
                "c",
                C_highlight_extensions,
                C_highlight_keywords,
                "//", "/*", "*/",
                HIGHLIGHT_NUMBERS | HIGHLIGHT_STRINGS,
                },
};

#define HIGHLIGHT_DATABASE_ENTRIES (sizeof(highlight_database) / sizeof(highlight_database[0]))

//--------------------------------------------------------------------------------------------


void editor_update_syntax(erow* row) {
    row->highlight = realloc(row->highlight, row->rsize);
    memset(row->highlight, HL_NORMAL, row->rsize);
    if (E.syntax == NULL) return;

    char **keywords = E.syntax->keywords;

    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    int scs_len = scs ? strlen(scs) : 0;
    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int prev_separator = 1;
    int in_string = 0;
    int in_comment = (row->index > 0 && E.row[row->index - 1].highlight_open_comment);

    int i = 0;
    while (i < row->rsize) {
        char c = row->render[i];
        unsigned char prev_highlight = (i > 0) ? row->highlight[i - 1] : HL_NORMAL;

        if (scs_len && !in_string && !in_comment) {
            if (!strncmp(&row->render[i], scs, scs_len)) {
                memset(&row->highlight[i], HL_COMMENT, row->rsize - i);
                break;
            }
        }

        if (mcs_len && mce_len && !in_string) {
            if (in_comment) {
                row->highlight[i] = HL_MLCOMMENT;
                if (!strncmp(&row->render[i], mce, mce_len)) {
                    memset(&row->highlight[i], HL_MLCOMMENT, mce_len);
                    i += mce_len;
                    in_comment = 0;
                    prev_separator = 1;
                    continue;
                } else {
                    i++;
                    continue;
                }
            } else if (!strncmp(&row->render[i], mcs, mcs_len)) {
                memset(&row->highlight[i], HL_MLCOMMENT, mcs_len);
                i += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        if (E.syntax->flags & HIGHLIGHT_STRINGS) {
            if (in_string) {
                row->highlight[i] = HL_STRING;

                if (c == '\\' && i + 1 < row->rsize) {
                    row->highlight[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (c == in_string) in_string = 0;
                i++;
                prev_separator = 1;
                continue;
            } else {
                if (c == '"' || c == '\'') {
                    in_string = c;
                    row->highlight[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if (E.syntax->flags & HIGHLIGHT_NUMBERS) {
            if ((isdigit(c) && (prev_separator || prev_highlight == HL_NUMBER)) ||
                (c == '.' && prev_highlight == HL_NUMBER)) {
                row->highlight[i] = HL_NUMBER;
                i++;
                prev_separator = 0;
                continue;
            }
        }

        if (prev_separator) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int keyword_len = strlen(keywords[j]);
                int kw2 = keywords[j][keyword_len - 1] == '|';
                if (kw2) keyword_len--;

                if (!strncmp(&row->render[i], keywords[j], keyword_len) && is_separator(row->render[i + keyword_len])) {
                    memset(&row->highlight[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, keyword_len);
                    i += keyword_len;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_separator = 0;
                continue;
            }
        }
        prev_separator = is_separator(c);
        i++;
    }
    int changed = (row->highlight_open_comment != in_comment);
    row->highlight_open_comment = in_comment;
    if (changed && row->index + 1 < E.num_rows)
        editor_update_syntax(&E.row[row->index + 1]);
}

int editor_syntax_to_color(int highlight) {
    switch (highlight) {
        case HL_STRING:
            return 35;
        case HL_MLCOMMENT:
        case HL_COMMENT:
            return 36;
        case HL_KEYWORD1:
            return 33;
        case HL_KEYWORD2:
            return 32;
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
