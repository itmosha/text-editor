#include "src/system_functions.h"

int main(int argc, char* argv[]) {
    enable_raw_mode();
    editor_init();

    if (argc >= 2)
        editor_open(argv[1]);

    editor_set_status_bar_message("HELP: CTRL+q = quit | Ctrl+s = save | Ctrl+f = find");
    while (true) {
        editor_refresh_screen();
        editor_execute_keypress();
    }
}
