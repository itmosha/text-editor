#include "highlighting.h"

void editor_update_syntax(erow* row) {
    row->highlight = realloc(row->highlight, row->rsize);
    memset(row->highlight, HL_NORMAL, row->rsize);

    for (int i = 0; i < row->rsize; i++) {
        if (isdigit(row->render[i])) {
            row->highlight[i] = HL_NUMBER;
        }
    }
}

int editor_syntax_to_color(int highlight) {
    switch (highlight) {
        case HL_NUMBER:
            return 31;
        default:
            return 37;
    }
}
