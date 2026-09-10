#include "./mlux_terminal.hpp"
#include "../common/definitions.hpp"
#include <cstdlib>
#include <pty.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace mlux {

Pane &MLUX::create_new_pane() {
    Pane &pane = panes.emplace_back();
    pane.id = next_pane_id++;

    struct winsize ws {};
    ws.ws_row = static_cast<unsigned short>(pane.size.height);
    ws.ws_col = static_cast<unsigned short>(pane.size.width);
    int master = -1;
    pid_t pid = forkpty(&master, nullptr, nullptr, &ws);
    if (pid == 0) { // child
        const char *shell = std::getenv("SHELL");
        if (shell == nullptr)
            shell = "/bin/sh";
        execl(shell, shell, "-l", nullptr);
        _exit(COULD_NOT_EXEC);
    }
    pane.shell_pid = pid;
    pane.pty_master = master;
    return pane;
}

void MLUX::handle_input() {}

void MLUX::main_loop() {
    while (running) {
        // ------------------------------------------------------
        // WAIT + POLL FOR EVENTS
        // ------------------------------------------------------------

        handle_input();
    }
}
} // namespace mlux
