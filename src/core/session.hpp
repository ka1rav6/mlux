#ifndef MLUX_SESSION_H
#define MLUX_SESSION_H

#include "definitions.hpp"
#include "window.hpp"

#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mlux {

class Session {
  public:
    Session(SessionId id, std::string name);
    ~Session() = default;
    [[nodiscard]] SessionId id() const;

    const std::string &name() const;
    WindowId activeWindow() const;
    Window &createWindow();
    void destroyWindow(WindowId);
    Window *getWindow(WindowId);
    const Window *getWindow(WindowId) const;
    void selectWindow(WindowId);

    std::vector<WindowId> windows() const;

  private:
    SessionId _id;
    std::string _name;
    std::unordered_map<WindowId, std::unique_ptr<Window>> _windows;
    WindowId _activeWindow;
    time_t _creationTime;
};

} // namespace mlux

#endif
