#include "camerainternal.h"

#include "engine.h"
#include "scene.h"
#include "gizmos.h"
#include "profiler.h"
#include "gizmospainter.h"
#include "geometryutility.h"

#include "math/mathf.h"
#include "internal/rendering/renderingcontext.h"

static Camera* main_;
Camera* Camera::GetMain() { return main_; }
void Camera::SetMain(Camera* value) { main_ = value; }

Camera::Camera() : Component(new CameraInternal) {}
Camera::~Camera() { if (main_ == this) { main_ = nullptr; } }

void Camera::SetDepth(int value) { _suede_dptr()->SetDepth(value); }
int Camera::GetDepth() const { return _suede_dptr()->GetDepth(); }
void Camera::SetCullingMask(int value) { _suede_dptr()->SetCullingMask(value); }
int Camera::GetCullingMask() const { return _suede_dptr()->GetCullingMask(); }
bool Camera::GetPerspective() const { return _suede_dptr()->GetPerspective(); }
void Camera::SetPerspective(bool value) { _suede_dptr()->SetPerspective(value); }
float Camera::GetOrthographicSize() const { return _suede_dptr()->GetOrthographicSize(); }
void Camera::SetOrthographicSize(float value) { _suede_dptr()->SetOrthographicSize(value); }
void Camera::SetClearType(ClearType value) { _suede_dptr()->SetClearType(value); }
ClearType Camera::GetClearType() const { return _suede_dptr()->GetClearType(); }
void Camera::SetRenderPath(RenderPath value) { _suede_dptr()->SetRenderPath(value); }
RenderPath Camera::GetRenderPath() const { return _suede_dptr()->GetRenderPath(); }
void Camera::SetDepthTextureMode(DepthTextureMode value) { _suede_dptr()->SetDepthTextureMode(value); }
DepthTextureMode Camera::GetDepthTextureMode() const { return _suede_dptr()->GetDepthTextureMode(); }
void Camera::SetClearColor(const Color& value) { _suede_dptr()->SetClearColor(value); }
Color Camera::GetClearColor() const { return _suede_dptr()->GetClearColor(); }
void Camera::SetTargetTexture(RenderTexture* value) { _suede_dptr()->SetTargetTexture(value); }
RenderTexture* Camera::GetTargetTexture() { return _suede_dptr()->GetTargetTexture(); }
void Camera::SetAspect(float value) { _suede_dptr()->SetAspect(value); }
float Camera::GetAspect() const { return _suede_dptr()->GetAspect(); }
void Camera::SetNearClipPlane(float value) { _suede_dptr()->SetNearClipPlane(value); }
float Camera::GetNearClipPlane() const { return _suede_dptr()->GetNearClipPlane(); }
void Camera::SetFarClipPlane(float value) { _suede_dptr()->SetFarClipPlane(value); }
float Camera::GetFarClipPlane() const { return _suede_dptr()->GetFarClipPlane(); }
void Camera::SetFieldOfView(float value) { _suede_dptr()->SetFieldOfView(value); }
float Camera::GetFieldOfView() const { return _suede_dptr()->GetFieldOfView(); }
void Camera::SetRect(const Rect& value) { _suede_dptr()->SetRect(value); }
const Rect& Camera::GetRect() const { return _suede_dptr()->GetRect(); }
const Matrix4& Camera::GetProjectionMatrix() { return _suede_dptr()->GetProjectionMatrix(); }
void Camera::GetVisibleGameObjects(std::vector<GameObject*>& gameObjects) { return _suede_dptr()->GetVisibleGameObjects(gameObjects); }
Vector3 Camera::WorldToScreenPoint(const Vector3& position) { return _suede_dptr()->WorldToScreenPoint(position); }
Vector3 Camera::ScreenToWorldPoint(const Vector3& position) { return _suede_dptr()->ScreenToWorldPoint(position); }
void Camera::Render() { _suede_dptr()->Render(); }
ref_ptr<Texture2D> Camera::Capture() { return _suede_dptr()->Capture(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Camera, Component)

void Camera::OnPreRender() {
	RenderTexture::GetDefault()->Clear(Rect::unit, Color::black, 1, 1);
}

void Camera::OnPostRender() {
	bool needUnbind = false;
	for (GizmosPainter* painter : GetMain()->GetGameObject()->GetScene()->GetComponents<GizmosPainter>()) {
		if (!painter->GetActiveAndEnabled()) { continue; }
		if (!needUnbind) {
			needUnbind = true;
			RenderTexture::GetDefault()->BindWrite(GetMain()->GetRect());
		}

		painter->OnDrawGizmos();
	}

	if (needUnbind) {
		RenderTexture::GetDefault()->Unbind();
	}
}

CameraInternal::CameraInternal() : ComponentInternal(ObjectType::Camera) {
	renderingThread_ = context_->GetRenderingThread();

	pipelineBuilder_ = new PipelineBuilder(context_);
	frontPipelines_ = new RenderingPipelines(context_);
	backPipelines_ = new RenderingPipelines(context_);

	Screen::sizeChanged.subscribe(this, &CameraInternal::OnScreenSizeChanged);
	
	cullingTask_ = new CullingTask(context_);
	cullingTask_->finished.subscribe(this, &CameraInternal::OnCullingFinished);

	SetAspect((float)Screen::GetWidth() / Screen::GetHeight());
}

CameraInternal::~CameraInternal() {
	delete pipelineBuilder_;
	delete frontPipelines_;
	delete backPipelines_;

	Screen::sizeChanged.unsubscribe(this);
}

void CameraInternal::Awake() {
	transform_ = GetTransform();
}

void CameraInternal::RenderFrame() {
	UpdateFrameState();

	cullingTask_->SetCullingMask(cullingMask_);
	cullingTask_->SetWorldToClipMatrix(GetProjectionMatrix() * transform_->GetWorldToLocalMatrix());

	backPipelines_->Clear();
	context_->GetCullingThread()->AddTask(cullingTask_.get());

	if (pipelineReady_) {
		RenderingMatrices matrices;
		matrices.cameraPos = transform_->GetPosition();
		matrices.projParams = Vector4(GetNearClipPlane(), GetFarClipPlane(), GetAspect(), tanf(GetFieldOfView() / 2));
		matrices.projectionMatrix = GetProjectionMatrix();
		matrices.worldToCameraMatrix = transform_->GetWorldToLocalMatrix();

		renderingThread_->Render(frontPipelines_, matrices);
	}
	else {
		Debug::Log("Waiting for first frame...");
	}
}

void CameraInternal::UpdateFrameState() {
	FrameState* fs = context_->GetFrameState();
	fs->camera = gameObject_;
	fs->normalizedRect = normalizedRect_;
	fs->clearType = clearType_;
	fs->clearColor = clearColor_;
	fs->depthTextureMode = depthTextureMode_;
	fs->renderPath = renderPath_;
	fs->targetTexture = targetTexture_.get();
}

void CameraInternal::Render() {
	if (IsValidViewportRect()) {
		RenderFrame();
	}
}

void CameraInternal::OnCullingFinished() {
	RenderingMatrices matrices;
	matrices.cameraPos = transform_->GetPosition();
	matrices.projectionMatrix = GetProjectionMatrix();
	matrices.worldToCameraMatrix = transform_->GetWorldToLocalMatrix();

	{
		std::lock_guard<std::mutex> lock(visibleGameObjectsMutex_);
		visibleGameObjects_ = cullingTask_->GetGameObjects();
	}

	pipelineBuilder_->Build(backPipelines_, visibleGameObjects_, matrices);

	std::swap(frontPipelines_, backPipelines_);
	pipelineReady_ = true;
}

void CameraInternal::OnScreenSizeChanged(uint width, uint height) {
	float aspect = (float)width / height;
	if (!Mathf::Approximately(aspect, GetAspect())) {
		SetAspect(aspect);
	}
}

bool CameraInternal::IsValidViewportRect() {
	return normalizedRect_.GetXMin() < 1 && normalizedRect_.GetYMin() < 1
		&& normalizedRect_.GetWidth() > 0 && normalizedRect_.GetHeight() > 0;
}

void CameraInternal::GetVisibleGameObjects(std::vector<GameObject*>& gameObjects) {
	std::lock_guard<std::mutex> lock(visibleGameObjectsMutex_);
	gameObjects = visibleGameObjects_;
}

Vector3 CameraInternal::WorldToScreenPoint(const Vector3& position) {
	IVector4 viewport;
	context_->GetIntegerv(GL_VIEWPORT, (int*)&viewport);
	return Matrix4::Project(position, transform_->GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
}

Vector3 CameraInternal::ScreenToWorldPoint(const Vector3& position) {
	IVector4 viewport;
	context_->GetIntegerv(GL_VIEWPORT, (int*)&viewport);
	return Matrix4::Unproject(position, transform_->GetWorldToLocalMatrix(), GetProjectionMatrix(), viewport);
}

ref_ptr<Texture2D> CameraInternal::Capture() {
	uint alignment = 4;
	std::vector<uchar> data;
	Framebuffer::GetDefault()->ReadBuffer(data, &alignment);

	Texture2D* texture = new Texture2D();
	const IVector4& viewport = Framebuffer::GetDefault()->GetViewport();
	texture->Create(TextureFormat::Rgb, &data[0], ColorStreamFormat::Rgb, (uint)viewport.z, (uint)viewport.w, alignment);

	return texture;
}
