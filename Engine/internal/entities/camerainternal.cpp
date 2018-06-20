#include <glm/gtx/transform.hpp>

#include "world.h"
#include "engine.h"
#include "gizmos.h"
#include "gizmospainter.h"
#include "debug/profiler.h"
#include "geometryutility.h"

#include "internal/entities/camerainternal.h"

CameraInternal::CameraInternal()
	: EntityInternal(ObjectTypeCamera), depth_(0), __isCulling(false), currentTraits_(nullptr)
	 /*, gbuffer_(nullptr) */{
	cullingThread_ = MEMORY_CREATE(CullingThread, this);

	traits0_ = MEMORY_CREATE(RenderableTraits, &p_);
	traits1_ = MEMORY_CREATE(RenderableTraits, &p_);

	renderingThread_ = MEMORY_CREATE(RenderingThread, &p_);// , this);

	executor_.execute(cullingThread_);
	//executor_.execute(renderingThread_);

	Engine::AddFrameEventListener(this);
	Screen::AddScreenSizeChangedListener(this);

	SetAspect((float)Screen::GetWidth() / Screen::GetHeight());
}

CameraInternal::~CameraInternal() {
	//MEMORY_RELEASE(gbuffer_);
	cullingThread_->Stop();
	executor_.wait();

	MEMORY_RELEASE(traits0_);
	MEMORY_RELEASE(traits1_);

	MEMORY_RELEASE(renderingThread_);
	Engine::RemoveFrameEventListener(this);
	Screen::RemoveScreenSizeChangedListener(this);
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
	if (!cullingThread_->IsWorking()) {
		cullingThread_->Cull(GetProjectionMatrix() * GetTransform()->GetWorldToLocalMatrix());
	}

	if (currentTraits_ != nullptr) {
		RenderingMatrices matrices;
		matrices.position = GetTransform()->GetPosition();
		matrices.projectionMatrix = GetProjectionMatrix();
		matrices.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
		renderingThread_->Render(currentTraits_->GetPipelines(), matrices);
	}
}

void CameraInternal::OnScreenSizeChanged(uint width, uint height) {
	renderingThread_->Resize(width, height);

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
	free->Traits(cullingThread_->GetEntities(), matrices);

	currentTraits_ = free;
}

// void CameraInternal::OnRenderingFinished() {
// }

int CameraInternal::GetFrameEventQueue() {
	return IsMainCamera() ? std::numeric_limits<int>::max() : FrameEventListener::GetFrameEventQueue();
}

void CameraInternal::OnFrameLeave() {
	if (IsMainCamera()) {
		WorldInstance()->GetScreenRenderTarget()->BindWrite(GetRect());
		OnDrawGizmos();
		WorldInstance()->GetScreenRenderTarget()->Unbind();
	}
}

bool CameraInternal::IsMainCamera() const {
	return WorldInstance()->GetMainCamera().get() == this;
}

void CameraInternal::SetRect(const Rect& value) {
	if (p_.normalizedRect != value) {
		p_.normalizedRect = value;
		renderingThread_->ClearRenderTextures();
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
	// TODO: VIEWPORT.
	std::vector<uchar> data;
	Framebuffer0::Get()->ReadBuffer(data);

	Texture2D texture = NewTexture2D();
	const glm::uvec4& viewport = Framebuffer0::Get()->GetViewport();
	texture->Load(TextureFormatRgb, &data[0], ColorStreamFormatRgb, viewport.z, viewport.w);

	return texture;
}

void CameraInternal::OnDrawGizmos() {
	for (int i = 0; i < gizmosPainters_.size(); ++i) {
		gizmosPainters_[i]->OnDrawGizmos();
	}

	Gizmos::Flush();
}
