#ifndef MLUX_CSI
#define MLUX_CSI

#include <optional>
#include <vector>

namespace mlux {

enum class csi_parser_state {
    GROUND,
    CSI_ENTRY,
    CSI_MARKER,
    CSI_PARAM,
    CSI_INTERMEDIATE,
    CSI_FINAL
};

struct csi_node {
    std::optional<char> private_marker;
    std::vector<int> params;
    std::optional<std::vector<char>> intermediate;
    char final;
    csi_node(std::optional<char> private_marker, std::vector<int> params,
             std::optional<std::vector<char>> intermediate, char final)
        : private_marker(private_marker), params(std::move(params)),
          intermediate(std::move(intermediate)), final(final) {}
    ~csi_node() = default;
};

class byte_lexer {
    byte_lexer(const char *src);
    ~byte_lexer() = default;

  private:
    const char *src;
    std::vector<int> split_params();
    size_t current = 0;
    csi_parser_state state;
    char advance();
    char peek();
    bool is_alpha(char c);
    bool is_int(char c);
    bool is_esc(char c);
};

} // namespace mlux

#endif
