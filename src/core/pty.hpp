/*
 * wrapper over a pty. contains all the necessary information about a specific
 * pty and wrappers for all the functions of the pty
 *
 */

#ifndef MLUX_PTY_H
#define MLUX_PTY_H

#include "definitions.hpp"
#include "geometry.hpp"
#include "process.hpp"

#include <ctime>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#define READ_BUFFER_SIZE 2048

namespace mlux
{
class Pty
{
public:
    Pty(FileDescriptor _master_fd, ProcessId cpid) : _master_fd(_master_fd), _child_pid(cpid) {}
    Pty(const Pty&) = delete;
    static std::unique_ptr<Pty> spawn(const ProcessSpec& process);
    ~Pty();
    [[nodiscard]] int masterFd() const;
    void resize(Size size);
    // write is still a `const` function as we are writing through the FileDescriptor
    void write(std::span<const std::byte>) const;
    ssize_t read(std::span<std::byte>& buffer) const;
    [[nodiscard]] ProcessId childPid() const;

private:
    FileDescriptor _master_fd;
    ProcessId _child_pid;
};
} // namespace mlux

#endif
