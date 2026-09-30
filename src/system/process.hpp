#pragma once
#include <sys/types.h>

#include <cstddef>

#include "common/types.hpp"

namespace mlux {

// a wrapper just to keep track of what process of the pty has what pid and is
// assigned to which pane
struct Process {
    pid_t pid = 0;
    PaneId pane_id = 0;
};

} // namespace mlux
