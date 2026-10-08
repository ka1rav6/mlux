#pragma once

#include "definitions.hpp"
#include "geometry.hpp"
#include "layout.hpp"
#include "pane.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace mlux {

class Window {
  public:
    Window(WindowId id, std::string name);

    [[nodiscard]] WindowId id() const;
    [[nodiscard]] const std::string &name() const;
    void rename(std::string name);
    Pane &createPane();
    void destroyPane(PaneId id);
    [[nodiscard]] Pane *getPane(PaneId id);
    [[nodiscard]] const Pane *getPane(PaneId id) const;
    [[nodiscard]] PaneId activePane() const;
    void selectPane(PaneId id);
    [[nodiscard]] Layout &layout();
    [[nodiscard]] const Layout &layout() const;
    void resize(Size size);

  private:
    WindowId _id = 0;
    std::string _name;
    std::unordered_map<PaneId, std::unique_ptr<Pane>> _panes;
    PaneId _activePane = 0;
    Layout _layout;
    Size _size;
};
} // namespace mlux
