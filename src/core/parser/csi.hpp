/*
 * this file contains the states of the csi parser that are possible
 * as well as the csi node and the lexer definition
 * */

#pragma once

#include <optional>
#include <utility>
#include <vector>

namespace mlux {
// the main node.
// each exit sequence of the type `ESC [ ______` follows this blueprint and
// will be parsed and converted to this struct for further processing and
// execution of the escape sequence
struct CSINode {
    std::optional<char> privateMarker;
    std::vector<int> params;
    std::optional<std::vector<char>> intermediate;
    char finalByte;

    CSINode() = delete;
    // default constructor for the node.
    CSINode(std::optional<char> privateMarker, std::vector<int> params,
            std::optional<std::vector<char>> intermediate, char finalByte)
        : privateMarker(privateMarker), params(std::move(params)),
          intermediate(std::move(intermediate)), finalByte(finalByte) {}
};
} // namespace mlux
