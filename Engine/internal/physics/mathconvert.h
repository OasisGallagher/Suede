#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "bullet/LinearMath/btVector3.h"
#include "bullet/LinearMath/btQuaternion.h"

inline btVector3 btConvert(const glm::vec3& value) { return btVector3(value.x, value.y, value.z); }
inline btQuaternion btConvert(const glm::quat& value) { return btQuaternion(value.x, value.y, value.z, value.w); }

inline glm::vec3 btConvert(const btVector3& value) { return glm::vec3(value.x(), value.y(), value.z()); }
inline glm::quat btConvert(const btQuaternion& value) { return glm::quat(value.w(), value.x(), value.y(), value.z()); }
