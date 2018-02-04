#pragma once
#include <glm/glm.hpp>

#include "camera.h"
#include "skybox.h"
#include "imageeffect.h"
#include "internal/base/objectinternal.h"
#include "internal/entities/entityinternal.h"

class GBuffer;
class Framebuffer;
class FramebufferBase;

class CameraInternal : public ICamera, public EntityInternal {
	DEFINE_FACTORY_METHOD(Camera)
	
	enum RenderPass {
		RenderPassNone = -1,

		RenderPassShadowDepth,

		RenderPassForwardBackground,
		RenderPassForwardDepth,
		RenderPassForwardOpaque,
		RenderPassForwardTransparent,

		RenderPassDeferredGeometryPass,

		RenderPassCount
	};

public:
	CameraInternal();
	~CameraInternal();

public:
	virtual void SetDepth(int value) { depth_ = value; }
	virtual int GetDepth() { return depth_;  }

	virtual void SetClearType(ClearType value) { clearType_ = value; }
	virtual ClearType GetClearType() { return clearType_; }

	virtual void SetRenderPath(RenderPath value) { renderPath_ = value; }
	virtual RenderPath GetRenderPath() { return renderPath_; }

	virtual void SetDepthTextureMode(DepthTextureMode value) { depthTextureMode_ = value; }
	virtual DepthTextureMode GetDepthTextureMode() { return depthTextureMode_; }

	virtual void SetSkybox(Skybox value) { skybox_ = value; }
	virtual Skybox GetSkybox() { return skybox_; }

	virtual void SetClearColor(const glm::vec3& value);
	virtual glm::vec3 GetClearColor();

	virtual void SetRenderTexture(RenderTexture value);
	virtual RenderTexture GetRenderTexture() { return renderTexture_; }

public:
	virtual void Update();
	virtual void Render();

public:
	virtual void SetAspect(float value);
	virtual void SetNearClipPlane(float value);
	virtual void SetFarClipPlane(float value);
	virtual void SetFieldOfView(float value);

	virtual float GetAspect() { return aspect_; }
	virtual float GetNearClipPlane() { return near_; }
	virtual float GetFarClipPlane() { return far_; }
	virtual float GetFieldOfView() { return fieldOfView_; }

	virtual const glm::mat4& GetProjectionMatrix() { return projection_; }

	virtual glm::vec3 WorldToScreenPoint(const glm::vec3& position);
	virtual glm::vec3 ScreenToWorldPoint(const glm::vec3& position);

public:
	virtual void AddImageEffect(ImageEffect* effect) { imageEffects_.push_back(effect); }
	virtual Texture2D Capture();

private:
	void InitializeVariables();
	void CreateFramebuffers();
	void CreateDepthMaterial();
	void CreateShadowMaterial();

	void ForwardRendering(const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);
	void DeferredRendering(const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(const std::vector<Entity>& entities);

	void UpdateSkybox();
	void SetUpFramebuffer1();
	void CreateFramebuffer2();

	void OnContextSizeChanged(int w, int h);
	FramebufferBase* GetActiveFramebuffer();

	void ShadowDepthPass(const std::vector<Entity>& entities, Light light);

	void ForwardDepthPass(const std::vector<Entity>& entities);
	int ForwardBackgroundPass(const std::vector<Entity>& entities, int from);
	int ForwardOpaquePass(const std::vector<Entity>& entities, int from);
	int ForwardTransparentPass(const std::vector<Entity>& entities, int from);

	bool IsRenderable(Entity entity);

	void RenderEntity(Entity entity, Renderer renderer);
	void UpdateMaterial(Entity entity, Material material);

	void GetRenderableEntities(std::vector<Entity>& entities);
	void SortRenderableEntities(std::vector<Entity>& entities);

	void SetForwardBaseLightParameter(const std::vector<Entity>& entities, Light light);

	void RenderForwardBase(const std::vector<Entity>& entities, Light light);
	void RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights);

	void OnPostRender();
	void OnImageEffects();

	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);

private:
	int depth_;

	float aspect_;
	float near_, far_;
	float fieldOfView_;
	glm::mat4 projection_;
	glm::mat4 viewToShadowSpaceMatrix_;

	GBuffer* gbuffer_;

	Framebuffer* fb1_;
	Framebuffer* fb2_;

	RenderTexture depthTexture_;
	RenderTexture shadowTexture_;

	RenderTexture renderTexture_;
	RenderTexture renderTexture2_;

	// TODO: Common material.
	Material depthMaterial_;
	Material deferredMaterial_;
	Material directionalLightShadowMaterial_;

	std::vector<ImageEffect*> imageEffects_;

	RenderPass pass_;

	Skybox skybox_;

	ClearType clearType_;
	RenderPath renderPath_;
	DepthTextureMode depthTextureMode_;
};
