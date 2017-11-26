#include <glm/glm.hpp>

#include "tools/path.h"
#include "tools/debug.h"
#include "textureinternal.h"
#include "internal/file/image.h"

void TextureInternal::Bind(uint index) {
	AssertX(glIsTexture(texture_), "invalid texture");
	location_ = index + GL_TEXTURE0;
	glActiveTexture(location_);
	BindTexture();
}

void TextureInternal::Unbind() {
	if (location_ != 0) {
		glActiveTexture(location_);
		UnbindTexture();
		location_ = 0;
	}
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
	std::vector<uchar> data;
	if (!ImageCodec::Decode(data, width, height, Path::GetResourceRootDirectory() + path)) {
		return false;
	}

	return Load(&data[0], ColorFormatRgba, width, height);
}

bool Texture2DInternal::Load(const void* data, ColorFormat format, int width, int height) {
	DestroyTexture();

	width_ = width;
	height_ = height;

	glGenTextures(1, &texture_);
	
	BindTexture();

	GLenum glFormat[2];
	ColorFormatToGLEnum(format, glFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, glFormat[0], glFormat[1], data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	UnbindTexture();

	return true;
}

bool Texture2DInternal::EncodeToPng(std::vector<uchar>& data) {
	BindTexture();
	data.resize(4 * GetWidth() * GetHeight());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	UnbindTexture();

	return ImageCodec::Encode(GetWidth(), GetHeight(), data, "PNG");
}

bool Texture2DInternal::EncodeToJpg(std::vector<uchar>& data) {
	BindTexture();
	data.resize(4 * GetWidth() * GetHeight());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	UnbindTexture();

	return ImageCodec::Encode(GetWidth(), GetHeight(), data, "JPG");
}

void Texture2DInternal::ColorFormatToGLEnum(ColorFormat format, GLenum(&parameters)[2]) {
	GLenum glFormat = GL_RGBA;
	GLenum glType = GL_UNSIGNED_BYTE;
	switch (format) {
		case ColorFormatRgb:
			glFormat = GL_RGB;
			break;
		case ColorFormatRgba:
			glFormat = GL_RGBA;
			break;
		case ColorFormatArgb:
			glFormat = GL_BGRA;
			glType = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;
		case ColorFormatLuminanceAlpha:
			glFormat = GL_LUMINANCE_ALPHA;
			break;
	}

	parameters[0] = glFormat;
	parameters[1] = glType;
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
		std::vector<uchar> data;
		
		if (!ImageCodec::Decode(data, width, height, Path::GetResourceRootDirectory() + textures[i])) {
			DestroyTexture();
			return false;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

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
