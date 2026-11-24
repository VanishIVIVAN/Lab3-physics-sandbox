#include "sandbox/ShapeFactory.h"

#include <stdexcept>

#include "sandbox/Shapes.h"

namespace sandbox {

std::shared_ptr<const Shape> ShapeFactory::create(ShapeType type) {
  switch (type) {
    case ShapeType::Box:
      return std::make_shared<BoxShape>();
    case ShapeType::Circle:
      return std::make_shared<CircleShape>();
    case ShapeType::Triangle:
      return std::make_shared<TriangleShape>();
    case ShapeType::HeavyBall:
      return std::make_shared<HeavyBallShape>();
  }
  throw std::invalid_argument("ShapeFactory::create: unknown shape type");
}

const std::array<ShapeType, 4>& ShapeFactory::allTypes() {
  static const std::array<ShapeType, 4> kTypes = {ShapeType::Box, ShapeType::Circle,
                                                  ShapeType::Triangle, ShapeType::HeavyBall};
  return kTypes;
}

std::string ShapeFactory::toKey(ShapeType type) {
  switch (type) {
    case ShapeType::Box:
      return "box";
    case ShapeType::Circle:
      return "circle";
    case ShapeType::Triangle:
      return "triangle";
    case ShapeType::HeavyBall:
      return "heavy_ball";
  }
  throw std::invalid_argument("ShapeFactory::toKey: unknown shape type");
}

std::optional<ShapeType> ShapeFactory::fromKey(const std::string& key) {
  for (ShapeType type : allTypes()) {
    if (toKey(type) == key) return type;
  }
  return std::nullopt;
}

}  // namespace sandbox
