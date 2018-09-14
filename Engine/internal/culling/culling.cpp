#include "world.h"
#include "culling.h"
#include "renderer.h"
#include "statistics.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "internal/async/guard.h"
#include "internal/base/renderdefines.h"

#include "profiler.h"

Culling::Culling(CullingListener* listener) : cond_(mutex_), listener_(listener), working_(false), stopped_(false) {
}

void Culling::run() {
	for (; !stopped_;) {
		if (working_) {
			entities_.clear();
			uint64 start = Profiler::instance()->GetTimeStamp();
			World::instance()->CullingUpdate();

			World::instance()->WalkGameObjectHierarchy(this);
			listener_->OnCullingFinished();

			Statistics::instance()->SetCullingElapsed(
				Profiler::instance()->TimeStampToSeconds(Profiler::instance()->GetTimeStamp() - start)
			);

			working_ = false;
		}

		ZTHREAD_LOCK_SCOPE(mutex_);
		cond_.wait();
	}
}

void Culling::Stop() {
	stopped_ = true;
	cond_.broadcast();
}

void Culling::Cull(const glm::mat4& worldToClipMatrix) {
	if (!working_) {
		worldToClipMatrix_ = worldToClipMatrix;
		working_ = true;
		cond_.signal();
	}
}

WalkCommand Culling::OnWalkGameObject(GameObject go) {
	if (!IsVisible(go, worldToClipMatrix_)) {
		return WalkCommand::Continue;
	}

	if (!go->GetActive()) {
		return WalkCommand::Next;
	}

	if (go->GetComponent<IRenderer>() && go->GetComponent<IMeshFilter>()) {
		entities_.push_back(go);
	}

	return WalkCommand::Continue;
}

bool Culling::IsVisible(GameObject go, const glm::mat4& worldToClipMatrix) {
	const Bounds& bounds = go->GetBounds();
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

	glm::vec2 size(max - min);
	return glm::dot(size, size) > MIN_NDC_RADIUS_SQUARED;
}
