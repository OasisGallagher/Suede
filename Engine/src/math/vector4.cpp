#include "math/vector4.h"

Vector4 Vector4::zero;
Vector4 Vector4::one(1);

Vector4::Vector4() : Vector4(0) {
}

Vector4::Vector4(float x) : Vector4(x, x, x, x) {
}

Vector4::Vector4(float x, float y) : Vector4(x, y, 0) {
}

Vector4::Vector4(float x, float y, float z) : Vector4(x, y, z, 0) {
}

Vector4::Vector4(float x, float y, float z, float w) : x_(x), y_(y), z_(z), w_(w) {
}
