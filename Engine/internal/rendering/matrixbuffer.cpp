#include "variables.h"
#include "api/glutils.h"
#include "matrixbuffer.h"

TextureBuffer textureBuffer_;

void MatrixBuffer::Initialize() {
}

void MatrixBuffer::Update(uint size, const void* data) {
	if (size > GLUtils::GetLimits(GLLimitsMaxTextureBufferSize)) {
		Debug::LogError("exceeds matrix buffer max size %u.");
		return;
	}

	if (!textureBuffer_ || textureBuffer_->GetSize() < size) {
		textureBuffer_ = NewTextureBuffer();
		textureBuffer_->Create(size);
	}

	textureBuffer_->Update(0, size, data);
}

void MatrixBuffer::AttachMatrixBuffer(Material material) {
	material->SetTexture(Variables::matrixBuffer, textureBuffer_);
}
