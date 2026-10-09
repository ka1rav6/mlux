/*
 * the client process. this is what runs when you type `mlux`.
 *
 * it owns the user's real terminal and nothing else: no sessions, no panes, no
 * ptys, no parser. it copies keystrokes to the daemon and draws whatever the
 * daemon sends back. all of the state lives on the other side of the socket
 *
 */

#ifndef MLUX_CLIENT_H
#define MLUX_CLIENT_H

#include "core/daemon/protocol.hpp"
#include "core/definitions.hpp"
#include "core/geometry.hpp"

#include <filesystem>
#include <string>
#include <termios.h>

namespace mlux
{

class Client
{
public:
    Client(std::filesystem::path socketPath, std::string sessionName);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    int run();

private:
    // the client starts the daemon process if it is not existing
    bool connectOrStartDaemon();
    void startDaemon() const;

    void takeOverTerminal();
    void restoreTerminal();
    [[nodiscard]] static Size terminalSize();

    bool forwardInput();
    bool readFromDaemon();
    void draw(const Message& message) const;

    std::filesystem::path _socket_path;
    std::string _session_name;
    FileDescriptor _socket = -1;

    termios _saved_termios{};
    bool _terminal_taken = false;
};

} // namespace mlux

#endif
