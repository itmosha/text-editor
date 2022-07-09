#ifndef TEXT_EDITOR_INPUT_OUTPUT_H
#define TEXT_EDITOR_INPUT_OUTPUT_H

#include "system_functions.h"


int editor_read_key();
void editor_refresh_screen();
void editor_draw_rows(struct append_buffer* ab);

#endif //TEXT_EDITOR_INPUT_OUTPUT_H
