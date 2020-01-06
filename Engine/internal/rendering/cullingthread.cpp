#include "cullingthread.h"

#include "mesh.h"
#include "time2.h"
#include "renderer.h"
#include "profiler.h"
#include "geometryutility.h"
#include "internal/base/renderdefines.h"
#include "internal/engine/sceneinternal.h"
#include "internal/rendering/renderingcontext.h"

CullingThread::CullingThread(RenderingContext* context) : context_(context) {
	time_ = context_->GetTime();
	scene_ = context_->GetScene();
	profiler_ = context_->GetProfiler();

	lastTimeStamp_ = Time::GetTimeStamp();
}

bool CullingThread::OnWork() {
	gameObjects_.clear();
	uint64 start = Time::GetTimeStamp();

	if (time_->GetFrameCount() != cullingUpdateFrame_) {
		_suede_rptr(scene_)->CullingUpdate((float)Time::TimeStampToSeconds(start - lastTimeStamp_));
		cullingUpdateFrame_ = time_->GetFrameCount();
	}

	scene_->WalkGameObjectHierarchy([this](GameObject* go) { return OnWalkGameObject(go); });

	cullingFinished.raise();

	profiler_->SetCullingElapsed(
		Time::TimeStampToSeconds(Time::GetTimeStamp() - start)
	);

	lastTimeStamp_ = start;
	return true;
}

void CullingThread::Cull(const Matrix4& worldToClipMatrix) {
	if (!IsStopped() && !IsWorking()) {
		worldToClipMatrix_ = worldToClipMatrix;
		SetWorking(true);
	}
}

WalkCommand CullingThread::OnWalkGameObject(GameObject* go) {
	if (IsStopped()) { return WalkCommand::Break; }

	if (!go->GetActive()) {
		return WalkCommand::Next;
	}

	if (!IsVisible(go, worldToClipMatrix_)) {
		return WalkCommand::Continue;
	}

	if (go->GetComponent<Renderer>() && go->GetComponent<MeshProvider>()) {
		gameObjects_.push_back(go);
	}

	return WalkCommand::Continue;
}

bool CullingThread::IsVisible(GameObject* go, const Matrix4& worldToClipMatrix) {
	Renderer* renderer = go->GetComponent<Renderer>();
	if (renderer == nullptr) { return false; }

	const Bounds& bounds = renderer->GetBounds();
	if (bounds.IsEmpty()) { return false; }

	return FrustumCulling(bounds, worldToClipMatrix);
}

bool CullingThread::FrustumCulling(const Bounds& bounds, const Matrix4& worldToClipMatrix) {
	Vector2 outx, outy, outz;

	std::vector<Vector3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	Vector2 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());

	for (int i = 0; i < points.size(); ++i) {
		Vector4 p = worldToClipMatrix * Vector4(points[i].x, points[i].y, points[i].z, 1);

		// Note that the frustum culling(clipping) is performed in the clip coordinates,
		// just before dividing.

		if (p.x < -p.w) { ++outx.x; }
		else if (p.x > p.w) { ++outx.y; }

		if (p.y < -p.w) { ++outy.x; }
		else if (p.y > p.w) { ++outy.y; }

		if (p.z < -p.w) { ++outz.x; }
		else if (p.z > p.w) { ++outz.y; }

		Vector2 p2(Mathf::Clamp(p.x / p.w, -1.f, 1.f), Mathf::Clamp(p.y / p.w, -1.f, 1.f));

		min = Vector2::Min(min, p2);
		max = Vector2::Max(max, p2);
	}

	if (outx.x == 8 || outx.y == 8 || outy.x == 8 || outy.y == 8 || outz.x == 8 || outz.y == 8) {
		return false;
	}

	Vector2 size(max - min);
	return size.GetSqrMagnitude() > MIN_NDC_RADIUS_SQUARED;
}
