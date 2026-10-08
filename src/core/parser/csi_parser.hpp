#ifndef MLUX_CSI_PARSER_H
#define MLUX_CSI_PARSER_H

#include "base_parser.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace mlux
{

enum class CSIState : uint8_t
{
    Entry,
    Marker,
    Param,
    Intermediate,
    Final
};

class CSIParser : public BaseParser
{
public:
    CSIParser(Terminal& terminal, ParserState& state);

    void feed(std::span<const std::byte> data) override;

private:
    void processByte(uint8_t byte);

    // CSI-specific state/data
    CSIState _csiState = CSIState::Entry;
    std::vector<int> _params;
    std::string _intermediate;
};

} // namespace mlux

#endif
