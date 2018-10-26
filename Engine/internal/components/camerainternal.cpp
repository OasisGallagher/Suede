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
void ICamera::SetDepth(int value) { _suede_dptr()->SetDepth(_shared_this(), value); }
int ICamera::GetDepth() { return _suede_dptr()->GetDepth(); }
bool ICamera::GetPerspective() const { return _suede_dptr()->GetPerspective(); }
void ICamera::SetPerspective(bool value) { _suede_dptr()->SetPerspective(value); }
float ICamera::GetOrthographicSize() const { return _suede_dptr()->GetOrthographicSize(); }
void ICamera::SetOrthographicSize(float value) { _suede_dptr()->SetOrthographicSize(value); }
void ICamera::SetClearType(ClearType value) { _suede_dptr()->SetClearType(value); }
ClearType ICamera::GetClearType() { return _suede_dptr()->GetClearType(); }
void ICamera::SetRenderPath(RenderPath value) { _suede_dptr()->SetRenderPath(value); }
RenderPath ICamera::GetRenderPath() { return _suede_dptr()->GetRenderPath(); }
void ICamera::SetDepthTextureMode(DepthTextureMode value) { _suede_dptr()->SetDepthTextureMode(value); }
DepthTextureMode ICamera::GetDepthTextureMode() { return _suede_dptr()->GetDepthTextureMode(); }
void ICamera::SetClearColor(const Color& value) { _suede_dptr()->SetClearColor(value); }
Color ICamera::GetClearColor() { return _suede_dptr()->GetClearColor(); }
void ICamera::SetTargetTexture(RenderTexture value) { _suede_dptr()->SetTargetTexture(value); }
RenderTexture ICamera::GetTargetTexture() { return _suede_dptr()->GetTargetTexture(); }
void ICamera::SetAspect(float value) { _suede_dptr()->SetAspect(value); }
float ICamera::GetAspect() const { return _suede_dptr()->GetAspect(); }
void ICamera::SetNearClipPlane(float value) { _suede_dptr()->SetNearClipPlane(value); }
float ICamera::GetNearClipPlane() const { return _suede_dptr()->GetNearClipPlane(); }
void ICamera::SetFarClipPlane(float value) { _suede_dptr()->SetFarClipPlane(value); }
float ICamera::GetFarClipPlane() const { return _suede_dptr()->GetFarClipPlane(); }
void ICamera::SetFieldOfView(float value) { _suede_dptr()->SetFieldOfView(value); }
float ICamera::GetFieldOfView() const { return _suede_dptr()->GetFieldOfView(); }
void ICamera::SetRect(const Rect& value) { _suede_dptr()->SetRect(value); }
const Rect& ICamera::GetRect() const { return _suede_dptr()->GetRect(); }
const glm::mat4& ICamera::GetProjectionMatrix() { return _suede_dptr()->GetProjectionMatrix(); }
void ICamera::GetVisibleGameObjects(std::vector<GameObject>& gameObjects) { return _suede_dptr()->GetVisibleGameObjects(gameObjects); }
glm::vec3 ICamera::WorldToScreenPoint(const glm::vec3& position) { return _suede_dptr()->WorldToScreenPoint(position); }
glm::vec3 ICamera::ScreenToWorldPoint(const glm::vec3& position) { return _suede_dptr()->ScreenToWorldPoint(position); }
Texture2D ICamera::Capture() { return _suede_dptr()->Capture(); }
void ICamera::Render() { _suede_dptr()->Render(); }
void ICamera::OnBeforeWorldDestroyed() { _suede_dptr()->OnBeforeWorldDestroyed(); }

static Camera main_;
Camera CameraUtility::GetMain() { return main_; }
void CameraUtility::SetMain(Camera value) { main_ = value; }

void CameraUtility::OnPreRender() {
	Framebuffer0::Get()->SetViewport(0, 0, Screen::instance()->GetWidth(), Screen::instance()->GetHeight());
	Framebuffer0::Get()->SetClearDepth(1);
	Framebuffer0::Get()->SetClearStencil(1);
	Framebuffer0::Get()->SetClearColor(Color::black);
	Framebuffer0::Get()->Clear(FramebufferClearMaskColorDepthStencil);
}

void CameraUtility::OnPostRender() {
	RenderTexture target = RenderTextureUtility::GetDefault();
	target->BindWrite(GetMain()->GetRect());

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
	cullingThread_ = MEMORY_NEW(ZThread::Thread, culling_);

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

void CameraInternal::Awake() {
	p_.camera = GetGameObject().get();
}

void CameraInternal::OnBeforeWorldDestroyed() {
	CancelThreads();
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
