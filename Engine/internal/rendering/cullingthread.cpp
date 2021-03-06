#include "cullingthread.h"

#include "time2.h"
#include "renderer.h"
#include "profiler.h"
#include "frameevents.h"
#include "geometryutility.h"
#include "internal/base/renderdefines.h"
#include "internal/engine/sceneinternal.h"
#include "internal/rendering/renderingcontext.h"

CullingTask::CullingTask(RenderingContext* context) : context_(context) {
	time_ = context_->GetTime();
	scene_ = context_->GetScene();
	profiler_ = context_->GetProfiler();

	frustumPlanes_ = new Plane[6];
	lastTimeStamp_ = Time::GetTimeStamp();
}

CullingTask::~CullingTask() {
	delete[] frustumPlanes_;
}

void CullingTask::SetWorldToClipMatrix(const Matrix4& value) {
	GeometryUtility::CalculateFrustumPlanes(frustumPlanes_, value);
}

void CullingTask::Run() {
	uint64 start = Time::GetTimeStamp();

	visibleGameObjects_.clear();

	std::vector<GameObject*> allRenderers = scene_->GetGameObjectsOfComponent(Renderer::GetComponentGUID());
	for (GameObject* go : allRenderers) {
		if ((cullingMask_ & (1 << go->GetLayer())) == 0) {
			continue;
		}

		Renderer* renderer = go->GetComponent<Renderer>();
		if (!renderer->GetActiveAndEnabled() || !IsVisible(renderer)) {
			continue;
		}

		if (go->GetComponent<MeshProvider>()) {
			visibleGameObjects_.push_back(go);
		}
	}

	finished.raise();

	profiler_->SetVisibleGameObjects(visibleGameObjects_.size(), allRenderers.size());
	profiler_->SetCullingElapsed(Time::TimeStampToSeconds(Time::GetTimeStamp() - start));
}

bool CullingTask::IsVisible(Renderer* renderer) {
	const Bounds& bounds = renderer->GetBounds();
	if (bounds.IsEmpty()) { return false; }

	if (!GeometryUtility::FrustumIntersectsAABB(frustumPlanes_, bounds)) {
		return false;
	}

	return true;
}

//bool CullingTask::PrespectiveFrustumCulling(const Bounds& bounds, const Matrix4& worldToClipMatrix) {
//	IVector2 outx, outy, outz;
//	 
//	std::vector<Vector3> points;
//	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);
//
//	Vector2 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
//
//	for (int i = 0; i < points.size(); ++i) {
//		Vector4 p = worldToClipMatrix * Vector4(points[i].x, points[i].y, points[i].z, 1);
//
//		// Note that the frustum culling(clipping) is performed in the clip coordinates,
//		// just before dividing.
//
//		if (p.x < -p.w) { ++outx.x; }
//		else if (p.x > p.w) { ++outx.y; }
//
//		if (p.y < -p.w) { ++outy.x; }
//		else if (p.y > p.w) { ++outy.y; }
//
//		if (p.z < -p.w) { ++outz.x; }
//		else if (p.z > p.w) { ++outz.y; }
//
//		Vector2 p2(Mathf::Clamp(p.x / p.w, -1.f, 1.f), Mathf::Clamp(p.y / p.w, -1.f, 1.f));
//
//		min = Vector2::Min(min, p2);
//		max = Vector2::Max(max, p2);
//	}
//
//	if (outx.x == 8 || outx.y == 8 || outy.x == 8 || outy.y == 8 || outz.x == 8 || outz.y == 8) {
//		return false;
//	}
//
//	Vector2 size(max - min);
//	return size.GetSqrMagnitude() > MIN_NDC_RADIUS_SQUARED;
//}
