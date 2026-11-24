#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string>

#include "sandbox/Shape.h"

namespace sandbox {

/// Creates Shape objects by type and maps types to/from their JSON keys.
class ShapeFactory {
 public:
  /// Throws std::invalid_argument for an unknown type.
  static std::shared_ptr<const Shape> create(ShapeType type);

  static const std::array<ShapeType, 4>& allTypes();

  /// Stable key used in scene files ("box", "circle", "triangle", "heavy_ball").
  static std::string toKey(ShapeType type);
  static std::optional<ShapeType> fromKey(const std::string& key);
};

}  // namespace sandbox
