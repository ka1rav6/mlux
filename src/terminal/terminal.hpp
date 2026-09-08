#ifndef MLUX_TERMINAL
#define MLUX_TERMINAL

namespace mlux {

class Terminal {
  public:
    Terminal() = default;
    ~Terminal() = default;
    Terminal(const Terminal &) = delete;
    Terminal(Terminal &&) = delete;
    void enable_raw_mode();
    void disable_raw_mode();
};

} // namespace mlux

#endif
