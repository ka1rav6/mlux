#include "client.hpp"

#include "core/daemon/socket.hpp"

#include <array>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <string_view>
#include <sys/ioctl.h>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

namespace mlux
{

namespace
{
// set from the signal handler, which is why it cannot be anything richer.
volatile sig_atomic_t g_resized = 0;

void onSigwinch(int /* signal */)
{
    g_resized = 1;
}

// how long to keep trying after starting a daemon, in 20ms steps. the socket
// does not exist until the daemon has got as far as bind().
constexpr int DAEMON_WAIT_ATTEMPTS = 100;
constexpr int DAEMON_WAIT_STEP_US = 20000;

void writeAllToStdout(std::string_view text)
{
    const char* data = text.data();
    size_t size = text.size();

    while (size > 0)
    {
        const ssize_t written = ::write(STDOUT_FILENO, data, size);
        if (written < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return;
        }
        data += written;
        size -= static_cast<size_t>(written);
    }
}

void appendUtf8(std::string& out, char32_t character)
{
    if (character < 0x80)
    {
        out.push_back(static_cast<char>(character));
    }
    else if (character < 0x800)
    {
        out.push_back(static_cast<char>(0xc0 | (character >> 6)));
        out.push_back(static_cast<char>(0x80 | (character & 0x3f)));
    }
    else if (character < 0x10000)
    {
        out.push_back(static_cast<char>(0xe0 | (character >> 12)));
        out.push_back(static_cast<char>(0x80 | ((character >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (character & 0x3f)));
    }
    else
    {
        out.push_back(static_cast<char>(0xf0 | (character >> 18)));
        out.push_back(static_cast<char>(0x80 | ((character >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((character >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (character & 0x3f)));
    }
}
} // namespace

Client::Client(std::filesystem::path socketPath, std::string sessionName)
    : _socket_path(std::move(socketPath)), _session_name(std::move(sessionName))
{
}

Client::~Client()
{
    restoreTerminal();

    if (_socket >= 0)
    {
        close(_socket);
    }
}

int Client::run()
{
    if (isatty(STDIN_FILENO) == 0)
    {
        std::fputs("mlux: stdin is not a terminal\n", stderr);
        return 1;
    }

    if (!connectOrStartDaemon())
    {
        std::fputs("mlux: could not reach the daemon\n", stderr);
        return 1;
    }

    takeOverTerminal();

    if (!sendMessage(_socket, MessageType::Attach, encodeAttach(terminalSize(), _session_name)))
    {
        return 1;
    }

    std::array<pollfd, 2> watched{pollfd{STDIN_FILENO, POLLIN, 0}, pollfd{_socket, POLLIN, 0}};

    while (true)
    {
        if (poll(watched.data(), watched.size(), -1) < 0)
        {
            if (errno == EINTR)
            {
                // almost certainly the SIGWINCH below.
                if (g_resized != 0)
                {
                    g_resized = 0;
                    sendMessage(_socket, MessageType::Resize, encodeSize(terminalSize()));
                }
                continue;
            }
            break;
        }

        if ((watched[0].revents & POLLIN) != 0 && !forwardInput())
        {
            break;
        }

        if ((watched[1].revents & (POLLIN | POLLHUP)) != 0 && !readFromDaemon())
        {
            break;
        }
    }

    return 0;
}

bool Client::connectOrStartDaemon()
{
    _socket = connectToSocket(_socket_path);
    if (_socket >= 0)
    {
        return true;
    }

    startDaemon();

    for (int attempt = 0; attempt < DAEMON_WAIT_ATTEMPTS; ++attempt)
    {
        usleep(DAEMON_WAIT_STEP_US);

        _socket = connectToSocket(_socket_path);
        if (_socket >= 0)
        {
            return true;
        }
    }

    return false;
}

void Client::startDaemon() const
{
    const pid_t pid = fork();
    if (pid < 0)
    {
        throw std::system_error(errno, std::generic_category(), "fork");
    }

    if (pid != 0)
    {
        return;
    }

    // setsid is the whole point of this fork: it puts the daemon in its own
    // session so that the terminal this client is attached to is no longer its
    // controlling terminal, and closing that terminal cannot signal it.
    setsid();

    const int null = open("/dev/null", O_RDWR);
    if (null >= 0)
    {
        dup2(null, STDIN_FILENO);
        dup2(null, STDOUT_FILENO);
        dup2(null, STDERR_FILENO);
        if (null > STDERR_FILENO)
        {
            close(null);
        }
    }

    // re-exec this exact binary rather than looking mlux up in PATH, so the
    // daemon can never end up being a different build than the client.
    execl("/proc/self/exe", "mlux", "--daemon", nullptr);
    _exit(127);
}

void Client::takeOverTerminal()
{
    if (tcgetattr(STDIN_FILENO, &_saved_termios) < 0)
    {
        throw std::system_error(errno, std::generic_category(), "tcgetattr");
    }
    _terminal_taken = true;

    termios raw = _saved_termios;
    cfmakeraw(&raw);

    // read returns as soon as a single byte is there. anything else would add
    // latency between a keypress and the shell seeing it.
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) < 0)
    {
        throw std::system_error(errno, std::generic_category(), "tcsetattr");
    }

    struct sigaction action{};
    action.sa_handler = onSigwinch;
    sigemptyset(&action.sa_mask);
    sigaction(SIGWINCH, &action, nullptr);

    // alternate screen, so whatever was on the terminal before comes back
    // untouched on detach.
    writeAllToStdout("\x1b[?1049h\x1b[2J");
}

void Client::restoreTerminal()
{
    if (!_terminal_taken)
    {
        return;
    }
    _terminal_taken = false;

    writeAllToStdout("\x1b[?25h\x1b[?1049l");
    tcsetattr(STDIN_FILENO, TCSANOW, &_saved_termios);
}

Size Client::terminalSize()
{
    winsize window{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) < 0 || window.ws_row == 0)
    {
        // a sane default beats refusing to start when the terminal will not say.
        return Size{24, 80};
    }

    return Size{window.ws_row, window.ws_col};
}

bool Client::forwardInput()
{
    std::array<std::byte, 1024> buffer{};
    const ssize_t got = ::read(STDIN_FILENO, buffer.data(), buffer.size());
    if (got <= 0)
    {
        return got < 0 && errno == EINTR;
    }

    // raw bytes, straight through. what they mean is the daemon's business.
    return sendMessage(_socket, MessageType::Input, {buffer.data(), static_cast<size_t>(got)});
}

bool Client::readFromDaemon()
{
    const std::optional<Message> message = receiveMessage(_socket);
    if (!message.has_value())
    {
        return false;
    }

    switch (message->type)
    {
        case MessageType::Draw:
            draw(*message);
            return true;

        case MessageType::Detach:
            return false;

        default:
            return true;
    }
}

void Client::draw(const Message& message) const
{
    if (message.payload.size() < sizeof(DrawHeader))
    {
        return;
    }

    DrawHeader header{};
    std::memcpy(&header, message.payload.data(), sizeof header);

    const size_t expected = sizeof header + (header.cellCount * sizeof(CellUpdate));
    if (message.payload.size() < expected)
    {
        return;
    }

    std::vector<CellUpdate> cells(header.cellCount);
    if (header.cellCount > 0)
    {
        std::memcpy(cells.data(), message.payload.data() + sizeof header,
                    header.cellCount * sizeof(CellUpdate));
    }

    // one write for the whole update. drawing cell by cell would make the
    // terminal flicker and cost a syscall per character.
    std::string out = "\x1b[?25l";

    int last_row = -1;
    int last_column = -1;
    for (const CellUpdate& cell : cells)
    {
        // only move the cursor when the next cell is not where it already is.
        if (static_cast<int>(cell.row) != last_row || static_cast<int>(cell.column) != last_column)
        {
            out += "\x1b[" + std::to_string(cell.row + 1) + ";" + std::to_string(cell.column + 1) +
                   "H";
        }

        appendUtf8(out, cell.character == 0 ? U' ' : cell.character);

        last_row = cell.row;
        last_column = cell.column + 1;
    }

    out += "\x1b[" + std::to_string(header.cursorRow + 1) + ";" +
           std::to_string(header.cursorColumn + 1) + "H";
    if (header.cursorVisible != 0)
    {
        out += "\x1b[?25h";
    }

    writeAllToStdout(out);
}

} // namespace mlux
