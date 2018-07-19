#pragma once
#include "material.h"
#include "tools/singleton.h"

class MatrixBuffer : public Singleton<MatrixBuffer> {
	friend class Singleton<MatrixBuffer>;

public:
	void Update(uint size, const void* data);
	void AttachMatrixBuffer(Material material);

private:
	MatrixBuffer();

private:
	TextureBuffer textureBuffer_;
};
