#include "world.h"
#include "culling.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "internal/async/guard.h"
#include "internal/base/renderdefines.h"

Culling::Culling(CullingListener* listener) : listener_(listener), working_(false), stopped_(false) {
}

void Culling::run() {
	for (; !stopped_;) {
		if (working_) {
			entities_.clear();
			World::get()->WalkEntityHierarchy(this);
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
	glm::ivec2 outx, outy, outz;

	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	glm::vec2 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	for (int i = 0; i < points.size(); ++i) {
		glm::vec4 p = worldToClipMatrix * glm::vec4(points[i], 1);

		// Note that the frustum culling(clipping) is performed in the clip coordinates,
		// just before dividing.

		if (p.x < -p.w) { ++outx.x; }
		else if (p.x > p.w) { ++outx.y; }

		if (p.y < -p.w) { ++outy.x; }
		else if (p.y > p.w) { ++outy.y; }

		if (p.z < -p.w) { ++outz.x; }
		else if (p.z > p.w) { ++outz.y; }

		glm::vec2 p2(Math::Clamp(p.x / p.w, -1.f, 1.f), Math::Clamp(p.y / p.w, -1.f, 1.f));

		min = glm::min(min, p2);
		max = glm::max(max, p2);
	}

	if (outx.x == 8 || outx.y == 8 || outy.x == 8 || outy.y == 8 || outz.x == 8 || outz.y == 8) {
		return false;
	}

	glm::vec2 size(max.x - min.x, max.y - min.y);
	return glm::dot(size, size) > MIN_NDC_RADIUS_SQUARED;
}
