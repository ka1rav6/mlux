#pragma once
#include <sys/types.h>

#include "common/types.hpp"

namespace mlux {

// a wrapper over the psuedoterminal that has all the data of each pty session
// stored
class PTY {
  public:
    PTY(const CellSize &size);
    ~PTY();

  private:
    pid_t shell_pid = 0;
    int pty_master = -1;
    void launch_shell();
    void run_shell();
};

// the class that stores all the information about each pane that is being
// rendered
class Pane {
  public:
    CellSize size;
    Position pos;
    PaneId uid = 0;
    PTY *pty;
    bool is_focused = false;
    bool is_dead = false;
    Pane(const CellSize &size, const Position &pos, PaneId uid);
    ~Pane() = default;
    Pane(const Pane &) = delete; // not allowing pane to be copiable
};
} // namespace mlux
