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
#include <utility>
#include <vector>

namespace mlux
{
struct ProcessSpec
{
    std::string executable;
    std::vector<std::string> arguments;
    std::vector<std::string> environment;
    std::filesystem::path workingDirectory;
    std::string shell;

    ProcessSpec(std::string executable, std::vector<std::string> arguments,
                std::vector<std::string> environment, std::filesystem::path workingDirectory,
                std::string shell)
        : executable(std::move(executable)), arguments(std::move(arguments)),
          environment(std::move(environment)), workingDirectory(std::move(workingDirectory)),
          shell(std::move(shell))
    {
    }
    //-----------------------------------------------------------------------------------------------------------//
    // The following functions are created to be a wrapper for interoperability with C APIs

    [[nodiscard]] std::vector<char*> getArgs() const
    {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(this->executable.c_str()));
        for (const auto& arg : this->arguments)
        {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }

        argv.push_back(nullptr);
        return argv;
    }
    [[nodiscard]] const char* getExecutable() const
    {
        return this->executable.c_str();
    }
    [[nodiscard]] std::vector<char*> getEnv() const
    {
        std::vector<char*> envp(environment.size());
        for (const auto& env : this->environment)
        {
            envp.push_back(const_cast<char*>(env.c_str()));
        }
        envp.push_back(nullptr);
        return envp;
    }
};

} // namespace mlux

#endif
