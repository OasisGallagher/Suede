#include "camerainternal.h"

#include <glm/gtx/transform.hpp>

#include "world.h"
#include "engine.h"
#include "gizmos.h"
#include "profiler.h"
#include "gizmospainter.h"
#include "geometryutility.h"

#include "time2.h"
#include "tools/math2.h"

#include "internal/async/async.h"
#include "internal/rendering/uniformbuffermanager.h"

ICamera::ICamera() : IComponent(MEMORY_NEW(CameraInternal, this)) {}
void ICamera::SetDepth(int value) { _suede_dptr()->SetDepth(value); }
int ICamera::GetDepth() const { return _suede_dptr()->GetDepth(); }
bool ICamera::GetPerspective() const { return _suede_dptr()->GetPerspective(); }
void ICamera::SetPerspective(bool value) { _suede_dptr()->SetPerspective(value); }
float ICamera::GetOrthographicSize() const { return _suede_dptr()->GetOrthographicSize(); }
void ICamera::SetOrthographicSize(float value) { _suede_dptr()->SetOrthographicSize(value); }
void ICamera::SetClearType(ClearType value) { _suede_dptr()->SetClearType(value); }
ClearType ICamera::GetClearType() const { return _suede_dptr()->GetClearType(); }
void ICamera::SetRenderPath(RenderPath value) { _suede_dptr()->SetRenderPath(value); }
RenderPath ICamera::GetRenderPath() const { return _suede_dptr()->GetRenderPath(); }
void ICamera::SetDepthTextureMode(DepthTextureMode value) { _suede_dptr()->SetDepthTextureMode(value); }
DepthTextureMode ICamera::GetDepthTextureMode() const { return _suede_dptr()->GetDepthTextureMode(); }
void ICamera::SetClearColor(const Color& value) { _suede_dptr()->SetClearColor(value); }
Color ICamera::GetClearColor() const { return _suede_dptr()->GetClearColor(); }
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
void ICamera::Render() { _suede_dptr()->Render(); }
void ICamera::OnBeforeWorldDestroyed() { _suede_dptr()->OnBeforeWorldDestroyed(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Camera, Component)

static Camera main_;
Camera CameraUtility::GetMain() { return main_; }
void CameraUtility::SetMain(Camera value) { main_ = value; }

void CameraUtility::OnPreRender() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::GetRealTimeSinceStartup();
	p.time.y = Time::GetDeltaTime();
	Rendering::GetUniformBufferManager()->Update(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));

	RenderTextureUtility::GetDefault()->Clear(Rect(0, 0, 1, 1), Color::black, 1);
}

void CameraUtility::OnPostRender() {
	RenderTextureUtility::GetDefault()->BindWrite(GetMain()->GetRect());

	for (GizmosPainter painter : World::GetComponents<GizmosPainter>()) {
		painter->OnDrawGizmos();
	}

	RenderTextureUtility::GetDefault()->Unbind();
}

CameraInternal::CameraInternal(ICamera* self)
	: ComponentInternal(self, ObjectType::Camera), depth_(0), traitsReady_(false) /*, gbuffer_(nullptr) */{
	p_ = MEMORY_NEW(RenderingParameters);

	culling_ = MEMORY_NEW(Culling, this);
	cullingBuffer_ = new ITexture2D();
	cullingThread_ = MEMORY_NEW(ZThread::Thread, culling_);

	rendering_ = MEMORY_NEW(Rendering, p_);

	traits0_ = MEMORY_NEW(RenderableTraits, p_);
	traits1_ = MEMORY_NEW(RenderableTraits, p_);

	Screen::AddScreenSizeChangedListener(this);

	SetAspect((float)Screen::GetWidth() / Screen::GetHeight());
}

CameraInternal::~CameraInternal() {
	//MEMORY_DELETE(gbuffer_);
	CancelThreads();

	MEMORY_DELETE(p_);

	MEMORY_DELETE(traits0_);
	MEMORY_DELETE(traits1_);

	MEMORY_DELETE(rendering_);

	Screen::RemoveScreenSizeChangedListener(this);
}

void CameraInternal::Awake() {
	p_->camera = GetGameObject().get();
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

void CameraInternal::SetDepth(int value) {
	if (depth_ != value) {
		depth_ = value;
		CameraDepthChangedEventPtr e = NewWorldEvent<CameraDepthChangedEventPtr>();
		e->component = _suede_self();
		World::FireEvent(e);
	}
}

#include "graphics.h"

void CameraInternal::Render() {
	if (!IsValidViewportRect()) {
		return;
	}

	Transform transform = GetTransform();
	if (!culling_->IsWorking()) {
		traits1_->Clear();
		culling_->Cull(transform->GetPosition(), transform->GetForward(), GetFarClipPlane(), GetProjectionMatrix() * transform->GetWorldToLocalMatrix());
	}

	if (traitsReady_) {
		RenderingMatrices matrices;
		matrices.projParams = glm::vec4(GetNearClipPlane(), GetFarClipPlane(), GetAspect(), Math::Tan(GetFieldOfView() / 2));
		matrices.cameraPos = transform->GetPosition();
		matrices.projectionMatrix = GetProjectionMatrix();
		matrices.worldToCameraMatrix = transform->GetWorldToLocalMatrix();
		rendering_->Render(traits0_->GetPipelines(), matrices);

		/*TexelMap texels;
		culling_->GetCullingBuffer(texels);
		cullingBuffer_->SetPixels(texels.textureFormat, &texels.data[0], texels.colorStreamFormat, texels.width, texels.height, texels.alignment);

		Graphics::Blit(cullingBuffer_, RenderTextureUtility::GetDefault(), Rect(0, 0, 1, 1), Rect(0, 0, 0.32f, 0.32f / GetAspect()));
		*/
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
	const Rect& r = p_->normalizedRect;
	if (r.GetXMin() >= 1 || r.GetYMin() >= 1) { return false; }
	if (r.GetWidth() <= 0 || r.GetHeight() <= 0) { return false; }

	return true;
}

void CameraInternal::SetRect(const Rect& value) {
	p_->normalizedRect = value;
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
