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

namespace mlux
{
class Pty
{
public:
    Pty() = delete;
    Pty(const Pty&) = delete;
    static std::unique_ptr<Pty> spawn(const ProcessSpec& process, Size size);
    ~Pty();
    [[nodiscard]] int masterFd() const;
    void write(std::span<const std::byte>);
    size_t read(std::span<std::byte>);
    void resize(Size size);
    [[nodiscard]] ProcessId childPid() const;
    [[nodiscard]] bool alive() const;

private:
    FileDescripto

        _master_fd ProcessId _child_pid;
};
} // namespace mlux

#endif
