#include "tools/debug.h"
#include "transformfeedback.h"
#include "internal/memory/memory.h"
#include "internal/base/vertexarrayobject.h"

TransformFeedback::TransformFeedback() : tfbs_(nullptr), tfCount_(0), bindingVboIndex_(-1) {
	vao_ = Memory::Create<VertexArrayObject>();
}

TransformFeedback::~TransformFeedback() {
	Destroy();
	Memory::Release(vao_);
}

void TransformFeedback::Create(size_t n, size_t size, const void* data) {
	Assert(n > 0 && size > 0);
	Destroy();

	tfCount_ = n;

	vao_->Create(n);
	vao_->Bind();

	tfbs_ = Memory::CreateArray<GLuint>(n);
	glCreateTransformFeedbacks(n, tfbs_);
	
	for (size_t i = 0; i < n; ++i) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfbs_[i]);
		vao_->SetBuffer(i, GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);

		// This makes this buffer a transform feedback buffer and places it as index zero.
		// We can have the primitives redirected into more than one buffer by binding several buffers at different indices. 
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vao_->GetBufferNativePointer(i));

		// Now we have two transform feedback objects with corresponding buffer objects that
		// can serve both as vertex buffers as well as transform feedback buffers.
	}
}

void TransformFeedback::Destroy() {
	if (tfCount_ == 0) {
		return;
	}

	glDeleteTransformFeedbacks(tfCount_, tfbs_);
	Memory::ReleaseArray(tfbs_);
	tfbs_ = nullptr;

	vao_->Destroy();

	tfCount_ = 0;
}

void TransformFeedback::Bind(int tfbIndex, int vboIndex) {
	Assert(tfbIndex >= 0 && tfbIndex < tfCount_);
	Assert(vboIndex >= 0 && vboIndex < tfCount_);

	vao_->BindBuffer(vboIndex);
	bindingVboIndex_ = vboIndex;

	glGetIntegerv(GL_TRANSFORM_FEEDBACK_BINDING, (GLint*)&oldTfb_);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfbs_[tfbIndex]);
}

void TransformFeedback::Unbind() {
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, oldTfb_);
	vao_->UnbindBuffer(bindingVboIndex_);
	bindingVboIndex_ = -1;
}

void TransformFeedback::SetVertexDataSource(int index, int location, int size, GLenum type, bool normalized, int stride, unsigned offset) {
	vao_->SetVertexDataSource(index, location, size, type, normalized, stride, offset);
}
