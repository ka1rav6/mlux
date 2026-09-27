#include "pane.hpp"
#include <pty.h>
#include <sys/ioctl.h>

namespace mlux {

Pane::Pane(const Size &size, const Position &pos, int32_t uid) {
    this->size = std::move(size);
    this->pos = std::move(pos);
    this->uid = uid;
    this->is_focused = true;
    this->is_dead = false;
    this->pty = new PTY(size);
}
PTY::PTY(const Size &size) {
    // calculate windowsize
    struct winsize windowsize;
    windowsize.ws_col = size.columns;
    windowsize.ws_row = size.rows;
    // use getenv() to get which shell is being run
    auto user_shell = std::getenv("SHELL");
    // TODO: remove once shell is used
    std::cout
        << "Temp printing of user shell to remove unused variable warnings "
        << user_shell << std::endl;
    // register signal handler

    // forkpty() and
    // store the pty_master
    this->shell_pid = forkpty(&this->pty_master, NULL, NULL, &windowsize);

    if (this->shell_pid == 0) {
        // child process
        // runs until the signal handler calls _exit();
        // exec gets called in run_shell()
        run_shell(); // has an always running while loop
    } else {
        // parent process
    }

    // use exec to launch shell
}
} // namespace mlux
