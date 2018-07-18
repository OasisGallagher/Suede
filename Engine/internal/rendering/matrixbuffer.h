#pragma once
#include "material.h"
#include "tools/singleton.h"

class MatrixBuffer : public Singleton<MatrixBuffer> {
public:
	void Update(uint size, const void* data);
	void AttachMatrixBuffer(Material material);

public:
	MatrixBuffer();

private:
	TextureBuffer textureBuffer_;
};
