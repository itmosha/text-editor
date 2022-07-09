#include "src/system_functions.h"

int main(int argc, char* argv[]) {
    enable_raw_mode();
    editor_init();

    if (argc >= 2)
        editor_open(argv[1]);

    while (true) {
        editor_refresh_screen();
        editor_execute_keypress();
    }
}
