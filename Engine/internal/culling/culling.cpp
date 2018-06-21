#include "world.h"
#include "culling.h"
#include "geometryutility.h"
#include "internal/async/guard.h"
#include "internal/base/renderdefines.h"

Culling::Culling(CullingListener* listener) : listener_(listener), working_(false), stopped_(false) {
}

void Culling::run() {
	for (; !stopped_;) {
		if (working_) {
			entities_.clear();
			WorldInstance()->WalkEntityHierarchy(this);
			listener_->OnCullingFinished();

			working_ = false;
		}
	}
}

void Culling::Stop() {
	stopped_ = true;
}

void Culling::Cull(const glm::mat4& worldToClipMatrix) {
	if (!working_) {
		worldToClipMatrix_ = worldToClipMatrix;
		working_ = true;
	}
}

WorldEntityWalker::WalkCommand Culling::OnWalkEntity(Entity entity) {
	// TODO: fix bug for particle system by calculating its bounds.
	if (!IsVisible(entity, worldToClipMatrix_)) {
		return WorldEntityWalker::WalkCommandContinue;
	}

	if (!entity->GetActive()) {
		return WorldEntityWalker::WalkCommandNext;
	}

	if (entity->GetRenderer() && entity->GetMesh()) {
		entities_.push_back(entity);
	}

	return WorldEntityWalker::WalkCommandContinue;
}

bool Culling::IsVisible(Entity entity, const glm::mat4& worldToClipMatrix) {
	const Bounds& bounds = entity->GetBounds();
	if (bounds.IsEmpty()) {
		return false;
	}

	return FrustumCulling(bounds, worldToClipMatrix);
}

bool Culling::FrustumCulling(const Bounds& bounds, const glm::mat4& worldToClipMatrix) {
	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	bool inside = false;
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (int i = 0; i < points.size(); ++i) {
		glm::vec4 p = worldToClipMatrix * glm::vec4(points[i], 1);
		p /= p.w;
		if (p.x >= -1 && p.x <= 1 && p.y >= -1 && p.y <= 1 && p.z >= -1 && p.z <= 1) {
			inside = true;
		}

		points[i] = glm::vec3(p);
		min = glm::min(min, points[i]);
		max = glm::max(max, points[i]);
	}

	if (inside) {
		glm::vec2 size(max.x - min.x, max.y - min.y);
		return glm::dot(size, size) > MIN_NDC_RADIUS_SQUARED;
	}

	return false;
}
