#include "sandbox/Renderer.h"

#include <algorithm>

#include "sandbox/Config.h"
#include "sandbox/Units.h"

namespace sandbox {
namespace {

sf::Vector2f toSf(Vec2 v) { return sf::Vector2f(v.x, v.y); }
sf::Color toSf(Rgba c) { return sf::Color(c.r, c.g, c.b, c.a); }

sf::Color darker(Rgba c) {
  return sf::Color(static_cast<sf::Uint8>(c.r * 0.55f), static_cast<sf::Uint8>(c.g * 0.55f),
                   static_cast<sf::Uint8>(c.b * 0.55f), c.a);
}

const sf::Color kBackground(24, 26, 32);
const sf::Color kBoundaryColor(88, 92, 108);
constexpr float kOutlinePx = 2.0f;

}  // namespace

Renderer::Renderer()
    : window_(sf::VideoMode(config::kWindowWidthPx, config::kWindowHeightPx), "2D Physics Sandbox",
              sf::Style::Titlebar | sf::Style::Close, sf::ContextSettings(0, 0, 4)) {
  window_.setVerticalSyncEnabled(true);
  window_.setKeyRepeatEnabled(false);  // holding S must not spam spawns
}

void Renderer::beginFrame() { window_.clear(kBackground); }

void Renderer::endFrame() { window_.display(); }

void Renderer::drawBoundaries() {
  for (const config::Rect& rect : config::boundaries()) {
    const float width = units::lengthToPixels(rect.halfSize.x * 2.0f);
    const float height = units::lengthToPixels(rect.halfSize.y * 2.0f);
    sf::RectangleShape shape(sf::Vector2f(width, height));
    shape.setOrigin(width / 2.0f, height / 2.0f);
    shape.setPosition(toSf(units::toPixels(rect.center)));
    shape.setFillColor(kBoundaryColor);
    window_.draw(shape);
  }
}

void Renderer::drawBodies(const PhysicsWorld& world) {
  world.forEachBody([this](const BodyView& body) { drawBody(body); });
}

void Renderer::drawBody(const BodyView& body) {
  const ShapeGeometry& geometry = body.shape.geometry();
  const Rgba color = body.shape.color();
  const sf::Vector2f position = toSf(units::toPixels(body.position));
  const float rotation = units::angleToScreenDegrees(body.angle);

  if (geometry.isCircle) {
    const float radius = units::lengthToPixels(geometry.radius);
    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(position);
    circle.setRotation(rotation);
    circle.setFillColor(toSf(color));
    circle.setOutlineColor(darker(color));
    circle.setOutlineThickness(-kOutlinePx);
    window_.draw(circle);

    // A spoke makes the rotation of the ball visible.
    sf::RectangleShape spoke(sf::Vector2f(radius, kOutlinePx));
    spoke.setOrigin(0.0f, kOutlinePx / 2.0f);
    spoke.setPosition(position);
    spoke.setRotation(rotation);
    spoke.setFillColor(darker(color));
    window_.draw(spoke);
  } else {
    sf::ConvexShape polygon(geometry.vertices.size());
    for (std::size_t i = 0; i < geometry.vertices.size(); ++i) {
      polygon.setPoint(i, toSf(units::offsetToPixels(geometry.vertices[i])));
    }
    polygon.setPosition(position);
    polygon.setRotation(rotation);
    polygon.setFillColor(toSf(color));
    polygon.setOutlineColor(darker(color));
    polygon.setOutlineThickness(-kOutlinePx);
    window_.draw(polygon);
  }
}

void Renderer::drawDebris(const DebrisEffect& debris) {
  for (const Fragment& fragment : debris.fragments()) {
    const float s = units::lengthToPixels(fragment.size);
    sf::ConvexShape piece(3);
    piece.setPoint(0, sf::Vector2f(0.0f, -s));
    piece.setPoint(1, sf::Vector2f(-0.9f * s, 0.6f * s));
    piece.setPoint(2, sf::Vector2f(0.9f * s, 0.6f * s));
    piece.setPosition(toSf(units::toPixels(fragment.position)));
    piece.setRotation(units::angleToScreenDegrees(fragment.angle));

    Rgba color = fragment.color;
    const float remaining = std::clamp(1.0f - fragment.age / fragment.lifetime, 0.0f, 1.0f);
    color.a = static_cast<std::uint8_t>(255.0f * remaining);
    piece.setFillColor(toSf(color));
    window_.draw(piece);
  }
}

}  // namespace sandbox
