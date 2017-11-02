#include "math/vector2.h"

Vector2 Vector2::zero;
Vector2 Vector2::one(1);

Vector2::Vector2() : Vector2(0) {
}

Vector2::Vector2(float x) : Vector2(x, x) {
}

Vector2::Vector2(float x, float y) : x_(x), y_(y) {
}
