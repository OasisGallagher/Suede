#include <glm/glm.hpp>

#include "tools/path.h"
#include "debug/debug.h"
#include "textureinternal.h"

TextureInternal::TextureInternal(ObjectType type) :ObjectInternal(type)
	, texture_(0), width_(0), height_(0), location_(0), internalFormat_(0) {
}

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

void TextureInternal::SetMinFilterMode(TextureMinFilterMode value) {
	BindTexture();
	GL::TexParameteri(GetGLTextureType(), GL_TEXTURE_MIN_FILTER, TextureMinFilterModeToGLenum(value));
	UnbindTexture();
}

TextureMinFilterMode TextureInternal::GetMinFilterMode() const {
	BindTexture();
	GLint parameter = 0;
	GL::GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_MIN_FILTER, &parameter);
	UnbindTexture();
	return GLenumToTextureMinFilterMode(parameter);
}

void TextureInternal::SetMagFilterMode(TextureMagFilterMode value) {
	BindTexture();
	GL::TexParameteri(GetGLTextureType(), GL_TEXTURE_MAG_FILTER, TextureMagFilterModeToGLenum(value));
	UnbindTexture();
}

TextureMagFilterMode TextureInternal::GetMagFilterMode() const {
	BindTexture();
	GLint parameter = 0;
	GL::GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_MAG_FILTER, &parameter);
	UnbindTexture();
	return GLenumToTextureMagFilterMode(parameter);
}

void TextureInternal::SetWrapModeS(TextureWrapMode value) {
	BindTexture();
	GL::TexParameteri(GetGLTextureType(), GL_TEXTURE_WRAP_S, TextureWrapModeToGLenum(value));
	UnbindTexture();
}

TextureWrapMode TextureInternal::GetWrapModeS() const {
	BindTexture();
	GLint parameter = 0;
	GL::GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_WRAP_S, &parameter);
	UnbindTexture();
	return GLenumToTextureWrapMode(parameter);
}

void TextureInternal::SetWrapModeT(TextureWrapMode value) {
	BindTexture();
	GL::TexParameteri(GetGLTextureType(), GL_TEXTURE_WRAP_T, TextureWrapModeToGLenum(value));
	UnbindTexture();
}

TextureWrapMode TextureInternal::GetWrapModeT() const {
	BindTexture();
	GLint parameter = 0;
	GL::GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_WRAP_T, &parameter);
	UnbindTexture();
	return GLenumToTextureWrapMode(parameter);
}

void TextureInternal::BindTexture() const {
	GL::GetIntegerv(GetGLTextureBindingName(), &oldBindingTexture_);
	GL::BindTexture(GetGLTextureType(), texture_);
}

void TextureInternal::UnbindTexture() const {
	GL::BindTexture(GetGLTextureType(), oldBindingTexture_);
	oldBindingTexture_ = 0;
}

void TextureInternal::DestroyTexture() {
	if (texture_ != 0) {
		GL::DeleteTextures(1, &texture_);
		texture_ = 0;
	}
}

BppType TextureInternal::GLenumToBpp(GLenum format) const {
	switch (format) {
		case GL_RGB: return BppType24;
		case GL_RGBA: return BppType32;
	}

	Debug::LogError("unknown internal format 0x%x.", format);
	return BppType24;
}

GLenum TextureInternal::TextureFormatToGLenum(TextureFormat textureFormat) const {
	switch (textureFormat) {
		case TextureFormatRgb: return GL_RGB;
		case TextureFormatRgb16: return GL_RGB16F;
		case TextureFormatRgb32: return GL_RGB32F;
		case TextureFormatRgba: return GL_RGBA;
		case TextureFormatRgba16: return GL_RGBA16F;
		case TextureFormatRgba32: return GL_RGBA32F;
		case TextureFormatRgbaS: return GL_RGBA_SNORM;
		case TextureFormatRgbS: return GL_RGB_SNORM;
	}

	Debug::LogError("invalid texture format %d.", textureFormat);
	return GL_RGB;
}

void TextureInternal::ColorStreamFormatToGLenum(GLenum(&parameters)[2], ColorStreamFormat format) const {
	GLenum glFormat = GL_RGBA, glType = GL_UNSIGNED_BYTE;
	switch (format) {
		case ColorStreamFormatRgb:
			glFormat = GL_RGB;
			break;
		case ColorStreamFormatBgr:
			glFormat = GL_BGR;
			break;
		case ColorStreamFormatRgba:
			glFormat = GL_RGBA;
			break;
		case ColorStreamFormatArgb:
			glFormat = GL_BGRA;
			glType = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;
		case ColorStreamFormatBgra:
			glFormat = GL_BGRA;
			break;
		case ColorStreamFormatLuminanceAlpha:
			glFormat = GL_LUMINANCE_ALPHA;
			break;
	}

	parameters[0] = glFormat;
	parameters[1] = glType;
}

GLenum TextureInternal::TextureMinFilterModeToGLenum(TextureMinFilterMode mode) const {
	switch (mode) {
		case TextureMinFilterModeNearest:  return GL_NEAREST;
		case TextureMinFilterModeLinear: return GL_LINEAR;
		case TextureMinFilterModeNearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case TextureMinFilterModeLinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
		case TextureMinFilterModeNearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
		case TextureMinFilterModeLinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
	}

	Debug::LogError("invalid TextureMinFilterMode %d.", mode);
	return GL_LINEAR;
}

GLenum TextureInternal::TextureMagFilterModeToGLenum(TextureMagFilterMode mode) const {
	switch (mode) {
		case TextureMagFilterModeNearest: return GL_NEAREST;
		case TextureMagFilterModeLinear: return GL_LINEAR;
	}

	Debug::LogError("invalid TextureMagFilterMode %d.", mode);
	return GL_LINEAR;
}

GLenum TextureInternal::TextureWrapModeToGLenum(TextureWrapMode mode) const {
	switch (mode) {
		case TextureWrapModeClampToEdge: return GL_CLAMP_TO_EDGE;
		case TextureWrapModeMirroredRepeat: return GL_MIRRORED_REPEAT;
		case TextureWrapModeRepeat: return GL_REPEAT;
	}

	Debug::LogError("invalid TextureWrapMode %d.", mode);
	return GL_REPEAT;
}

TextureMinFilterMode TextureInternal::GLenumToTextureMinFilterMode(GLenum value) const {
	switch (value) {
		case GL_NEAREST: return TextureMinFilterModeNearest;
		case GL_LINEAR: return TextureMinFilterModeLinear;
		case GL_NEAREST_MIPMAP_NEAREST: return TextureMinFilterModeNearestMipmapNearest;
		case GL_LINEAR_MIPMAP_NEAREST: return TextureMinFilterModeLinearMipmapNearest;
		case GL_NEAREST_MIPMAP_LINEAR: return TextureMinFilterModeNearestMipmapLinear;
		case GL_LINEAR_MIPMAP_LINEAR: return TextureMinFilterModeLinearMipmapLinear;
	}

	Debug::LogError("invalid GLenum %d.", value);
	return TextureMinFilterModeLinear;
}

TextureMagFilterMode TextureInternal::GLenumToTextureMagFilterMode(GLenum value) const {
	switch (value) {
		case GL_NEAREST: return TextureMagFilterModeNearest;
		case GL_LINEAR: return TextureMagFilterModeLinear;
	}

	Debug::LogError("invalid GLenum %d.", value);
	return TextureMagFilterModeLinear;
}

TextureWrapMode TextureInternal::GLenumToTextureWrapMode(GLenum value) const {
	switch (value) {
		case GL_CLAMP_TO_EDGE: return TextureWrapModeClampToEdge;
		case GL_MIRRORED_REPEAT: return TextureWrapModeMirroredRepeat;
		case GL_REPEAT: return TextureWrapModeRepeat;
	}

	Debug::LogError("invalid GLenum %d.", value);
	return TextureWrapModeRepeat;
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

	return Load(texelMap.textureFormat, &texelMap.data[0], texelMap.format, texelMap.width, texelMap.height);
}

// TODO: assume UNPACK_ALIGNMENT = 4.
bool Texture2DInternal::Load(TextureFormat textureFormat, const void* data, ColorStreamFormat format, int width, int height, bool mipmap) {
	DestroyTexture();

	width_ = width;
	height_ = height;

	GL::GenTextures(1, &texture_);

	BindTexture();

	GLenum glFormat[2];
	ColorStreamFormatToGLenum(glFormat, format);
	GLenum internalFormat = TextureFormatToGLenum(textureFormat);
	GL::TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat[0], glFormat[1], data);

	if (mipmap) {
		GL::GenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		SetMinFilterMode(TextureMinFilterModeNearest);
	}

	UnbindTexture();

	internalFormat_ = internalFormat;

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
	BppType bpp = GLenumToBpp(internalFormat_);
	texelMap.data.resize((bpp / 8) * GetWidth() * GetHeight());
	GL::GetTexImage(GL_TEXTURE_2D, 0, internalFormat_, GL_UNSIGNED_BYTE, &texelMap.data[0]);
	UnbindTexture();

	texelMap.textureFormat = (bpp == BppType24) ? TextureFormatRgb : TextureFormatRgba;
	texelMap.format = (bpp == BppType24) ? ColorStreamFormatRgb : ColorStreamFormatRgba;

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

		GLenum glFormat[2];
		ColorStreamFormatToGLenum(glFormat, texelMap.format);
		GLenum internalFormat = TextureFormatToGLenum(texelMap.textureFormat);
		GL::TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, texelMap.width, texelMap.height, 0, glFormat[0], glFormat[1], &texelMap.data[0]);

		internalFormat_ = internalFormat;

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
	RenderTextureFormatToGLenum(format, glFormat);
	GL::TexImage2D(GL_TEXTURE_2D, 0, glFormat[0], width, height, 0, glFormat[1], glFormat[2], nullptr);
	internalFormat_ = glFormat[0];

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

void RenderTextureInternal::RenderTextureFormatToGLenum(RenderTextureFormat input, GLenum(&parameters)[3]) {
	GLenum internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	switch (input) {
		case  RenderTextureFormatRgba:
			internalFormat = GL_RGBA;
			break;
		case RenderTextureFormatRgbaSn:
			internalFormat = GL_RGBA_SNORM;
			break;
		case RenderTextureFormatRgbHdr:
			internalFormat = GL_RGB32F;
			format = GL_RGB;
			type = GL_FLOAT;
			break;
		case RenderTextureFormatRgbaHdr:
			internalFormat = GL_RGBA32F;
			type = GL_FLOAT;
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
