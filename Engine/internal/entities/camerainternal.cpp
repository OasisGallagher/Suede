#include <glm/gtx/transform.hpp>

#include "world.h"
#include "engine.h"
#include "gizmos.h"
#include "profiler.h"
#include "gizmospainter.h"
#include "geometryutility.h"

#include "internal/entities/camerainternal.h"

CameraInternal::CameraInternal()
	: EntityInternal(ObjectTypeCamera), depth_(0), __isCulling(false), currentTraits_(nullptr)
	 /*, gbuffer_(nullptr) */{
	culling_ = MEMORY_CREATE(Culling, this);
	cullingThread_ = new ZThread::Thread(culling_);

	traits0_ = MEMORY_CREATE(RenderableTraits, &p_);
	traits1_ = MEMORY_CREATE(RenderableTraits, &p_);

	rendering_ = MEMORY_CREATE(Rendering, &p_);// , this);

	Engine::get()->AddFrameEventListener(this);
	Screen::get()->AddScreenSizeChangedListener(this);

	SetAspect((float)Screen::get()->GetWidth() / Screen::get()->GetHeight());
}

CameraInternal::~CameraInternal() {
	//MEMORY_RELEASE(gbuffer_);
	CancelThreads();

	MEMORY_RELEASE(traits0_);
	MEMORY_RELEASE(traits1_);

	MEMORY_RELEASE(rendering_);
	Engine::get()->RemoveFrameEventListener(this);
	Screen::get()->RemoveScreenSizeChangedListener(this);
}

void CameraInternal::OnBeforeWorldDestroyed() {
	CancelThreads();
}

void CameraInternal::CancelThreads() {
	if (cullingThread_ != nullptr) {
		culling_->Stop();
		cullingThread_->wait();
		MEMORY_RELEASE(cullingThread_);
		cullingThread_ = nullptr;
	}
}

void CameraInternal::SetDepth(int value) {
	if (depth_ != value) {
		depth_ = value;
		CameraDepthChangedEventPointer e = NewWorldEvent<CameraDepthChangedEventPointer>();
		e->entity = SharedThis();
		WorldInstance()->FireEvent(e);
	}
}

void CameraInternal::Update() {
}

void CameraInternal::Render() {
	if (!culling_->IsWorking()) {
		culling_->Cull(GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());
	}

	if (currentTraits_ != nullptr) {
		RenderingMatrices matrices;
		matrices.position = GetTransform()->GetPosition();
		matrices.projectionMatrix = GetProjectionMatrix();
		matrices.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
		rendering_->Render(currentTraits_->GetPipelines(), matrices);
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
	RenderableTraits* free = (currentTraits_ != traits0_) ? traits0_ : traits1_;

	RenderingMatrices matrices;
	matrices.position = GetTransform()->GetPosition();
	matrices.projectionMatrix = GetProjectionMatrix();
	matrices.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
	free->Traits(culling_->GetEntities(), matrices);

	currentTraits_ = free;
}

// void CameraInternal::OnRenderingFinished() {
// }

int CameraInternal::GetFrameEventQueue() {
	return IsMainCamera() ? std::numeric_limits<int>::max() : FrameEventListener::GetFrameEventQueue();
}

void CameraInternal::OnFrameLeave() {
	if (IsMainCamera()) {
		RenderTexture target = WorldInstance()->GetScreenRenderTarget();

		target->BindWrite(GetRect());
		OnDrawGizmos();
		target->Unbind();
	}
}

bool CameraInternal::IsMainCamera() const {
	return WorldInstance()->GetMainCamera().get() == this;
}

void CameraInternal::SetRect(const Rect& value) {
	if (p_.normalizedRect != value) {
		p_.normalizedRect = value;
		rendering_->ClearRenderTextures();
	}
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
	texture->Load(TextureFormatRgb, &data[0], ColorStreamFormatRgb, viewport.z, viewport.w, alignment);

	return texture;
}

void CameraInternal::OnDrawGizmos() {
	for (int i = 0; i < gizmosPainters_.size(); ++i) {
		gizmosPainters_[i]->OnDrawGizmos();
	}

	Gizmos::get()->Flush();
}
