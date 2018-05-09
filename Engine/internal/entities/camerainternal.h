#pragma once
#include <glm/glm.hpp>

#include "plane.h"
#include "screen.h"
#include "camera.h"
#include "frustum.h"
#include "internal/entities/entityinternal.h"

class GBuffer;
class Pipeline;

class ImageEffect;
class GizmosPainter;

class Framebuffer;
class FramebufferBase;
struct FramebufferState;

class Sample;
class CameraInternal : public ICamera, public EntityInternal, public Frustum, public ScreenSizeChangedListener {
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

	virtual void SetTargetTexture(RenderTexture value);
	virtual RenderTexture GetTargetTexture();

	virtual Texture2D Capture();

public:
	virtual void Update();
	virtual void Render();

public:
	virtual bool GetPerspective() const { return Frustum::GetPerspective(); }
	virtual void SetPerspective(bool value) { return Frustum::SetPerspective(value); }

	virtual float GetOrthographicSize() const { return Frustum::GetOrthographicSize(); }
	virtual void SetOrthographicSize(float value) { return Frustum::SetOrthographicSize(value); }

	virtual void SetAspect(float value) { return Frustum::SetAspect(value); }
	virtual float GetAspect() const { return Frustum::GetAspect(); }

	virtual void SetNearClipPlane(float value) { return Frustum::SetNearClipPlane(value); }
	virtual float GetNearClipPlane() const { return Frustum::GetNearClipPlane(); }

	virtual void SetFarClipPlane(float value) { return Frustum::SetFarClipPlane(value); }
	virtual float GetFarClipPlane() const { return Frustum::GetFarClipPlane(); }

	virtual void SetFieldOfView(float value) { return Frustum::SetFieldOfView(value); }
	virtual float GetFieldOfView() const { return Frustum::GetFieldOfView(); }

	virtual const glm::mat4& GetProjectionMatrix() { return Frustum::GetProjectionMatrix(); }

	virtual glm::vec3 WorldToScreenPoint(const glm::vec3& position);
	virtual glm::vec3 ScreenToWorldPoint(const glm::vec3& position);

public:
	virtual void AddImageEffect(ImageEffect* effect) { imageEffects_.push_back(effect); }
	virtual void AddGizmosPainter(GizmosPainter* painter) { gizmosPainters_.push_back(painter); }

public:
	virtual void OnScreenSizeChanged(uint width, uint height);

protected:
	virtual void OnProjectionMatrixChanged();

private:
	void InitializeVariables();
	void CreateFramebuffers();
	void CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue);

	void ClearFramebuffers();
	void UpdateTimeUniformBuffer();
	void UpdateTransformsUniformBuffer();

	void ForwardRendering(const FramebufferState& state, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);
	void DeferredRendering(const FramebufferState& state, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(const FramebufferState& state, const std::vector<Entity>& entities);

	void SetUpFramebuffer1();
	void CreateFramebuffer2();
	void CreateDepthFramebuffer();
	void RenderSkybox(const FramebufferState& state);

	FramebufferBase* GetActiveFramebuffer();

	//void ShadowDepthPass(const std::vector<Entity>& entities, Light light);

	void ForwardPass(const FramebufferState& state, const std::vector<Entity>& entities);
	void ForwardDepthPass(const std::vector<Entity>& entities);

	void RenderEntity(const FramebufferState& state, Entity entity, Renderer renderer);
	void RenderSubMesh(const FramebufferState& state, Entity entity, int subMeshIndex, Material material, int pass);

	void UpdateForwardBaseLightUniformBuffer(const std::vector<Entity>& entities, Light light);

	void RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights);
	void RenderForwardBase(const FramebufferState& state, const std::vector<Entity>& entities, Light light);

	void RenderDecals(const FramebufferState& state);
	void OnPostRender();

	void OnDrawGizmos();
	void OnImageEffects();

	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);

private:
	int depth_;

	GBuffer* gbuffer_;

	Plane planes_[6];

	Framebuffer* fb1_;
	Framebuffer* fb2_;
	Framebuffer* fbDepth_;

	Pipeline* pipeline_;
	Sample *push_renderables, *forward_pass, *get_renderable_entities;

	RenderTexture depthTexture_;

	RenderTexture renderTexture_;
	RenderTexture renderTexture2_;

	// TODO: Common material.
	Material decalMaterial_;
	Material depthMaterial_;
	Material skyboxMaterial_;
	Material deferredMaterial_;

	std::vector<ImageEffect*> imageEffects_;
	std::vector<GizmosPainter*> gizmosPainters_;

	ClearType clearType_;
	RenderPath renderPath_;
	DepthTextureMode depthTextureMode_;
};
