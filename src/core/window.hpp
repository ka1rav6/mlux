#ifndef MLUX_WINDOW_H
#define MLUX_WINDOW_H

#include "definitions.hpp"
#include "pane.hpp"

#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
namespace mlux {

class Window {
  public:
    Window(WindowId id, std::string name);
    [[nodiscard]] WindowId id() const;
    const std::string &name() const;
    void rename(std::string);
    Pane &createPane();
    void destroyPane(PaneId);
    Pane *getPane(PaneId);
    PaneId activePane() const;
    void selectPane(PaneId);
    Layout &layout();
    const Layout &layout() const;
    void resize(Size size);

  private:
    WindowId _id;
    std::string _name;
    std::unordered_map<PaneId, std::unique_ptr<Pane>> _panes;
    PaneId _activePane;
    Layout _layout;
    Size _size;
};
} // namespace mlux

#endif
