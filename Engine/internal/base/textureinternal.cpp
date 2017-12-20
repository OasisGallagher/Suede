#include <glm/glm.hpp>

#include "debug.h"
#include "tools/path.h"
#include "textureinternal.h"

void TextureInternal::Bind(uint index) {
	if (!GL::IsTexture(texture_)) {
		Debug::LogError("invalid texture");
		return;
	}

	location_ = index + GL_TEXTURE0;
	GL::ActiveTexture(location_);
	BindTexture();
}

void TextureInternal::Unbind() {
	if (location_ != 0) {
		GL::ActiveTexture(location_);
		UnbindTexture();
		location_ = 0;
	}
}

void TextureInternal::BindTexture() {
	GL::GetIntegerv(GetGLTextureBindingName(), &oldBindingTexture_);
	GL::BindTexture(GetGLTextureType(), texture_);
}

void TextureInternal::UnbindTexture() {
	GL::BindTexture(GetGLTextureType(), oldBindingTexture_);
	oldBindingTexture_ = 0;
}

void TextureInternal::DestroyTexture() {
	if (texture_ != 0) {
		GL::DeleteTextures(1, &texture_);
		texture_ = 0;
	}
}

BppType TextureInternal::GLTextureFormatToBpp(GLenum format) {
	switch (format) {
		case GL_RGB: return BppType24;
		case GL_RGBA: return BppType32;
	}

	Debug::LogError("unknown internal format 0x%x.", format);
	return BppType24;
}

void TextureInternal::ColorFormatToGLTextureFormat(ColorFormat format, GLenum(&parameters)[3]) {
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
	TexelMap texelMap;
	if (!ImageCodec::Decode(texelMap, Path::GetResourceRootDirectory() + path)) {
		return false;
	}

	return Load(&texelMap.data[0], texelMap.format, texelMap.width, texelMap.height);
}

// TODO: assume UNPACK_ALIGNMENT = 4.
bool Texture2DInternal::Load(const void* data, ColorFormat format, int width, int height) {
	DestroyTexture();

	width_ = width;
	height_ = height;

	GL::GenTextures(1, &texture_);
	
	BindTexture();

	GLenum glFormat[3];
	ColorFormatToGLTextureFormat(format, glFormat);
	GL::TexImage2D(GL_TEXTURE_2D, 0, glFormat[0], width, height, 0, glFormat[1], glFormat[2], data);

	format_ = glFormat[0];

	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	UnbindTexture();

	return true;
}

bool Texture2DInternal::EncodeToPng(std::vector<uchar>& data) {
	return EncodeTo(data, ImageTypePng);
}

bool Texture2DInternal::EncodeToJpg(std::vector<uchar>& data) {
	return EncodeTo(data, ImageTypeJpg);
}

bool Texture2DInternal::EncodeTo(std::vector<uchar>& data, ImageType type) {
	BindTexture();

	TexelMap texelMap;
	texelMap.width = GetWidth();
	texelMap.height = GetHeight();
	texelMap.alignment = 4;
	BppType bpp = GLTextureFormatToBpp(format_);
	texelMap.data.resize((bpp / 8) * GetWidth() * GetHeight());
	GL::GetTexImage(GL_TEXTURE_2D, 0, format_, GL_UNSIGNED_BYTE, &texelMap.data[0]);
	UnbindTexture();

	texelMap.format = (bpp == BppType24) ? ColorFormatRgb : ColorFormatRgba;

	return ImageCodec::Encode(data, type, texelMap);
}

TextureCubeInternal::TextureCubeInternal() : TextureInternal(ObjectTypeTextureCube) {
}

TextureCubeInternal::~TextureCubeInternal() {
	DestroyTexture();
}

bool TextureCubeInternal::Load(const std::string(&textures)[6]) {
	DestroyTexture();

	GL::GenTextures(1, &texture_);

	BindTexture();

	for (int i = 0; i < 6; ++i) {
		TexelMap texelMap;
		if (!ImageCodec::Decode(texelMap, Path::GetResourceRootDirectory() + textures[i])) {
			DestroyTexture();
			return false;
		}

		GLenum glFormat[3];
		ColorFormatToGLTextureFormat(texelMap.format, glFormat);
		GL::TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glFormat[0], texelMap.width, texelMap.height, 0, glFormat[1], glFormat[2], &texelMap.data[0]);
		format_ = glFormat[0];
		GL::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		GL::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GL::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		GL::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GL::TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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

	GL::GenTextures(1, &texture_);
	BindTexture();

	GLenum glFormat[3];
	RenderTextureFormatToGLEnum(format, glFormat);
	GL::TexImage2D(GL_TEXTURE_2D, 0, glFormat[0], width, height, 0, glFormat[1], glFormat[2], nullptr);
	format_ = glFormat[0];

	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (format == RenderTextureFormatShadow) {
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
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
			internalFormat = GL_DEPTH_COMPONENT16;
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
			break;
		case RenderTextureFormatDepthStencil:
			internalFormat = GL_DEPTH32F_STENCIL8;
			format = GL_DEPTH_STENCIL;
			type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
			break;
		default:
			Debug::LogError("invalid render texture format: %d.", input);
			break;
	}

	parameters[0] = internalFormat;
	parameters[1] = format;
	parameters[2] = type;
}
