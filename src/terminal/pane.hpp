#ifndef MLUX_TERMINAL_PANE_HPP
#define MLUX_TERMINAL_PANE_HPP

#include <sys/types.h>

#include "../common/types.hpp"

namespace mlux {

struct Pane {
  Size size;
  Position pos;
  std::int32_t id = 0;
  pid_t shell_pid = 0;
  std::int32_t pty_master = 0;
  bool is_focused = false;
  bool is_dead = false;
};

} // namespace mlux

#endif // MLUX_TERMINAL_PANE_HPP
