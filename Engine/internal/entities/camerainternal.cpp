#include <glm/gtx/transform.hpp>

#include "world.h"
#include "engine.h"
#include "gizmos.h"
#include "gizmospainter.h"
#include "debug/profiler.h"
#include "geometryutility.h"
#include "internal/base/framebuffer.h"
#include "internal/entities/camerainternal.h"

CameraInternal::CameraInternal()
	: EntityInternal(ObjectTypeCamera), depth_(0)
	 /*, gbuffer_(nullptr) */{
	rendering_ = MEMORY_CREATE(Rendering);

	Engine::AddFrameEventListener(this);
	Screen::AddScreenSizeChangedListener(this);

	SetAspect((float)Screen::GetWidth() / Screen::GetHeight());
}

CameraInternal::~CameraInternal() {
	//MEMORY_RELEASE(gbuffer_);
	MEMORY_RELEASE(rendering_);
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
	Rendering::Matrices matrix;
	matrix.position = GetTransform()->GetPosition();
	matrix.projectionMatrix = GetProjectionMatrix();
	matrix.worldToCameraMatrix = GetTransform()->GetWorldToLocalMatrix();
	rendering_->Render(matrix);
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
	rendering_->SetRect(value);
}

const Rect& CameraInternal::GetRect() const {
	return rendering_->GetRect();
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
