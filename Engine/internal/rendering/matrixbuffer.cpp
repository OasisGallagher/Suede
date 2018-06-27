#include "variables.h"
#include "api/glutils.h"
#include "matrixbuffer.h"
#include "internal/base/renderdefines.h"

TextureBuffer textureBuffer_;

void MatrixBuffer::Initialize() {
	textureBuffer_ = NewTextureBuffer();
	textureBuffer_->Create(INIT_RENDERABLE_CAPACITY * sizeof(glm::mat4) * 2);
}

void MatrixBuffer::Update(uint size, const void* data) {
	size *= sizeof(glm::mat4) * 2;

	if (size > GLUtils::GetLimits(GLLimitsMaxTextureBufferSize)) {
		Debug::LogError("%u exceeds matrix buffer max size %u.", size, GLUtils::GetLimits(GLLimitsMaxTextureBufferSize));
		return;
	}

	uint newSize = textureBuffer_->GetSize();
	for (; size > newSize; newSize *= 2)
		;
	
	if (newSize > textureBuffer_->GetSize()) {
		textureBuffer_->Create(newSize);
	}

	textureBuffer_->Update(0, size, data);
}

void MatrixBuffer::AttachMatrixBuffer(Material material) {
	material->SetTexture(Variables::matrixBuffer, textureBuffer_);
}
