#ifndef TEXT_EDITOR_HIGHLIGHTING_H
#define TEXT_EDITOR_HIGHLIGHTING_H

#include "types.h"
#include "system_functions.h"

void editor_update_syntax(erow* row);
int editor_syntax_to_color(int highlight);
void editor_select_syntax_highlight();

int is_separator(int c);

#endif //TEXT_EDITOR_HIGHLIGHTING_H
