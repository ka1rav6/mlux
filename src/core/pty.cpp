#include "pty.hpp"

#include <cerrno>
#include <cstdlib>
#include <pty.h>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

namespace mlux
{

std::unique_ptr<Pty> spawn(const ProcessSpec& process)
{
    FileDescriptor master_fd = -1;
    // the rest of the params are nullptr so that they can be dynamically changed
    ProcessId pid = forkpty(&master_fd, nullptr, nullptr, nullptr);

    if (pid == 0)
    {
        // Child process.
        execvp(process.getExecutable(), process.getArgs().data());

        // execvp() only returns on failure.
        perror("execvp");
        _exit(127);
    }

    if (pid < 0)
    {
        throw std::system_error(errno, std::generic_category(), "forkpty");
    }

    // Parent process.
    // Not waiting here otherwise this function would not return
    return std::make_unique<Pty>(master_fd, pid);
}
void Pty::write(std::span<const std::byte> data) const
{
    while (!data.empty())
    {
        const ssize_t written = ::write(_master_fd, data.data(), data.size_bytes());
        if (written < 0)
        {
            if (errno == EINTR)
            {
                continue;
            } // retry
            throw std::system_error(errno, std::generic_category(), "write");
        }
        data = data.subspan(static_cast<size_t>(written)); // advance past what went out
    }
}

ssize_t Pty::read(std::span<std::byte>& buffer) const
{
    return ::read(_master_fd, buffer.data(), buffer.size_bytes());
}

ProcessId Pty::childPid() const
{
    return _child_pid;
}
Pty::~Pty()
{
    int ret;
    waitpid(_child_pid, &ret, 0);
}

} // namespace mlux
