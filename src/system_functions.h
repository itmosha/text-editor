#ifndef TEXT_EDITOR_SYSTEM_FUNCTIONS_H
#define TEXT_EDITOR_SYSTEM_FUNCTIONS_H

#include "stdlib.h"
#include "stdio.h"
#include "termios.h"
#include "unistd.h"
#include "ctype.h"
#include "errno.h"

struct termios orig_termios;

void enable_raw_mode();
void disable_raw_mode();

void kill(const char* error_message);

#endif //TEXT_EDITOR_SYSTEM_FUNCTIONS_H
