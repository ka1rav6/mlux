#ifndef MLUX_SYSTEM_PROCESS_HPP
#define MLUX_SYSTEM_PROCESS_HPP

#include <sys/types.h>

namespace mlux {

// a wrapper just to keep track of what process of the pty has what pid and is
// assigned to which pane
struct Process {
    pid_t pid = 0;
    size_t pane_id = 0;
};

} // namespace mlux

#endif // MLUX_SYSTEM_PROCESS_HPP
