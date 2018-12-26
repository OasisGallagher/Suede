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
	void Attach(Material material);

public:
	/**
	 * @brief ssao render texture updated by Rendering.
	 */
	RenderTexture GetSSAOTexture() { return ssao_; }

	/**
	 * @brief depth render texture updated by Rendering.
	 */
	RenderTexture GetDepthTexture() { return depth_; }

	/**
	 * @brief shadow depth render texture updated by Shadows.
	 */
	RenderTexture GetShadowDepthTexture() { return shadowDepth_; }

	/**
	 * @brief matrix texture buffer updated by MatrixBuffer.
	 */
	TextureBuffer GetMatrixTextureBuffer() { return matrixBuffer_; }

private:
	RenderTexture ssao_;
	RenderTexture depth_;
	RenderTexture shadowDepth_;
	TextureBuffer matrixBuffer_;
};
