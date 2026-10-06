/*
 * defines a data structure that provides all the necessary information about
 * the process that it is representing. Things like the cli arguments,
 * exectuable path, environment variables, working directory.
 *
 */

#ifndef MLUX_PROCESS_H
#define MLUX_PROCESS_H

#include <filesystem>
#include <string>
#include <vector>

namespace mlux {
struct ProcessSpec {
    std::string executable;
    std::vector<std::string> arguments;
    std::vector<std::string> environment;
    std::filesystem::path workingDirectory;
    std::string shell;

    ProcessSpec(std::string executable, std::vector<std::string> &arguments,
                std::vector<std::string> &environment,
                std::filesystem::path workingDirectory, std::string &shell)
        : executable(std::move(executable)), arguments(std::move(arguments)),
          environment(std::move(environment)),
          workingDirectory(std::move(workingDirectory)), shell(shell) {}
    ~ProcessSpec() = default;
};

} // namespace mlux

#endif
