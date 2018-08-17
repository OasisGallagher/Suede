#include "world.h"
#include "culling.h"
#include "tools/math2.h"
#include "geometryutility.h"
#include "internal/async/guard.h"
#include "internal/base/renderdefines.h"

#include "profiler.h"

Culling::Culling(CullingListener* listener) : cond_(mutex_), listener_(listener), working_(false), stopped_(false) {
}

#include <Windows.h>
#undef min
#undef max

class Synchronizer {
public:
	Synchronizer() : time_(INT_MAX) {}

public:
	void WaitForTargetFps() {
		const int FRAMES_PER_SECOND = 60;
		const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
		int next_game_tick = time_ + SKIP_TICKS;

		int sleep_time = next_game_tick - GetTickCount();
		if (sleep_time >= 0) {
			Sleep(sleep_time);
		}
		else {
			Debug::Output("");
		}
	}

private:
	int time_;
};

void Culling::run() {
	for (; !stopped_;) {
		if (working_) {
			entities_.clear();
			uint64 start = Profiler::instance()->GetTimeStamp();
			World::instance()->CullingUpdate();
			double update = Profiler::instance()->TimeStampToSeconds(Profiler::instance()->GetTimeStamp() - start);

			World::instance()->WalkEntityHierarchy(this);
			listener_->OnCullingFinished();

			double delta = Profiler::instance()->TimeStampToSeconds(Profiler::instance()->GetTimeStamp() - start);

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

	glm::vec2 size(max - min);
	return glm::dot(size, size) > MIN_NDC_RADIUS_SQUARED;
}
