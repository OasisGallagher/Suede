#pragma once

#include "bullet/LinearMath/btVector3.h"
#include "bullet/LinearMath/btQuaternion.h"

inline btVector3 btConvert(const Vector3& value) { return btVector3(value.x, value.y, value.z); }
inline btQuaternion btConvert(const Quaternion& value) { return btQuaternion(value.x, value.y, value.z, value.w); }

inline Vector3 btConvert(const btVector3& value) { return Vector3(value.x(), value.y(), value.z()); }
inline Quaternion btConvert(const btQuaternion& value) { return Quaternion(value.w(), value.x(), value.y(), value.z()); }
