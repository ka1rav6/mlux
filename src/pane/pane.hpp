#ifndef MLUX_TERMINAL_PANE_HPP
#define MLUX_TERMINAL_PANE_HPP

#include <sys/types.h>

#include "../common/types.hpp"

namespace mlux {

class PTY {
  public:
    pid_t shell_pid;
    int pty_master;
    int pty_slave;
    int fd[2]; // to write/read from the bash
    PTY(const Size &size);
    ~PTY();

  private:
    void launch_shell();
    void run_shell();
    void init_signal_handling();
    void signal_handler(int signum);
};

class Pane {
    Size size;
    Position pos;
    int32_t uid = 0;
    PTY *pty;
    bool is_focused = false;
    bool is_dead = false;
    Pane(const Size &size, const Position &pos, int32_t uid);
    ~Pane();
};
} // namespace mlux

#endif // MLUX_TERMINAL_PANE_HPP
