#include "camerainternal.h"

#include <glm/gtx/transform.hpp>

#include "world.h"
#include "engine.h"
#include "gizmos.h"
#include "profiler.h"
#include "gizmospainter.h"
#include "geometryutility.h"

#include "tools/math2.h"
#include "internal/async/guard.h"

static Camera main_;
Camera Camera::main() { return main_; }
void Camera::main(Camera value) { main_ = value; }

void Camera::OnPreRender() {
	Framebuffer0::Get()->SetViewport(0, 0, Screen::instance()->GetWidth(), Screen::instance()->GetHeight());
	Framebuffer0::Get()->SetClearDepth(1);
	Framebuffer0::Get()->SetClearStencil(1);
	Framebuffer0::Get()->SetClearColor(Color::black);
	Framebuffer0::Get()->Clear(FramebufferClearMaskColorDepthStencil);
}

void Camera::OnPostRender() {
	RenderTexture target = RenderTexture::GetDefault();
	target->BindWrite(main()->GetRect());

	for (std::shared_ptr<GizmosPainter>& painter : World::instance()->GetComponents<GizmosPainter>()) {
		painter->OnDrawGizmos();
	}

	target->Unbind();
}

SUEDE_DEFINE_COMPONENT(ICamera, IComponent)

CameraInternal::CameraInternal()
	: ComponentInternal(ObjectType::Camera), depth_(0), traitsReady_(false)
	 /*, gbuffer_(nullptr) */{
	culling_ = MEMORY_NEW(Culling, this);
	cullingThread_ = new ZThread::Thread(culling_);

	rendering_ = MEMORY_NEW(Rendering, &p_);// , this);

	traits0_ = MEMORY_NEW(RenderableTraits, &p_);
	traits1_ = MEMORY_NEW(RenderableTraits, &p_);

	Screen::instance()->AddScreenSizeChangedListener(this);

	SetAspect((float)Screen::instance()->GetWidth() / Screen::instance()->GetHeight());
}

CameraInternal::~CameraInternal() {
	//MEMORY_DELETE(gbuffer_);
	CancelThreads();

	MEMORY_DELETE(traits0_);
	MEMORY_DELETE(traits1_);

	MEMORY_DELETE(rendering_);
	Screen::instance()->RemoveScreenSizeChangedListener(this);
}

void CameraInternal::OnBeforeWorldDestroyed() {
	CancelThreads();
	Camera::main(nullptr);
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
		e->component = SharedThis();
		World::instance()->FireEvent(e);
	}
}

void CameraInternal::Render() {
	if (!IsValidViewportRect()) {
		return;
	}

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
		ZTHREAD_LOCK_SCOPE(visibleGameObjectsMutex_);
		visibleGameObjects_ = culling_->GetGameObjects();
	}

	traits1_->Traits(visibleGameObjects_, matrices);

	std::swap(traits0_, traits1_);
	traitsReady_ = true;
}

// void CameraInternal::OnRenderingFinished() {
// }

bool CameraInternal::IsValidViewportRect() {
	const Rect& r = p_.normalizedRect;
	if (r.GetXMin() >= 1 || r.GetYMin() >= 1) { return false; }
	if (r.GetWidth() <= 0 || r.GetHeight() <= 0) { return false; }

	return true;
}

void CameraInternal::SetRect(const Rect& value) {
	p_.normalizedRect = value;
}

void CameraInternal::GetVisibleGameObjects(std::vector<GameObject>& gameObjects) {
	ZTHREAD_LOCK_SCOPE(visibleGameObjectsMutex_);
	gameObjects = visibleGameObjects_;
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
