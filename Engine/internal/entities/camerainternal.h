#pragma once
#include <glm/glm.hpp>

#include "plane.h"
#include "screen.h"
#include "camera.h"
#include "frustum.h"
#include "internal/entities/entityinternal.h"

//class GBuffer;
class Pipeline;

class ImageEffect;
class GizmosPainter;

class Sample;
class CameraInternal : public ICamera, public EntityInternal, public Frustum, public ScreenSizeChangedListener {
	DEFINE_FACTORY_METHOD(Camera)

public:
	CameraInternal();
	~CameraInternal();

public:
	virtual void SetDepth(int value);
	virtual int GetDepth() { return depth_;  }

	virtual void SetClearType(ClearType value) { clearType_ = value; }
	virtual ClearType GetClearType() { return clearType_; }

	virtual void SetRenderPath(RenderPath value) { renderPath_ = value; }
	virtual RenderPath GetRenderPath() { return renderPath_; }

	virtual void SetDepthTextureMode(DepthTextureMode value) { depthTextureMode_ = value; }
	virtual DepthTextureMode GetDepthTextureMode() { return depthTextureMode_; }

	virtual void SetClearColor(const glm::vec3& value) { clearColor_ = value; }
	virtual glm::vec3 GetClearColor() { return clearColor_; }

	virtual void SetTargetTexture(RenderTexture value) { targetTexture_ = value; }
	virtual RenderTexture GetTargetTexture() { return targetTexture_; }

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

	virtual void SetRect(const Rect& value);
	virtual const Rect& GetRect() const { return normalizedRect_; }

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
	void CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue);

	void ClearRenderTextures();

	void UpdateTransformsUniformBuffer();

	void ForwardRendering(RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);
	void DeferredRendering(RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(RenderTexture target, const std::vector<Entity>& entities);

	void CreateAuxTexture1();
	void CreateAuxTexture2();
	void CreateDepthTexture();
	void RenderSkybox(RenderTexture target);

	RenderTexture GetActiveRenderTarget();

	void ForwardPass(RenderTexture target, const std::vector<Entity>& entities);
	void ForwardDepthPass(const std::vector<Entity>& entities);

	void RenderEntity(RenderTexture target, Entity entity, Renderer renderer);
	void RenderSubMesh(RenderTexture target, Entity entity, int subMeshIndex, Material material, int pass);

	void UpdateForwardBaseLightUniformBuffer(const std::vector<Entity>& entities, Light light);

	void RenderForwardAdd(const std::vector<Entity>& entities, const std::vector<Light>& lights);
	void RenderForwardBase(RenderTexture target, const std::vector<Entity>& entities, Light light);

	void RenderDecals(RenderTexture target);
	void OnPostRender();

	void OnDrawGizmos();
	void OnImageEffects();

	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);

private:
	int depth_;
	Rect normalizedRect_;

	//GBuffer* gbuffer_;

	Plane planes_[6];

	RenderTexture auxTexture1_;
	RenderTexture auxTexture2_;
	RenderTexture depthTexture_;
	RenderTexture targetTexture_;

	Pipeline* pipeline_;
	Sample *push_renderables, *forward_pass, *get_renderable_entities;

	// TODO: Common material.
	Material decalMaterial_;
	Material depthMaterial_;
	Material skyboxMaterial_;
	Material deferredMaterial_;

	std::vector<ImageEffect*> imageEffects_;
	std::vector<GizmosPainter*> gizmosPainters_;

	ClearType clearType_;
	glm::vec3 clearColor_;

	RenderPath renderPath_;
	DepthTextureMode depthTextureMode_;
};
