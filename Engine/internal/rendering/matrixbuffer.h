#pragma once
#include "material.h"

class MatrixBuffer {
public:
	static void Initialize();

	static void Update(uint size, const void* data);
	static void AttachMatrixBuffer(Material material);
};
