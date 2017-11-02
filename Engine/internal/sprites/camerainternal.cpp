#include <glm/gtc/matrix_transform.hpp>

#include "light.h"
#include "camera.h"
#include "variables.h"
#include "tools/debug.h"
#include "tools/mathf.h"
#include "internal/memory/factory.h"
#include "internal/file/imagecodec.h"
#include "internal/base/framebuffer.h"
#include "internal/resources/resources.h"
#include "internal/base/shaderinternal.h"
#include "internal/world/worldinternal.h"
#include "internal/misc/screeninternal.h"
#include "internal/base/textureinternal.h"
#include "internal/base/materialinternal.h"
#include "internal/base/rendererinternal.h"
#include "internal/misc/graphicsinternal.h"
#include "internal/sprites/camerainternal.h"

CameraInternal::CameraInternal() 
	: SpriteInternal(ObjectTypeCamera), clearType_(ClearTypeColor)
	, depth_(0), aspect_(1.3f), near_(1.f), far_(100.f)
	, fieldOfView_(3.141592f / 3.f), projection_(glm::perspective(fieldOfView_, aspect_, near_, far_))
	, pass_(RenderPassNone), fbRenderTexture_(nullptr), viewToShadowSpaceMatrix_(1) {
	CreateFramebuffers();
	CreateDepthRenderer();
	CreateShadowRenderer();
	glClearDepth(1);
}

CameraInternal::~CameraInternal() {
	Memory::Release(fb0_);
	Memory::Release(fbDepth_);
	Memory::Release(fbShadow_);
	Memory::Release(fbRenderTexture_);
}

void CameraInternal::SetClearColor(const glm::vec3 & value) {
	fb0_->SetClearColor(value);
}

glm::vec3 CameraInternal::GetClearColor() {
	return fb0_->GetClearColor();
}

void CameraInternal::SetAspect(float value) {
	if (!Mathf::Approximately(aspect_, value)) {
		aspect_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetNearClipPlane(float value) {
	if (!Mathf::Approximately(near_, value)) {
		near_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetFarClipPlane(float value) {
	if (!Mathf::Approximately(far_, value)) {
		far_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetFieldOfView(float value) {
	if (!Mathf::Approximately(near_, value)) {
		near_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void CameraInternal::SetRenderTexture(RenderTexture value) {
	if (value) {
		fbRenderTexture_->SetRenderTexture(value);
	}
	else {
		fbRenderTexture_->SetRenderTexture(renderTexture_);
	}
}

void CameraInternal::Update() {
	int w = screenInstance->GetContextWidth();
	int h = screenInstance->GetContextHeight();

	if (w != fb0_->GetWidth() || h != fb0_->GetHeight()) {
		OnContextSizeChanged(w, h);
	}

	UpdateSkybox();
}

void CameraInternal::Render() {
	std::vector<Sprite> sprites;
	if (!GetRenderableSprites(sprites)) {
		return;
	}

	//RenderDepthPass(sprites);

	Light forwardBase;
	std::vector<Light> forwardAdd;
	GetLights(forwardBase, forwardAdd);

	RenderShadowPass(sprites, forwardBase);

	//Renderer renderer = FactoryCreate(SurfaceRenderer);
	//Material material = FactoryCreate(Material);
	//Shader shader = FactoryCreate(Shader);
	//shader->Load("buildin/shaders/blit");
	//material->SetShader(shader);
	//renderer->AddMaterial(material);

	//fb0_->Bind();
	//graphicsInstance->Blit(fbShadow_->GetDepthTexture(), nullptr, renderer);
	//fb0_->Unbind();
	//return;
	
	Framebuffer0* active = GetActiveFramebuffer();
	active->Bind();

	if (forwardBase) {
		RenderForwardBase(sprites, forwardBase);
	}

	if (!forwardAdd.empty()) {
		RenderForwardAdd(sprites, forwardAdd);
	}

	OnPostRender();

	active->Unbind();

	pass_ = RenderPassNone;
}

Texture2D CameraInternal::Capture() {
	std::vector<unsigned char> data;
	fb0_->ReadBuffer(data);

	Texture2D tex = CREATE_OBJECT(Texture2D);
	tex->Load(&data[0], fb0_->GetWidth(), fb0_->GetHeight());

	return tex;
}

void CameraInternal::CreateFramebuffers() {
	int w = screenInstance->GetContextWidth();
	int h = screenInstance->GetContextHeight();

	fb0_ = Memory::Create<Framebuffer0>();
	fb0_->Create(w, h);

	fbDepth_ = Memory::Create<Framebuffer>();
	fbDepth_->Create(w, h);
	RenderTexture depthTexture = CREATE_OBJECT(RenderTexture);
	depthTexture->Load(RenderTextureFormatDepth, w, h);
	fbDepth_->SetDepthTexture(depthTexture);

	fbShadow_ = Memory::Create<Framebuffer>();
	fbShadow_->Create(w, h);
	RenderTexture shadowTexture = CREATE_OBJECT(RenderTexture);
	shadowTexture->Load(RenderTextureFormatShadow, w, h);
	fbShadow_->SetDepthTexture(shadowTexture);

	fbRenderTexture_ = Memory::Create<Framebuffer>();
	fbRenderTexture_->Create(w, h);
	renderTexture_ = CREATE_OBJECT(RenderTexture);
	renderTexture_->Load(RenderTextureFormatRgba, w, h);
	fbRenderTexture_->SetRenderTexture(renderTexture_);
	
	fbRenderTexture2_ = Memory::Create<Framebuffer>();
	fbRenderTexture2_->Create(w, h);
	renderTexture2_ = CREATE_OBJECT(RenderTexture);
	renderTexture2_->Load(RenderTextureFormatRgba, w, h);
	fbRenderTexture2_->SetRenderTexture(renderTexture2_);
}

void CameraInternal::CreateDepthRenderer() {
	depthRenderer_ = CREATE_OBJECT(SurfaceRenderer);
	Shader shader = CREATE_OBJECT(Shader);
	shader->Load("buildin/shaders/depth");

	Material material = CREATE_OBJECT(Material);
	material->SetShader(shader);

	depthRenderer_->AddMaterial(material);
	depthRenderer_->SetRenderState(Cull, Back);
	depthRenderer_->SetRenderState(DepthWrite, On);
	depthRenderer_->SetRenderState(DepthTest, LessEqual);
}

void CameraInternal::CreateShadowRenderer() {
	directionalLightShadowRenderer_ = CREATE_OBJECT(SurfaceRenderer);
	Shader shader = CREATE_OBJECT(Shader);
	shader->Load("buildin/shaders/directional_light_depth");

	Material material = CREATE_OBJECT(Material);
	material->SetShader(shader);

	directionalLightShadowRenderer_->AddMaterial(material);
	directionalLightShadowRenderer_->SetRenderState(Cull, Back);
	directionalLightShadowRenderer_->SetRenderState(DepthWrite, On);
	directionalLightShadowRenderer_->SetRenderState(DepthTest, LessEqual);
}

void CameraInternal::UpdateSkybox() {
	if (clearType_ != ClearTypeSkybox) { return; }
	Skybox skybox = skybox_;
	if (!skybox) {
		skybox = worldInstance->GetEnvironment()->GetSkybox();
	}

	if (!skybox) {
		Debug::LogError("skybox does not exist.");
		return;
	}

	skybox->SetPosition(GetPosition());
}

void CameraInternal::OnContextSizeChanged(int w, int h) {
	fb0_->Resize(w, h);
	fbDepth_->Resize(w, h);
	fbShadow_->Resize(w, h);
	fbRenderTexture_->Resize(w, h);
	fbRenderTexture2_->Resize(w, h);

	float aspect = (float)w / h;
	if (!Mathf::Approximately(aspect, GetAspect())) {
		SetAspect(aspect);
	}
}

Framebuffer0* CameraInternal::GetActiveFramebuffer() {
	Framebuffer0* active = nullptr;
	if (fbRenderTexture_->GetRenderTexture() != renderTexture_ || !postEffects_.empty()) {
		active = fbRenderTexture_;
	}
	else {
		active = fb0_;
	}

	return active;
}

void CameraInternal::SetForwardBaseLightParameter(const std::vector<Sprite>& sprites, Light light) {
	for (int i = 0; i < sprites.size(); ++i) {
		Renderer renderer = sprites[i]->GetRenderer();
		int materialCount = renderer->GetMaterialCount();
		for (int i = 0; i < materialCount; ++i) {
			Material material = renderer->GetMaterial(i);

			material->SetVector3(Variables::cameraPosition, GetPosition());
			material->SetVector3(Variables::ambientLightColor, worldInstance->GetEnvironment()->GetAmbientColor());
			material->SetVector3(Variables::lightColor, light->GetColor());
			material->SetVector3(Variables::lightPosition, light->GetPosition());
			material->SetVector3(Variables::lightDirection, light->GetRotation() * glm::vec3(0, 0, -1));
		}
	}
}

void CameraInternal::RenderForwardBase(const std::vector<Sprite>& sprites, Light light) {
	SetForwardBaseLightParameter(sprites, light);

	int from = 0;
	from = RenderBackgroundPass(sprites, from);
	from = RenderOpaquePass(sprites, from);
	from = RenderTransparentPass(sprites, from);
}

void CameraInternal::RenderShadowPass(const std::vector<Sprite>& sprites, Light light) {
	pass_ = RenderPassShadow;
	Assert(light->GetType() == ObjectTypeDirectionalLight);

	fbShadow_->Bind();

	glm::vec3 lightPosition = light->GetRotation() * glm::vec3(0, 0, 1);
	glm::mat4 projection = glm::ortho(-100.f, 100.f, -100.f, 100.f, -100.f, 100.f);
	glm::mat4 view = glm::lookAt(lightPosition * 10.f, glm::vec3(0), glm::vec3(0, 1, 0));
	glm::mat4 shadowDepthMatrix = projection * view;

	for (int i = 0; i < sprites.size(); ++i) {
		Sprite sprite = sprites[i];
		if (sprite->GetRenderer()->GetRenderQueue() < RenderQueueGeometry) {
			continue;
		}

		Material material = directionalLightShadowRenderer_->GetMaterial(0);
		material->SetMatrix4(Variables::localToOrthographicLightSpaceMatrix, shadowDepthMatrix * sprite->GetLocalToWorldMatrix());
		RenderSprite(sprite, directionalLightShadowRenderer_);
	}

	glm::mat4 bias(
		0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5, 0.5f, 0.5f, 1.f
	);

	viewToShadowSpaceMatrix_ = bias * shadowDepthMatrix;
	fbShadow_->Unbind();
}

void CameraInternal::RenderForwardAdd(const std::vector<Sprite>& sprites, const std::vector<Light>& lights) {
}

int CameraInternal::RenderBackgroundPass(const std::vector<Sprite>& sprites, int from) {
	pass_ = RenderPassBackground;
	return 0;
}

void CameraInternal::RenderDepthPass(const std::vector<Sprite>& sprites) {
	pass_ = RenderPassDepth;
	fbDepth_->Bind();

	for (int i = 0; i < sprites.size(); ++i) {
		Sprite sprite = sprites[i];
		if (sprite->GetRenderer()->GetRenderQueue() < RenderQueueGeometry) {
			continue;
		}

		RenderSprite(sprite, depthRenderer_);
	}

	fbDepth_->Unbind();
}

int CameraInternal::RenderOpaquePass(const std::vector<Sprite>& sprites, int from) {
	pass_ = RenderPassOpaque;
	for (int i = 0; i < sprites.size(); ++i) {
		Sprite sprite = sprites[i];
		RenderSprite(sprite, sprite->GetRenderer());
	}

	return 0;
}

int CameraInternal::RenderTransparentPass(const std::vector<Sprite>& sprites, int from) {
	pass_ = RenderPassTransparent;
	return 0;
}

void CameraInternal::GetLights(Light& forwardBase, std::vector<Light>& forwardAdd) {
	std::vector<Sprite> lights;
	if (!worldInstance->GetSprites(ObjectTypeLights, lights)) {
		return;
	}

	forwardBase = dsp_cast<Light>(lights.front());
	for (int i = 1; i < lights.size(); ++i) {
		forwardAdd.push_back(dsp_cast<Light>(lights[i]));
	}
}

void CameraInternal::OnPostRender() {
	if (postEffects_.empty()) { return; }

	Framebuffer* framebuffers[] = { fbRenderTexture_, fbRenderTexture2_ };
	RenderTexture textures[] = { fbRenderTexture_->GetRenderTexture(), renderTexture2_ };

	int index = 1;
	for (int i = 0; i < postEffects_.size(); ++i) {
		Framebuffer0* active = framebuffers[index];
		
		if (i + 1 == postEffects_.size()) {
			active = fb0_;
			textures[index].reset();
		}

		active->Bind();
		postEffects_[i]->OnRenderImage(textures[1 - index], textures[index]);
		active->Unbind();

		index = 1 - index;
	}

	fb0_->Unbind();
}

bool CameraInternal::IsRenderable(Sprite sprite) {
	return sprite->GetActive() && sprite->GetSurface() && sprite->GetRenderer();
}

bool CameraInternal::GetRenderableSprites(std::vector<Sprite>& sprites) {
	worldInstance->GetSprites(ObjectTypeSprite, sprites);

	// Sort sprites by render queue.
	int p = 0;
	for (int i = 0; i < sprites.size(); ++i) {
		Sprite key = sprites[i];
		if (!IsRenderable(key)) {
			continue;
		}

		int j = p - 1;

		for (; j >= 0 && sprites[j]->GetRenderer()->GetRenderQueue() > key->GetRenderer()->GetRenderQueue(); --j) {
			sprites[j + 1] = sprites[j];
		}

		sprites[j + 1] = key;
		++p;
	}

	sprites.erase(sprites.begin() + p, sprites.end());
	return !sprites.empty();
}

void CameraInternal::RenderSprite(Sprite sprite, Renderer renderer) {
	for (int i = 0; i < renderer->GetMaterialCount(); ++i) {
		UpdateMaterial(sprite, renderer->GetMaterial(i));
	}

	renderer->RenderSprite(sprite);
}

void CameraInternal::UpdateMaterial(Sprite sprite, Material material) {
	glm::mat4 localToWorldMatrix = sprite->GetLocalToWorldMatrix();
	glm::mat4 worldToCameraSpaceMatrix = GetWorldToLocalMatrix();
	glm::mat4 worldToClipSpaceMatrix = projection_ * worldToCameraSpaceMatrix;
	glm::mat4 localToClipSpaceMatrix = worldToClipSpaceMatrix * sprite->GetLocalToWorldMatrix();
	material->SetMatrix4(Variables::worldToClipSpaceMatrix, worldToClipSpaceMatrix);
	material->SetMatrix4(Variables::worldToCameraSpaceMatrix, worldToCameraSpaceMatrix);
	material->SetMatrix4(Variables::localToClipSpaceMatrix, localToClipSpaceMatrix);

	if (pass_ >= RenderPassOpaque) {
		material->SetMatrix4(Variables::localToShadowSpaceMatrix, viewToShadowSpaceMatrix_ * localToWorldMatrix);
		material->SetTexture(Variables::shadowDepthTexture, fbShadow_->GetDepthTexture());
	}
}
