#ifndef MLUX_TERMINAL_PANE_HPP
#define MLUX_TERMINAL_PANE_HPP

#include <sys/types.h>

#include "../common/types.hpp"

namespace mlux {

// a wrapper over the psuedoterminal that has all the data of each pty session
// stored
class PTY {
  public:
    pid_t shell_pid;
    int pty_master;
    int pty_slave;
    int fd[2]; // to write/read from bash
    PTY(const Size &size);
    ~PTY();

  private:
    void launch_shell();
    void run_shell();
};

// the class that stores all the information about each pane that is being
// rendered
class Pane {
    Size size;
    Position pos;
    int32_t uid = 0;
    PTY *pty;
    bool is_focused = false;
    bool is_dead = false;
    Pane(const Size &size, const Position &pos, int32_t uid);
    ~Pane() = default;
};
} // namespace mlux

#endif // MLUX_TERMINAL_PANE_HPP
