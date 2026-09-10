#ifndef MLUX_MLUX
#define MLUX_MLUX

#include "./pane.hpp"
#include <vector>

namespace mlux {

class MLUX {
    int focused_pane_id;
    std::vector<Pane> panes;
    void main_loop();
    void handle_input();

  private:
    Pane &create_new_pane();
    bool running = false;
    int next_pane_id = 0;
};

} // namespace mlux

#endif // MLUX_MLUX
