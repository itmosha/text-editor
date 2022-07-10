#ifndef TEXT_EDITOR_INPUT_OUTPUT_H
#define TEXT_EDITOR_INPUT_OUTPUT_H

#include "system_functions.h"
#include "types.h"

void editor_refresh_screen();
void editor_draw_rows(append_buffer* ab);
void editor_draw_status_bar(append_buffer *ab);
void editor_draw_message_bar(append_buffer *ab);

int editor_read_key();
void editor_row_insert_char(erow* row, int at, int c);
void editor_insert_char(char c);

char* editor_prompt(char* prompt);

#endif //TEXT_EDITOR_INPUT_OUTPUT_H
