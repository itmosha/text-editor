#include "src/system_functions.h"

int main(int argc, char** argv) {
    enable_raw_mode();
    init_editor();

    while (1) {
        editor_refresh_screen();
        editor_execute_keypress();
    }

    return 0;
}
