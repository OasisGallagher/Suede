#include "variables.h"
#include "matrixbuffer.h"
#include "../api/glutils.h"
#include "sharedtexturemanager.h"
#include "internal/base/renderdefines.h"

MatrixBuffer::MatrixBuffer() {
	textureBuffer_ = SharedTextureManager::instance()->GetMatrixTextureBuffer();
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
