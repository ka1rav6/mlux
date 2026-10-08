#ifndef MLUX_LAYOUT_H
#define MLUX_LAYOUT_H

#include "definitions.hpp"
#include "geometry.hpp"

#include <cstdint>
#include <memory>
#include <optional>

namespace mlux
{
// Layout tree node. Can be of three types :
// Pane : THe main original pane
// HorizontalSplit : Both the child nodes created after a pane is horizontally
// split into half
// VerticalSplit : Both the child nodes created after a pane is
// veritcally split into half
class LayoutNode
{
public:
    enum class Type : uint8_t
    {
        Pane,
        HorizontalSplit,
        VerticalSplit
    };

    [[nodiscard]] Type type() const;
    [[nodiscard]] std::optional<PaneId> pane() const;

    [[nodiscard]] LayoutNode* first();
    [[nodiscard]] const LayoutNode* first() const;
    [[nodiscard]] LayoutNode* second();
    [[nodiscard]] const LayoutNode* second() const;

private:
    Type _type = Type::Pane;
    std::optional<PaneId> _pane;
    std::unique_ptr<LayoutNode> _first;
    std::unique_ptr<LayoutNode> _second;
    float _ratio = 0.5F;
};

// The main Layout Tree that contains the root layout node.
class Layout
{
public:
    PaneId addPane();
    void removePane(PaneId id);
    void splitHorizontal(PaneId id);
    void splitVertical(PaneId id);
    void resizePane(PaneId id, int delta);
    void calculate(Size windowSize);
    [[nodiscard]] std::optional<Rectangle> geometry(PaneId id) const;

private:
    std::unique_ptr<LayoutNode> _root;
};
} // namespace mlux

#endif
