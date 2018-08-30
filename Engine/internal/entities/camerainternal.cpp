#include <glm/gtx/transform.hpp>

#include "world.h"
#include "engine.h"
#include "gizmos.h"
#include "profiler.h"
#include "gizmospainter.h"
#include "geometryutility.h"

#include "tools/math2.h"
#include "internal/async/guard.h"
#include "internal/entities/camerainternal.h"

static Camera main_;
Camera Camera::GetMain() { return main_; }
void Camera::SetMain(Camera value) { main_ = value; }

CameraInternal::CameraInternal()
	: EntityInternal(ObjectType::Camera), depth_(0), traitsReady_(false)
	 /*, gbuffer_(nullptr) */{
	culling_ = MEMORY_NEW(Culling, this);
	cullingThread_ = new ZThread::Thread(culling_);

	rendering_ = MEMORY_NEW(Rendering, &p_);// , this);

	traits0_ = MEMORY_NEW(RenderableTraits, &p_);
	traits1_ = MEMORY_NEW(RenderableTraits, &p_);

	Engine::instance()->AddFrameEventListener(this);
	Screen::instance()->AddScreenSizeChangedListener(this);

	SetAspect((float)Screen::instance()->GetWidth() / Screen::instance()->GetHeight());
}

CameraInternal::~CameraInternal() {
	//MEMORY_DELETE(gbuffer_);
	CancelThreads();

	MEMORY_DELETE(traits0_);
	MEMORY_DELETE(traits1_);

	MEMORY_DELETE(rendering_);
	Engine::instance()->RemoveFrameEventListener(this);
	Screen::instance()->RemoveScreenSizeChangedListener(this);
}

void CameraInternal::OnBeforeWorldDestroyed() {
	CancelThreads();
	Camera::SetMain(nullptr);
}

void CameraInternal::CancelThreads() {
	if (cullingThread_ != nullptr) {
		culling_->Stop();
		cullingThread_->wait();
		MEMORY_DELETE(cullingThread_);
		cullingThread_ = nullptr;
	}
}

void CameraInternal::SetDepth(int value) {
	if (depth_ != value) {
		depth_ = value;
		CameraDepthChangedEventPointer e = NewWorldEvent<CameraDepthChangedEventPointer>();
		e->entity = SharedThis();
		World::instance()->FireEvent(e);
	}
}

void CameraInternal::Render() {
	if (!culling_->IsWorking()) {
		traits1_->Clear();
		culling_->Cull(GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());
	}

	if (traitsReady_) {
		RenderingMatrices matrices;
		matrices.projParams = glm::vec4(GetNearClipPlane(), GetFarClipPlane(), GetAspect(), tanf(GetFieldOfView() / 2));
		matrices.cameraPos = GetTransform()->GetPosition();
		matrices.projectionMatrix = GetProjectionMatrix();
		matrices.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
		rendering_->Render(traits0_->GetPipelines(), matrices);
	}
	else {
		// SUEDE TODO: first frame not ready.
	}
}

void CameraInternal::OnScreenSizeChanged(uint width, uint height) {
	rendering_->Resize(width, height);

	float aspect = (float)width / height;
	if (!Math::Approximately(aspect, GetAspect())) {
		SetAspect(aspect);
	}
}

void CameraInternal::OnProjectionMatrixChanged() {
	GeometryUtility::CalculateFrustumPlanes(planes_, GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());
}

void CameraInternal::OnCullingFinished() {
	RenderingMatrices matrices;
	matrices.cameraPos = GetTransform()->GetPosition();
	matrices.projectionMatrix = GetProjectionMatrix();
	matrices.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();

	{
		ZTHREAD_LOCK_SCOPE(visibleEntitiesMutex_);
		visibleEntities_ = culling_->GetEntities();
	}

	traits1_->Traits(visibleEntities_, matrices);

	std::swap(traits0_, traits1_);
	traitsReady_ = true;
}

// void CameraInternal::OnRenderingFinished() {
// }

int CameraInternal::GetFrameEventQueue() {
	return IsMainCamera() ? std::numeric_limits<int>::max() : FrameEventListener::GetFrameEventQueue();
}

void CameraInternal::OnFrameLeave() {
	if (IsMainCamera()) {
		RenderTexture target = RenderTexture::GetDefault();
		target->BindWrite(GetRect());
		OnDrawGizmos();
		target->Unbind();
	}
}

bool CameraInternal::IsMainCamera() const {
	return Camera::GetMain().get() == this;
}

void CameraInternal::SetRect(const Rect& value) {
	if (p_.normalizedRect != value) {
		p_.normalizedRect = value;
		rendering_->ClearRenderTextures();
	}
}

void CameraInternal::GetVisibleEntities(std::vector<Entity>& entities) {
	ZTHREAD_LOCK_SCOPE(visibleEntitiesMutex_);
	entities = visibleEntities_;
}

glm::vec3 CameraInternal::WorldToScreenPoint(const glm::vec3& position) {
	glm::ivec4 viewport;
	GL::GetIntegerv(GL_VIEWPORT, (GLint*)&viewport);
	return glm::project(position, GetTransform()->GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
}

glm::vec3 CameraInternal::ScreenToWorldPoint(const glm::vec3& position) {
	glm::ivec4 viewport;
	GL::GetIntegerv(GL_VIEWPORT, (GLint*)&viewport);
	return glm::unProject(position, GetTransform()->GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
}

Texture2D CameraInternal::Capture() {
	uint alignment = 4;
	std::vector<uchar> data;
	Framebuffer0::Get()->ReadBuffer(data, &alignment);

	Texture2D texture = NewTexture2D();
	const glm::uvec4& viewport = Framebuffer0::Get()->GetViewport();
	texture->Create(TextureFormat::Rgb, &data[0], ColorStreamFormat::Rgb, viewport.z, viewport.w, alignment);

	return texture;
}

void CameraInternal::OnDrawGizmos() {
	for (int i = 0; i < gizmosPainters_.size(); ++i) {
		gizmosPainters_[i]->OnDrawGizmos();
	}

	Gizmos::instance()->Flush();
}
