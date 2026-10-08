#ifndef MLUX_SESSION_H
#define MLUX_SESSION_H

#include "definitions.hpp"
#include "window.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mlux
{

class Session
{
public:
    Session(SessionId id, std::string name);

    [[nodiscard]] SessionId id() const;
    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] WindowId activeWindow() const;

    Window& createWindow();
    void destroyWindow(WindowId id);
    [[nodiscard]] Window* getWindow(WindowId id);
    [[nodiscard]] const Window* getWindow(WindowId id) const;
    void selectWindow(WindowId id);

    [[nodiscard]] std::vector<WindowId> windows() const;

private:
    SessionId _id = 0;
    std::string _name;
    std::unordered_map<WindowId, std::unique_ptr<Window>> _windows;
    WindowId _active_window = 0;
    std::chrono::system_clock::time_point _creation_time;
};

} // namespace mlux

#endif
