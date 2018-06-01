#include "world.h"
#include "culling.h"
#include "debug/debug.h"
#include "geometryutility.h"
#include "internal/base/renderdefines.h"

Culling::Culling(const glm::mat4& worldToClipMatrix, AsyncEventListener* receiver) : AsyncWorker(receiver), worldToClipMatrix_(worldToClipMatrix) {
}

void Culling::OnRun() {
	GetRenderableEntitiesInHierarchy(entities_, WorldInstance()->GetRootTransform(), worldToClipMatrix_);
}

void Culling::GetRenderableEntitiesInHierarchy(std::vector<Entity>& entities, Transform root, const glm::mat4& worldToClipMatrix) {
	for (int i = 0; i < root->GetChildCount(); ++i) {
		if (root->GetEntity()->GetStatus() != EntityStatusReady) {
			Debug::Break();
		}

		Entity child = root->GetChildAt(i)->GetEntity();
		if (child->GetStatus() != EntityStatusReady) {
			continue;
		}

		// TODO: fix bug for particle system by calculating its bounds.
		if (!IsVisible(child, worldToClipMatrix)) {
			//	continue;
		}

		if (child->GetActive() && child->GetRenderer() && child->GetMesh()) {
			entities.push_back(child);
		}

		GetRenderableEntitiesInHierarchy(entities, child->GetTransform(), worldToClipMatrix);
	}
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

void CullingThreadPool::GetVisibleEntities(const glm::mat4& worldToClipMatrix) {
	Execute(new Culling(worldToClipMatrix, this));
}

void CullingThreadPool::SetCullingListener(CullingListener* listener) {
	listener_ = listener;
}

void CullingThreadPool::OnSchedule(ZThread::Task& schedule) {
	Culling* culling = (Culling*)schedule.get();

	// main thread.

	if (listener_ != nullptr) {
		listener_->OnCullingFinished(culling);
	}
}
