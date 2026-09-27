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
    void init_signal_handling();
    void signal_handler(int signum);

  private:
    struct termios original {};
    bool raw_enabled = false;
};

} // namespace mlux

#endif
