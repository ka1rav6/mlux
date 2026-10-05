#ifndef MLUX_LAYOUT_H
#define MLUX_LAYOUT_H

#include "definitions.hpp"
#include "geometry.hpp"

#include <memory>
#include <optional>

namespace mlux {
class LayoutNode {
  public:
    enum class Type : uint8_t { Pane, HorizontalSplit, VerticalSplit };
    [[nodiscard]] Type type() const;
    [[nodiscard]] PaneId pane() const;

    LayoutNode *first();
    LayoutNode *second();

  private:
    Type _type;
    std::optional<PaneId> _pane;
    std::unique_ptr<LayoutNode> _first;
    std::unique_ptr<LayoutNode> _second;
    float _ratio;
};

class Layout {
  public:
    PaneId addPane();
    void removePane(PaneId);
    void splitHorizontal(PaneId);
    void splitVertical(PaneId);
    void resizePane(PaneId, int delta);
    void calculate(Size windowSize);
    Rectangle geometry(PaneId) const;

  private:
    std::unique_ptr<LayoutNode> _root;
};
} // namespace mlux

#endif
