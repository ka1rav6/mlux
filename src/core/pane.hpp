#pragma once

#include "definitions.hpp"
#include "geometry.hpp"
#include "pty.hpp"
#include "terminal.hpp"

#include <cstddef>
#include <memory>
#include <span>

namespace mlux {
// the main pane class that the user sees.
// It contains a single pty (psuedoterminal) at a particular point of time.
// right now I have added the functions to attach to another/detach from a pty
// as well
class Pane {
  public:
    explicit Pane(PaneId id);

    [[nodiscard]] PaneId id() const;
    void attach(std::unique_ptr<Pty> pty);
    void detach();
    [[nodiscard]] Pty *pty();
    [[nodiscard]] const Pty *pty() const;
    [[nodiscard]] Terminal &terminal();
    [[nodiscard]] const Terminal &terminal() const;
    void resize(Size size);
    [[nodiscard]] const PaneGeometry &geometry() const;
    void setGeometry(PaneGeometry geometry);
    void sendInput(std::span<const std::byte> data);
    [[nodiscard]] ProcessId processId() const;

  private:
    PaneId _id = 0;
    std::unique_ptr<Pty> _pty;
    std::unique_ptr<Terminal> _terminal;
    PaneGeometry _geometry;
    ProcessId _processId = 0;
};
} // namespace mlux
