#include "./terminal.hpp"
#include <unistd.h>

namespace mlux {

Terminal::~Terminal() { disable_raw_mode(); }

bool Terminal::enable_raw_mode() {
    if (raw_enabled || !isatty(STDIN_FILENO)) {
        return false;
    }
    if (tcgetattr(STDIN_FILENO, &original) == -1)
        return false;
    struct termios raw = original;
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 0;  // read() returns as soon as any byte is available
    raw.c_cc[VTIME] = 0; // no timeout — poll() does the waiting

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        return false;
    raw_enabled = true;
    return true;
}

bool Terminal::disable_raw_mode() {
    if (!raw_enabled)
        return false;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
    raw_enabled = false;
    return true;
}
} // namespace mlux
