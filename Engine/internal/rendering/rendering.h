#pragma once
#include "light.h"
#include "entity.h"
#include "texture.h"
#include "material.h"
#include "tools/dirtybits.h"
#include "internal/culling/culling.h"

class Sample;
class Pipeline;
class ImageEffect;

class RenderingThread;
class RenderingListener {
public:
	virtual void OnRenderingFinished() = 0;
};

class RenderingThread : public ZThread::Runnable, public DirtyBits {
	enum {
		Waiting,
		Working,
		Finished,
	};

public:
	struct Matrices {
		glm::vec3 position;
		glm::mat4 projectionMatrix;
		glm::mat4 worldToCameraMatrix;
	};

public:
	RenderingThread(RenderingListener* listener);
	~RenderingThread();

public:
	void Render(std::vector<Entity>& entities, const Matrices& matrices);

	void SetTargetTexture(RenderTexture value) { targetTexture_ = value; }
	RenderTexture GetTargetTexture() { return targetTexture_; }

	void SetClearType(ClearType value) { clearType_ = value; }
	ClearType GetClearType() { return clearType_; }

	void SetRenderPath(RenderPath value) { renderPath_ = value; }
	RenderPath GetRenderPath() { return renderPath_; }

	void SetDepthTextureMode(DepthTextureMode value) { depthTextureMode_ = value; }
	DepthTextureMode GetDepthTextureMode() { return depthTextureMode_; }

	void SetClearColor(const glm::vec3& value) { clearColor_ = value; }
	glm::vec3 GetClearColor() { return clearColor_; }

	void AddImageEffect(ImageEffect* effect) { imageEffects_.push_back(effect); }

public:
	void Resize(uint width, uint height);

	void SetRect(const Rect& value);
	const Rect& GetRect() const { return normalizedRect_; }

protected:
	virtual void run();

private:
	void ApplyPendingOperations();

	void RenderEntities();
	void ClearRenderTextures();

	void UpdateTransformsUniformBuffer();
	void CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue);

	void ForwardRendering(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);
	void DeferredRendering(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities);

	void CreateAuxTexture1();
	void CreateAuxTexture2();
	void CreateDepthTexture();
	void RenderSkybox(Pipeline* pl, RenderTexture target);

	RenderTexture GetActiveRenderTarget();

	void ForwardPass(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities);
	void ForwardDepthPass(Pipeline* pl);

	void RenderEntity(Pipeline* pl, RenderTexture target, Entity entity, Renderer renderer);
	void RenderSubMesh(Pipeline* pl, RenderTexture target, Entity entity, int subMeshIndex, Material material, int pass);

	void UpdateForwardBaseLightUniformBuffer(const std::vector<Entity>& entities, Light light);

	void RenderForwardAdd(Pipeline* pl, const std::vector<Entity>& entities, const std::vector<Light>& lights);
	void RenderForwardBase(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities, Light light);

	void RenderDecals(Pipeline* pl, RenderTexture target);
	void OnPostRender();

	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);
	void OnImageEffects();

private:
	void ApplyResize();

private:
	enum {
		ScreenSize = 1,
		RenderTextures = 1 << 1,
	};

private:
	int status_;

	Matrices matrices_;
	std::vector<Entity> entities_;
	RenderingListener* listener_;

	glm::uvec2 size_;
	Rect normalizedRect_;

	ClearType clearType_;
	glm::vec3 clearColor_;

	RenderPath renderPath_;
	DepthTextureMode depthTextureMode_;

	// TODO: Common material.
	Material decalMaterial_;
	Material depthMaterial_;
	Material skyboxMaterial_;
	Material deferredMaterial_;

	RenderTexture auxTexture1_;
	RenderTexture auxTexture2_;
	RenderTexture depthTexture_;
	RenderTexture targetTexture_;

	std::vector<ImageEffect*> imageEffects_;
	Sample *push_renderables, *forward_pass, *get_renderable_entities;

	Pipeline* pipeline_;
};
