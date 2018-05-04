#pragma once
#include <glm/glm.hpp>

#include "plane.h"
#include "camera.h"
#include "frustum.h"
#include "internal/entities/entityinternal.h"

class GBuffer;
class Pipeline;

class ImageEffect;
class GizmosPainter;

class Framebuffer;
class FramebufferBase;

class Sample;
class CameraInternal : public ICamera, public EntityInternal, public ProjectionMatrixChangedListener {
	DEFINE_FACTORY_METHOD(Camera)

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

	virtual void SetClearColor(const glm::vec3& value);
	virtual glm::vec3 GetClearColor();

	virtual void SetRenderTexture(RenderTexture value);
	virtual RenderTexture GetRenderTexture() { return renderTexture_; }

public:
	virtual void Update();
	virtual void Render();

public:
	virtual bool GetPerspective() const;
	virtual void SetPerspective(bool value);

	virtual float GetOrthographicSize() const;
	virtual void SetOrthographicSize(float value);

	virtual void SetAspect(float value);
	virtual void SetNearClipPlane(float value);
	virtual void SetFarClipPlane(float value);
	virtual void SetFieldOfView(float value);

	virtual float GetAspect();
	virtual float GetNearClipPlane();
	virtual float GetFarClipPlane();
	virtual float GetFieldOfView();

	virtual const glm::mat4& GetProjectionMatrix();

	virtual glm::vec3 WorldToScreenPoint(const glm::vec3& position);
	virtual glm::vec3 ScreenToWorldPoint(const glm::vec3& position);

public:
	virtual void AddImageEffect(ImageEffect* effect) { imageEffects_.push_back(effect); }
	virtual void AddGizmosPainter(GizmosPainter* painter) { gizmosPainters_.push_back(painter); }

	virtual Texture2D Capture();

private:
	virtual void OnProjectionMatrixChanged();

private:
	void InitializeVariables();
	void CreateFramebuffers();
	void CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue);

	void ClearFramebuffers();
	void UpdateViewportSize();
	void UpdateTimeUniformBuffer();
	void UpdateTransformsUniformBuffer();

	void AddToPipeline(Mesh mesh, Material material, const glm::mat4& localToWorldMatrix);

	void ForwardRendering(const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);
	void DeferredRendering(const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(const std::vector<Entity>& entities);

	void SetUpFramebuffer1();
	void CreateFramebuffer2();
	void RenderSkybox();

	void OnViewportSizeChanged(int w, int h);
	FramebufferBase* GetActiveFramebuffer();

	void ShadowDepthPass(const std::vector<Entity>& entities, Light light);

	void ForwardPass(const std::vector<Entity>& entities);
	void ForwardDepthPass(const std::vector<Entity>& entities);

	void RenderEntity(Entity entity, Renderer renderer);
	void UpdateMaterial(Entity entity, const glm::mat4& worldToClipMatrix, Material material);

	void UpdateForwardBaseLightUniformBuffer(const std::vector<Entity>& entities, Light light);

	void RenderForwardBase(const std::vector<Entity>& entities, Light light);
	void RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights);

	void RenderDecals();
	void OnPostRender();

	void OnDrawGizmos();
	void OnImageEffects();

	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);

private:
	int depth_;
	glm::mat4 worldToShadowMatrix_;


	GBuffer* gbuffer_;

	Plane planes_[6];
	Frustum* frustum_;

	Framebuffer* fb1_;
	Framebuffer* fb2_;

	Pipeline* pipeline_;
	Sample *push_renderables, *forward_pass, *get_renderable_entities;

	RenderTexture depthTexture_;
	RenderTexture shadowTexture_;

	RenderTexture renderTexture_;
	RenderTexture renderTexture2_;

	// TODO: Common material.
	Material decalMaterial_;
	Material depthMaterial_;
	Material skyboxMaterial_;
	Material deferredMaterial_;
	Material directionalLightShadowMaterial_;

	std::vector<ImageEffect*> imageEffects_;
	std::vector<GizmosPainter*> gizmosPainters_;

	ClearType clearType_;
	RenderPath renderPath_;
	DepthTextureMode depthTextureMode_;
};
