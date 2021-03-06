#ifndef TEXT_EDITOR_SYSTEM_FUNCTIONS_H
#define TEXT_EDITOR_SYSTEM_FUNCTIONS_H

#include "types.h"
#include "input_output.h"
#include "highlighting.h"

struct editor_config E;

void enable_raw_mode();
void disable_raw_mode();

int get_window_size(int* rows, int* cols);
int get_cursor_position(int* rows, int* cols);

void ab_append(append_buffer* ab, const char* s, int len);
void ab_free(append_buffer* ab);
void editor_insert_row(int at, char* s, size_t len);
void editor_update_row(erow* row);
void editor_free_row(erow* row);
void editor_delete_row(int at);
void editor_row_append_string(erow* row, char* s, size_t len);
void editor_insert_new_line();

void editor_init();
void editor_open(char* filename);

void editor_row_delete_char(erow* row, int at);
void editor_delete_char();
void editor_execute_keypress();
void editor_move_cursor(int key);
void editor_scroll();

void editor_set_status_bar_message(const char* format, ...);
int editor_row_cx_to_rx(erow* row, int cx);
int editor_row_rx_to_cx(erow* row, int rx);
char* editor_rows_to_string(int* buflen);

void editor_find();
void editor_find_callback(char* query, int key);

void editor_save();
void kill(const char* error_message);

#endif //TEXT_EDITOR_SYSTEM_FUNCTIONS_H
