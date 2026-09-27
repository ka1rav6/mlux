#ifndef MLUX_TERMINAL
#define MLUX_TERMINAL

#include <termios.h>

namespace mlux {

class Terminal {
  public:
    Terminal() = default;
    ~Terminal();
    bool enable_raw_mode();
    bool disable_raw_mode();
    // handles all the signals that the user sends directly so it can be parsed
    void init_signal_handling();
    void signal_handler(int signum);

  private:
    // saves the original settings of the terminal so that it can be restored
    // later
    struct termios original {};
    bool raw_enabled = false;
};

} // namespace mlux

#endif
