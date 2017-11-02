#include "math/vector3.h"

Vector3 Vector3::zero;
Vector3 Vector3::one = Vector3(1);

Vector3 Vector3::up(0, 1, 0);
Vector3 Vector3::right(1, 0, 0);
Vector3 Vector3::forward(0, 0, -1);

Vector3::Vector3() : Vector3(0) {
}

Vector3::Vector3(float x) : Vector3(x, x, x) {
}

Vector3::Vector3(float x, float y, float z) : x_(x), y_(y), z_(z) {
}
