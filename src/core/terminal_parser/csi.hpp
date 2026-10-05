/*
 * this file contains the states of the csi parser that aer possible
 * as well as the csi node and the lexer definition
 * */

#pragma once

#include <optional>
#include <vector>

namespace mlux {

// the number of states the parser (finite state machine) can be in
enum class csi_parser_state {
    GROUND,
    CSI_ENTRY,
    CSI_MARKER,
    CSI_PARAM,
    CSI_INTERMEDIATE,
    CSI_FINAL
};

// the main node.
// each exit sequence of the type `ESC [ ______` follows this blueprint and
// will be parsed and converted to this struct for further processing and
// execution of the escape sequence
struct csi_node { // TODO: create copy and move constructors
    std::optional<char> private_marker;
    std::vector<int> params;
    std::optional<std::vector<char>> intermediate;
    char final;
    csi_node() = delete;
    // default constructor for the node.
    csi_node(std::optional<char> private_marker, std::vector<int> params,
             std::optional<std::vector<char>> intermediate, char final)
        : private_marker(private_marker), params(std::move(params)),
          intermediate(std::move(intermediate)), final(final) {}
    ~csi_node() = default;
};
} // namespace mlux
