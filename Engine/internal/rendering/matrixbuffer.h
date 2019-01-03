#pragma once
#include "material.h"

class MatrixBuffer {
public:
	MatrixBuffer(TextureBuffer buffer);

public:
	void Update(uint size, const void* data);

private:
	TextureBuffer textureBuffer_;
};
