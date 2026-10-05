#ifndef MLUX_PANE_H
#define MLUX_PANE_H
#include "definitions.hpp"
#include "geometry.hpp"
#include "pty.hpp"

#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mlux {

class Pane {
  public:
    Pane(PaneId id);
    [[nodiscard]] PaneId id() const;
    void attach(std::unique_ptr<Pty>);
    void detach();
    Pty *pty();
    Terminal &terminal();
    const Terminal &terminal() const;
    void resize(uint16_t width, uint16_t height);
    const PaneGeometry &geometry() const;
    void setGeometry(PaneGeometry);
    void sendInput(std::span<const std::byte> data);
    ProcessId processId() const;

  private:
    PaneId _id;
    std::unique_ptr<Pty> _pty;
    std::unique_ptr<Terminal> _terminal;
    PaneGeometry _geometry;
    ProcessId _processId;
};
} // namespace mlux

#endif
