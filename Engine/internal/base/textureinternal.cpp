#include <glm/glm.hpp>

#include "tools/path.h"
#include "tools/debug.h"
#include "textureinternal.h"
#include "internal/file/imagecodec.h"

void TextureInternal::Bind(GLenum location) {
	AssertX(glIsTexture(texture_), "invalid texture");
	location_ = location;
	glActiveTexture(location);
	BindTexture();
}

void TextureInternal::Unbind() {
	glActiveTexture(location_);
	UnbindTexture();
}

void TextureInternal::BindTexture() {
	glGetIntegerv(GetGLTextureBindingName(), &oldBindingTexture_);
	glBindTexture(GetGLTextureType(), texture_);
}

void TextureInternal::UnbindTexture() {
	glBindTexture(GetGLTextureType(), oldBindingTexture_);
	oldBindingTexture_ = 0;
}

void TextureInternal::DestroyTexture() {
	if (texture_ != 0) {
		glDeleteTextures(1, &texture_);
		texture_ = 0;
	}
}

Texture2DInternal::Texture2DInternal() : TextureInternal(ObjectTypeTexture2D) {
}

Texture2DInternal::~Texture2DInternal() {
	DestroyTexture();
}

bool Texture2DInternal::Load(const std::string& path) {
	int width, height;
	const void* data = ImageCodec::Decode(Path::GetResourceRootDirectory() + path, width, height);
	if (data == nullptr) {
		return false;
	}

	return Load(data, width, height);
}

bool Texture2DInternal::Load(const void* data, int width, int height) {
	DestroyTexture();

	width_ = width;
	height_ = height;

	glGenTextures(1, &texture_);
	
	BindTexture();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	UnbindTexture();

	return true;
}

bool Texture2DInternal::EncodeToPng(std::vector<unsigned char>& data) {
	BindTexture();
	data.resize(4 * GetWidth() * GetHeight());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	UnbindTexture();

	return ImageCodec::Encode(GetWidth(), GetHeight(), data, "PNG");
}

bool Texture2DInternal::EncodeToJpg(std::vector<unsigned char>& data) {
	BindTexture();
	data.resize(4 * GetWidth() * GetHeight());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	UnbindTexture();

	return ImageCodec::Encode(GetWidth(), GetHeight(), data, "JPG");
}

TextureCubeInternal::TextureCubeInternal() : TextureInternal(ObjectTypeTextureCube) {
}

TextureCubeInternal::~TextureCubeInternal() {
	DestroyTexture();
}

bool TextureCubeInternal::Load(const std::string(&textures)[6]) {
	DestroyTexture();

	glGenTextures(1, &texture_);

	BindTexture();

	for (int i = 0; i < 6; ++i) {
		int width, height;
		const void* data = ImageCodec::Decode(Path::GetResourceRootDirectory() + textures[i], width, height);

		if (data == nullptr) {
			DestroyTexture();
			return false;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
			width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	UnbindTexture();
	return true;
}

RenderTextureInternal::RenderTextureInternal() :TextureInternal(ObjectTypeRenderTexture) {	
}

bool RenderTextureInternal::Load(RenderTextureFormat format, int width, int height) {
	DestroyTexture();
	
	width_ = width;
	height_ = height;

	glGenTextures(1, &texture_);
	BindTexture();

	GLenum parameters[3];
	RenderTextureFormatToGLEnum(format, parameters);
	glTexImage2D(GL_TEXTURE_2D, 0, parameters[0], width, height, 0, parameters[1], parameters[2], nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (format == RenderTextureFormatShadow) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	}

	UnbindTexture();

	return true;
}

void RenderTextureInternal::RenderTextureFormatToGLEnum(RenderTextureFormat renderTextureFormat, GLenum(&parameters)[3]) {
	GLenum internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	switch (renderTextureFormat) {
		case  RenderTextureFormatRgba:
			internalFormat = GL_RGBA;
			break;
		case RenderTextureFormatRgbaHdr:
			internalFormat = GL_RGBA32F;
			break;
		case RenderTextureFormatDepth:
		case RenderTextureFormatShadow:
			internalFormat = GL_DEPTH_COMPONENT24;
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
			break;
		default:
			Debug::LogError("invalid render texture format: " + std::to_string(renderTextureFormat));
			break;
	}

	parameters[0] = internalFormat;
	parameters[1] = format;
	parameters[2] = type;
}
