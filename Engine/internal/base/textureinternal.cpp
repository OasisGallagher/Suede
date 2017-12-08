#include <glm/glm.hpp>

#include "debug.h"
#include "tools/path.h"
#include "textureinternal.h"
#include "internal/file/image.h"

void TextureInternal::Bind(uint index) {
	if (!glIsTexture(texture_)) {
		Debug::LogError("invalid texture");
		return;
	}

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

uint TextureInternal::GetFormatBitsPerPixel() {
	switch (format_) {
		case GL_RGB: return 3;
		case GL_RGBA: return 4;
	}

	Debug::LogError("unknown internal format 0x%x.", format_);
	return 3;
}

void TextureInternal::ColorFormatToGLEnum(ColorFormat format, GLenum(&parameters)[3]) {
	GLenum glInternalFormat = GL_RGBA, glFormat = GL_RGBA, glType = GL_UNSIGNED_BYTE;
	switch (format) {
		case ColorFormatRgb:
			glFormat = GL_RGB;
			glInternalFormat = GL_RGB;
			break;
		case ColorFormatBgr:
			glFormat = GL_BGR;
			glInternalFormat = GL_RGB;
			break;
		case ColorFormatRgba:
			glFormat = GL_RGBA;
			break;
		case ColorFormatArgb:
			glFormat = GL_BGRA;
			glType = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;
		case ColorFormatBgra:
			glFormat = GL_BGRA;
			break;
		case ColorFormatLuminanceAlpha:
			glInternalFormat = GL_RGB;
			glFormat = GL_LUMINANCE_ALPHA;
			break;
	}

	parameters[0] = glInternalFormat;
	parameters[1] = glFormat;
	parameters[2] = glType;
}

Texture2DInternal::Texture2DInternal() : TextureInternal(ObjectTypeTexture2D) {
}

Texture2DInternal::~Texture2DInternal() {
	DestroyTexture();
}

bool Texture2DInternal::Load(const std::string& path) {
	Bitmap bitmap;
	if (!ImageCodec::Decode(bitmap, Path::GetResourceRootDirectory() + path)) {
		return false;
	}

	return Load(&bitmap.data[0], bitmap.format, bitmap.width, bitmap.height);
}

// TODO: assume UNPACK_ALIGNMENT = 4.
bool Texture2DInternal::Load(const void* data, ColorFormat format, int width, int height) {
	DestroyTexture();

	width_ = width;
	height_ = height;

	glGenTextures(1, &texture_);
	
	BindTexture();

	GLenum glFormat[3];
	ColorFormatToGLEnum(format, glFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat[0], width, height, 0, glFormat[1], glFormat[2], data);

	format_ = glFormat[0];

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	UnbindTexture();

	return true;
}

bool Texture2DInternal::EncodeToPng(std::vector<uchar>& data) {
	BindTexture();
	data.resize(GetFormatBitsPerPixel() * GetWidth() * GetHeight());
	glGetTexImage(GL_TEXTURE_2D, 0, format_, GL_UNSIGNED_BYTE, &data[0]);
	UnbindTexture();

	return ImageCodec::Encode(GetWidth(), GetHeight(), data, BitsPerPixel32, ImageTypePng);
}

bool Texture2DInternal::EncodeToJpg(std::vector<uchar>& data) {
	BindTexture();
	data.resize(GetFormatBitsPerPixel() * GetWidth() * GetHeight());
	glGetTexImage(GL_TEXTURE_2D, 0, format_, GL_UNSIGNED_BYTE, &data[0]);
	UnbindTexture();

	return ImageCodec::Encode(GetWidth(), GetHeight(), data, BitsPerPixel24, ImageTypeJpg);
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
		Bitmap bitmap;
		if (!ImageCodec::Decode(bitmap, Path::GetResourceRootDirectory() + textures[i])) {
			DestroyTexture();
			return false;
		}

		GLenum glFormat[3];
		ColorFormatToGLEnum(bitmap.format, glFormat);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glFormat[0], bitmap.width, bitmap.height, 0, glFormat[1], glFormat[2], &bitmap.data[0]);
		format_ = glFormat[0];
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

	GLenum glFormat[3];
	RenderTextureFormatToGLEnum(format, glFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat[0], width, height, 0, glFormat[1], glFormat[2], nullptr);
	format_ = glFormat[0];

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

void RenderTextureInternal::RenderTextureFormatToGLEnum(RenderTextureFormat input, GLenum(&parameters)[3]) {
	GLenum internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	switch (input) {
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
			Debug::LogError("invalid render texture format: %d.", input);
			break;
	}

	parameters[0] = internalFormat;
	parameters[1] = format;
	parameters[2] = type;
}
