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

ICamera::ICamera() : IComponent(MEMORY_NEW(CameraInternal)) {}
void ICamera::SetDepth(int value) { dptr()->SetDepth(SharedThis(), value); }
int ICamera::GetDepth() { return dptr()->GetDepth(); }
bool ICamera::GetPerspective() const { return dptr()->GetPerspective(); }
void ICamera::SetPerspective(bool value) { dptr()->SetPerspective(value); }
float ICamera::GetOrthographicSize() const { return dptr()->GetOrthographicSize(); }
void ICamera::SetOrthographicSize(float value) { dptr()->SetOrthographicSize(value); }
void ICamera::SetClearType(ClearType value) { dptr()->SetClearType(value); }
ClearType ICamera::GetClearType() { return dptr()->GetClearType(); }
void ICamera::SetRenderPath(RenderPath value) { dptr()->SetRenderPath(value); }
RenderPath ICamera::GetRenderPath() { return dptr()->GetRenderPath(); }
void ICamera::SetDepthTextureMode(DepthTextureMode value) { dptr()->SetDepthTextureMode(value); }
DepthTextureMode ICamera::GetDepthTextureMode() { return dptr()->GetDepthTextureMode(); }
void ICamera::SetClearColor(const Color& value) { dptr()->SetClearColor(value); }
Color ICamera::GetClearColor() { return dptr()->GetClearColor(); }
void ICamera::SetTargetTexture(RenderTexture value) { dptr()->SetTargetTexture(value); }
RenderTexture ICamera::GetTargetTexture() { return dptr()->GetTargetTexture(); }
void ICamera::SetAspect(float value) { dptr()->SetAspect(value); }
float ICamera::GetAspect() const { return dptr()->GetAspect(); }
void ICamera::SetNearClipPlane(float value) { dptr()->SetNearClipPlane(value); }
float ICamera::GetNearClipPlane() const { return dptr()->GetNearClipPlane(); }
void ICamera::SetFarClipPlane(float value) { dptr()->SetFarClipPlane(value); }
float ICamera::GetFarClipPlane() const { return dptr()->GetFarClipPlane(); }
void ICamera::SetFieldOfView(float value) { dptr()->SetFieldOfView(value); }
float ICamera::GetFieldOfView() const { return dptr()->GetFieldOfView(); }
void ICamera::SetRect(const Rect& value) { dptr()->SetRect(value); }
const Rect& ICamera::GetRect() const { return dptr()->GetRect(); }
const glm::mat4& ICamera::GetProjectionMatrix() { return dptr()->GetProjectionMatrix(); }
void ICamera::GetVisibleGameObjects(std::vector<GameObject>& gameObjects) { return dptr()->GetVisibleGameObjects(gameObjects); }
glm::vec3 ICamera::WorldToScreenPoint(const glm::vec3& position) { return dptr()->WorldToScreenPoint(position); }
glm::vec3 ICamera::ScreenToWorldPoint(const glm::vec3& position) { return dptr()->ScreenToWorldPoint(position); }
Texture2D ICamera::Capture() { return dptr()->Capture(); }
void ICamera::Render() { dptr()->Render(); }
void ICamera::OnBeforeWorldDestroyed() { dptr()->OnBeforeWorldDestroyed(); }

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

void CameraInternal::SetDepth(Camera self, int value) {
	if (depth_ != value) {
		depth_ = value;
		CameraDepthChangedEventPtr e = NewWorldEvent<CameraDepthChangedEventPtr>();
		e->component = self;
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
		// Debug::Log("Waiting for first frame...");
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
