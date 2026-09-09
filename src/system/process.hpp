#ifndef MLUX_SYSTEM_PROCESS_HPP
#define MLUX_SYSTEM_PROCESS_HPP

#include <sys/types.h>

namespace mlux {

struct Process {
    pid_t pid = 0;
    std::size_t pane_id = 0;
};

} // namespace mlux

#endif // MLUX_SYSTEM_PROCESS_HPP
