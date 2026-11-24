#include "sandbox/Shapes.h"

namespace sandbox {

BoxShape::BoxShape() {
  geometry_.isCircle = false;
  geometry_.vertices = {{-kHalfExtent, -kHalfExtent},
                        {kHalfExtent, -kHalfExtent},
                        {kHalfExtent, kHalfExtent},
                        {-kHalfExtent, kHalfExtent}};
}

CircleShape::CircleShape() {
  geometry_.isCircle = true;
  geometry_.radius = kRadius;
}

TriangleShape::TriangleShape() {
  // Vertices are offset so that the centroid is exactly at the local origin;
  // then the body position is also its centre of mass.
  geometry_.isCircle = false;
  geometry_.vertices = {{-kBase / 2.0f, -kHeight / 3.0f},
                        {kBase / 2.0f, -kHeight / 3.0f},
                        {0.0f, 2.0f * kHeight / 3.0f}};
}

HeavyBallShape::HeavyBallShape() {
  geometry_.isCircle = true;
  geometry_.radius = kRadius;
}

}  // namespace sandbox
