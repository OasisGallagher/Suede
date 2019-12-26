#include "cullingthread.h"

#include "mesh.h"
#include "time2.h"
#include "renderer.h"
#include "profiler.h"
#include "geometryutility.h"
#include "internal/base/renderdefines.h"
#include "internal/engine/sceneinternal.h"
#include "internal/rendering/renderingcontext.h"

CullingThread::CullingThread(RenderingContext* context) : context_(context), working_(false), stopped_(false) {
	thread_ = std::thread(std::bind(&CullingThread::Run, this));
}

CullingThread::~CullingThread() {
	Stop();
	thread_.join();
}

void CullingThread::Run() {
	auto walker = [this](GameObject* go) { return OnWalkGameObject(go); };
	Time* time = context_->GetTime();
	Profiler* profiler = context_->GetProfiler();
	SceneInternal* scene = ((SceneInternal*)context_->GetScene()->d_);

	int cullingUpdateFrame = -1;
	uint64 lastTimeStamp = Time::GetTimeStamp();

	for (; !stopped_;) {
		if (working_) {
			gameObjects_.clear();
			uint64 start = Time::GetTimeStamp();

			if (time->GetFrameCount() != cullingUpdateFrame) {
				scene->CullingUpdate((float)Time::TimeStampToSeconds(start - lastTimeStamp));
				cullingUpdateFrame = time->GetFrameCount();
			}

			scene->WalkGameObjectHierarchy(walker);

			cullingFinished.raise();

			profiler->SetCullingElapsed(
				Time::TimeStampToSeconds(Time::GetTimeStamp() - start)
			);

			working_ = false;
			lastTimeStamp = start;
		}

		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock);
	}
}

void CullingThread::Stop() {
	if (!stopped_) {
		stopped_ = true;
		cond_.notify_all();
	}
}

void CullingThread::Cull(const Matrix4& worldToClipMatrix) {
	if (!working_) {
		worldToClipMatrix_ = worldToClipMatrix;
		working_ = true;
		cond_.notify_one();
	}
}

WalkCommand CullingThread::OnWalkGameObject(GameObject* go) {
	if (!IsVisible(go, worldToClipMatrix_)) {
		return WalkCommand::Continue;
	}

	if (!go->GetActive()) {
		return WalkCommand::Next;
	}

	if (go->GetComponent<Renderer>() && go->GetComponent<MeshProvider>()) {
		gameObjects_.push_back(go);
	}

	return WalkCommand::Continue;
}

bool CullingThread::IsVisible(GameObject* go, const Matrix4& worldToClipMatrix) {
	const Bounds& bounds = go->GetBounds();
	if (bounds.IsEmpty()) {
		return false;
	}

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
