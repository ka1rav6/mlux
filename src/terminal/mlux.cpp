#include "./mlux.hpp"

namespace mlux {

Pane &MLUX::create_new_pane() {}
// 1. create PTY

// 2. fork

// 3. child:
//      setsid()
//      TIOCSCTTY
//      dup2(slave, 0)
//      dup2(slave, 1)
//      dup2(slave, 2)
//      exec($SHELL)

// 4. parent:
//      close(slave)
//      keep master

void MLUX::main_loop() {
    while (running) {
        // ------------------------------------------------------
        // WAIT + POLL FOR EVENTS
        // ------------------------------------------------------------

        handle_input();
    }
}
} // namespace mlux
