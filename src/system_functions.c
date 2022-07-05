#include "system_functions.h"

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        kill("tcgetattr");
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;

    // ECHO to disable echoing the input
    // ICANON to read input byte by byte
    // IEXTEN to disable Ctrl+V (multiple input)
    // ISIG to disable Ctrl+C and Ctrl+Z (terminating + suspending)
    // BRKINT to send SIGINT in case of a break condition
    // ICRNL to disable auto translating \r into \n
    // INPCK to enable parity checking
    // ISTRIP to set every 8th bit of every byte to zero
    // IXON to disable Ctrl+S and Ctrl+Q (input stopping)
    // OPOST to disable auto translating \n into \r\n
    // CS8 to set character size to 8 bits
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        kill("tcsetattr");
    }
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        kill("tcsetattr");
}

void kill(const char* error_message) {
    perror(error_message);
    exit(1);
}
