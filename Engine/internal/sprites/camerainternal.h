#pragma once
#include <glm/glm.hpp>

#include "camera.h"
#include "skybox.h"
#include "posteffect.h"
#include "internal/base/objectinternal.h"
#include "internal/sprites/spriteinternal.h"

class Framebuffer;
class Framebuffer0;

class CameraInternal : public ICamera, public SpriteInternal {
	DEFINE_FACTORY_METHOD(Camera)
	
	enum RenderPass {
		RenderPassNone = -1,
		RenderPassBackground,
		RenderPassDepth,
		RenderPassShadow,
		RenderPassOpaque,
		RenderPassTransparent,
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

public:
	virtual void AddPostEffect(PostEffect* effect) { postEffects_.push_back(effect); }
	virtual Texture2D Capture();

private:
	void CreateFramebuffers();
	void CreateDepthRenderer();
	void CreateShadowRenderer();

	void UpdateSkybox();
	void OnContextSizeChanged(int w, int h);
	Framebuffer0* GetActiveFramebuffer();

	void RenderDepthPass(const std::vector<Sprite>& sprites);
	void RenderShadowPass(const std::vector<Sprite>& sprites, Light light);
	int RenderBackgroundPass(const std::vector<Sprite>& sprites, int from);
	int RenderOpaquePass(const std::vector<Sprite>& sprites, int from);
	int RenderTransparentPass(const std::vector<Sprite>& sprites, int from);

	bool IsRenderable(Sprite sprite);

	void RenderSprite(Sprite sprite, Renderer renderer);
	void UpdateMaterial(Sprite sprite, Material material);

	bool GetRenderableSprites(std::vector<Sprite>& sprites);

	void SetForwardBaseLightParameter(const std::vector<Sprite>& sprites, Light light);

	void RenderForwardBase(const std::vector<Sprite>& sprites, Light light);
	void RenderForwardAdd(const std::vector<Sprite>& sprites, const std::vector<Light>& lights);

	void OnPostRender();
	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);

private:
	int depth_;

	float aspect_;
	float near_, far_;
	float fieldOfView_;
	glm::mat4 projection_;
	glm::mat4 viewToShadowSpaceMatrix_;

	Framebuffer0* fb0_;
	Framebuffer* fbDepth_;
	Framebuffer* fbShadow_;
	Framebuffer* fbRenderTexture_;
	Framebuffer* fbRenderTexture2_;

	// TODO: Common renderer.
	Renderer depthRenderer_;
	Renderer directionalLightShadowRenderer_;

	RenderTexture renderTexture_;
	RenderTexture renderTexture2_;

	std::vector<PostEffect*> postEffects_;

	RenderPass pass_;

	Skybox skybox_;
	ClearType clearType_;
};
