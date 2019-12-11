#pragma once
#include "texture.h"
#include "material.h"
#include "tools/singleton.h"

class SharedTextureManager : public Singleton<SharedTextureManager> {
	friend class Singleton<SharedTextureManager>;

public:
	SharedTextureManager();
	~SharedTextureManager();

public:
	void Attach(Material* material);

public:
	/**
	 * @brief ssao render texture updated by Rendering.
	 */
	RenderTexture* GetSSAOTexture() { return ssao_.get(); }

	/**
	 * @brief depth render texture updated by Rendering.
	 */
	RenderTexture* GetDepthTexture() { return depth_.get(); }

	/**
	 * @brief shadow depth render texture updated by Shadows.
	 */
	RenderTexture* GetShadowDepthTexture() { return shadowDepth_.get(); }

	/**
	 * @brief matrix texture buffer updated by MatrixBuffer.
	 */
	TextureBuffer* GetMatrixTextureBuffer() { return matrixBuffer_.get(); }

private:
	ref_ptr<RenderTexture> ssao_;
	ref_ptr<RenderTexture> depth_;
	ref_ptr<RenderTexture> shadowDepth_;
	ref_ptr<TextureBuffer> matrixBuffer_;
};
