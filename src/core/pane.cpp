#include "pane.hpp"

#include <utility>

namespace mlux
{

namespace
{
// a pane exists before anyone knows how big the window is. the real size
// arrives from the layout on the first resize(), this is only so that the
// Terminal has a screen to write into until then.
constexpr Height INITIAL_HEIGHT = 24;
constexpr Width INITIAL_WIDTH = 80;
} // namespace

Pane::Pane(PaneId id)
    : _id(id), _terminal(std::make_unique<Terminal>(Size{INITIAL_HEIGHT, INITIAL_WIDTH})),
      _geometry(Rectangle{0, 0, INITIAL_HEIGHT, INITIAL_WIDTH})
{
}

PaneId Pane::id() const
{
    return _id;
}

void Pane::attach(std::unique_ptr<Pty> pty)
{
    _pty = std::move(pty);
    _process_id = _pty->childPid();

    // the child was spawned with whatever size forkpty defaulted to, so tell
    // it what it actually got.
    _pty->resize(Size{_geometry.outer.height, _geometry.outer.width});
}

void Pane::detach()
{
    // dropping the Pty closes the master, which hangs up the child.
    _pty.reset();
    _process_id = 0;
}

Pty* Pane::pty()
{
    return _pty.get();
}

const Pty* Pane::pty() const
{
    return _pty.get();
}

Terminal& Pane::terminal()
{
    return *_terminal;
}

const Terminal& Pane::terminal() const
{
    return *_terminal;
}

void Pane::resize(Size size)
{
    _terminal->resize(size);

    if (_pty)
    {
        // this is what makes the program inside the pane re-wrap its output.
        _pty->resize(size);
    }
}

const PaneGeometry& Pane::geometry() const
{
    return _geometry;
}

void Pane::setGeometry(PaneGeometry geometry)
{
    _geometry = geometry;
}

void Pane::sendInput(std::span<const std::byte> data)
{
    // a pane with no pty has nothing listening, so the bytes are dropped
    // rather than queued. there is nothing to replay them to.
    if (_pty)
    {
        _pty->write(data);
    }
}

ProcessId Pane::processId() const
{
    return _process_id;
}

} // namespace mlux
