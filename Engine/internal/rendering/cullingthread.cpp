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

	lastTimeStamp_ = Time::GetTimeStamp();
}

void CullingTask::Run() {
	gameObjects_.clear();
	scene_->WalkGameObjectHierarchy([this](GameObject* go) { return OnWalkGameObject(go); });
	finished.raise();
}

WalkCommand CullingTask::OnWalkGameObject(GameObject* go) {
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

bool CullingTask::IsVisible(GameObject* go, const Matrix4& worldToClipMatrix) {
	Renderer* renderer = go->GetComponent<Renderer>();
	if (renderer == nullptr) { return false; }

	const Bounds& bounds = renderer->GetBounds();
	if (bounds.IsEmpty()) { return false; }

	return FrustumCulling(bounds, worldToClipMatrix);
}

bool CullingTask::FrustumCulling(const Bounds& bounds, const Matrix4& worldToClipMatrix) {
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

CullingThread::CullingThread(RenderingContext* context) : ThreadPool(4), context_(context) {
	cullingUpdateTask_ = new UpdateTask();
	Engine::GetSubsystem<FrameEvents>()->frameEnter.subscribe(this, &CullingThread::OnFrameEnter, (int)FrameEventQueue::CullingThread);
	Engine::GetSubsystem<FrameEvents>()->frameLeave.subscribe(this, &CullingThread::OnFrameLeave, (int)FrameEventQueue::CullingThread);
}

CullingThread::~CullingThread() {
	Engine::GetSubsystem<FrameEvents>()->frameEnter.unsubscribe(this);
	Engine::GetSubsystem<FrameEvents>()->frameLeave.unsubscribe(this);
}

void CullingThread::OnFrameEnter() {
	AddTask(cullingUpdateTask_.get());
}

void CullingThread::OnFrameLeave() {
	WaitFinished();
}

CullingThread::UpdateTask::UpdateTask() {
	time_ = Engine::GetSubsystem<Time>();
	scene_ = Engine::GetSubsystem<Scene>();
	profiler_ = Engine::GetSubsystem<Profiler>();
}

void CullingThread::UpdateTask::Run() {
	uint64 start = Time::GetTimeStamp();
	_suede_rptr(scene_)->CullingUpdate(time_->GetDeltaTime());
	profiler_->SetCullingElapsed(
		Time::TimeStampToSeconds(Time::GetTimeStamp() - start)
	);
}
