#include "gl.h"
#include <vector>

class Destroyer {
public:
	Destroyer(GLsizei n, const GLuint* buffers, const std::function<void(GLsizei, const GLuint*)>& f) : n_(n), f_(f) {
		buffers_ = new GLuint[n];
		memcpy(buffers_, buffers, n * sizeof(GLuint));
	}

	~Destroyer() {
		delete[] buffers_;
	}

	void operator()() {
		f_(n_, buffers_);
	}

	Destroyer(Destroyer&& other) {
		n_ = other.n_;
		f_ = other.f_;
		buffers_ = other.buffers_;

		other.n_ = 0;
		other.buffers_ = nullptr;
		other.f_ = std::function<void(GLsizei, const GLuint*)>();
	}

	Destroyer(const Destroyer& other) = delete;
	Destroyer& operator=(const Destroyer&) = delete;

private:
	GLsizei n_;
	std::function<void(GLsizei, const GLuint*)> f_;
	GLuint* buffers_;
};

// TODO destroy gl resources in rendering thread.
static std::vector<Destroyer> delayDestroyers;

void GL::Update() {
	for (auto& destroyer : delayDestroyers) {
		destroyer();
	}

	delayDestroyers.clear();
}

void GL::DeleteBuffers(GLsizei n, const GLuint* buffers) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteBuffers(n, buffers));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, buffers, [](GLsizei n, const GLuint* buffers) {
			glDeleteBuffers(n, buffers);
		}));
	}
}

void GL::DeleteFramebuffers(GLsizei n, GLuint* framebuffers) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteFramebuffers(n, framebuffers));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, framebuffers, [](GLsizei n, const GLuint* buffers) {
			glDeleteFramebuffers(n, buffers);
		}));
	}
}

void GL::DeleteProgram(GLuint program) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteProgram(program));
	}
	else {
		delayDestroyers.push_back(Destroyer(1, &program, [](GLsizei n, const GLuint* buffers) {
			glDeleteProgram(*buffers);
		}));
	}
}

void GL::DeleteQueries(GLsizei n, GLuint* ids) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteQueries(n, ids));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, ids, [](GLsizei n, const GLuint* buffers) {
			glDeleteQueries(n, buffers);
		}));
	}
}

void GL::DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteRenderbuffers(n, renderbuffers));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, renderbuffers, [](GLsizei n, const GLuint* buffers) {
			glDeleteRenderbuffers(n, buffers);
		}));
	}
}

void GL::DeleteShader(GLuint shader) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteShader(shader));
	}
	else {
		delayDestroyers.push_back(Destroyer(1, &shader, [](GLsizei n, const GLuint* buffers) {
			glDeleteShader(*buffers);
		}));
	}
}

void GL::DeleteTextures(GLsizei n, const GLuint* textures) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteTextures(n, textures));
	}
	else {
		delayDestroyers.push_back(Destroyer(n, textures, [](GLsizei n, const GLuint* buffers) {
			glDeleteTextures(n, buffers);
		}));
	}
}

void GL::DeleteVertexArrays(GLsizei n, const GLuint* arrays) {
	if (ZThread::Thread::isMainThread()) {
		GL_CALL(glDeleteVertexArrays(n, arrays));
		
	}
	else {
		delayDestroyers.push_back(Destroyer(n, arrays, [](GLsizei n, const GLuint* buffers) {
			glDeleteVertexArrays(n, buffers);
		}));
	}
}
