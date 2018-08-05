#include <glm/glm.hpp>

#include "rect.h"
#include "screen.h"
#include "buffer.h"
#include "resources.h"
#include "tools/math2.h"
#include "framebuffer.h"
#include "os/filesystem.h"
#include "../api/glutils.h"
#include "textureinternal.h"

TextureInternal::TextureInternal(ObjectType type) :ObjectInternal(type)
	, texture_(0), width_(0), height_(0), location_(0), internalFormat_(0) {
}

TextureInternal::~TextureInternal() {
	DestroyTexture();
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
		//GL::ActiveTexture(location_);
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

BPPType TextureInternal::GLenumToBpp(GLenum format) const {
	switch (format) {
		case GL_RGB: return BPPType24;
		case GL_RGBA: return BPPType32;
	}

	Debug::LogError("unknown internal format 0x%x.", format);
	return BPPType24;
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
		case TextureMinFilterMode::Nearest:  return GL_NEAREST;
		case TextureMinFilterMode::Linear: return GL_LINEAR;
		case TextureMinFilterMode::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case TextureMinFilterMode::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
		case TextureMinFilterMode::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
		case TextureMinFilterMode::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
	}

	Debug::LogError("invalid TextureMinFilterMode %d.", mode);
	return GL_LINEAR;
}

GLenum TextureInternal::TextureMagFilterModeToGLenum(TextureMagFilterMode mode) const {
	switch (mode) {
		case TextureMagFilterMode::Nearest: return GL_NEAREST;
		case TextureMagFilterMode::Linear: return GL_LINEAR;
	}

	Debug::LogError("invalid TextureMagFilterMode %d.", mode);
	return GL_LINEAR;
}

GLenum TextureInternal::TextureWrapModeToGLenum(TextureWrapMode mode) const {
	switch (mode) {
		case TextureWrapMode::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case TextureWrapMode::MirroredRepeat: return GL_MIRRORED_REPEAT;
		case TextureWrapMode::Repeat: return GL_REPEAT;
	}

	Debug::LogError("invalid TextureWrapMode %d.", mode);
	return GL_REPEAT;
}

TextureMinFilterMode TextureInternal::GLenumToTextureMinFilterMode(GLenum value) const {
	switch (value) {
		case GL_NEAREST: return TextureMinFilterMode::Nearest;
		case GL_LINEAR: return TextureMinFilterMode::Linear;
		case GL_NEAREST_MIPMAP_NEAREST: return TextureMinFilterMode::NearestMipmapNearest;
		case GL_LINEAR_MIPMAP_NEAREST: return TextureMinFilterMode::LinearMipmapNearest;
		case GL_NEAREST_MIPMAP_LINEAR: return TextureMinFilterMode::NearestMipmapLinear;
		case GL_LINEAR_MIPMAP_LINEAR: return TextureMinFilterMode::LinearMipmapLinear;
	}

	Debug::LogError("invalid GLenum %d.", value);
	return TextureMinFilterMode::Linear;
}

TextureMagFilterMode TextureInternal::GLenumToTextureMagFilterMode(GLenum value) const {
	switch (value) {
		case GL_NEAREST: return TextureMagFilterMode::Nearest;
		case GL_LINEAR: return TextureMagFilterMode::Linear;
	}

	Debug::LogError("invalid GLenum %d.", value);
	return TextureMagFilterMode::Linear;
}

TextureWrapMode TextureInternal::GLenumToTextureWrapMode(GLenum value) const {
	switch (value) {
		case GL_CLAMP_TO_EDGE: return TextureWrapMode::ClampToEdge;
		case GL_MIRRORED_REPEAT: return TextureWrapMode::MirroredRepeat;
		case GL_REPEAT: return TextureWrapMode::Repeat;
	}

	Debug::LogError("invalid GLenum %d.", value);
	return TextureWrapMode::Repeat;
}

Texture2DInternal::Texture2DInternal() : TextureInternal(ObjectTypeTexture2D) {
}

Texture2DInternal::~Texture2DInternal() {
}

bool Texture2DInternal::Load(const std::string& path) {
	TexelMap texelMap;
	if (!ImageCodec::Decode(texelMap, Resources::instance()->GetTextureDirectory() + path)) {
		return false;
	}

	return Load(texelMap.textureFormat, &texelMap.data[0], texelMap.colorStreamFormat, texelMap.width, texelMap.height, texelMap.alignment);
}

bool Texture2DInternal::Load(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap) {
	DestroyTexture();

	width_ = width;
	height_ = height;

	GL::GenTextures(1, &texture_);

	BindTexture();

	GLenum glFormat[2];
	ColorStreamFormatToGLenum(glFormat, format);
	GLenum internalFormat = TextureFormatToGLenum(textureFormat);

	GLUtils::PushGLMode(GLModeUnpackAlignment, alignment);
	GL::TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat[0], glFormat[1], data);
	GLUtils::PopGLMode(GLModeUnpackAlignment);

	if (mipmap) {
		GL::GenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		SetMinFilterMode(TextureMinFilterMode::Nearest);
	}

	UnbindTexture();

	internalFormat_ = internalFormat;

	return true;
}

bool Texture2DInternal::EncodeToPNG(std::vector<uchar>& data) {
	return EncodeTo(data, ImageTypePNG);
}

bool Texture2DInternal::EncodeToJPG(std::vector<uchar>& data) {
	return EncodeTo(data, ImageTypeJPG);
}

bool Texture2DInternal::EncodeTo(std::vector<uchar>& data, ImageType type) {
	BindTexture();

	TexelMap texelMap;
	texelMap.width = GetWidth();
	texelMap.height = GetHeight();
	
	uint alignment = 4;
	GL::GetIntegerv(GL_UNPACK_ALIGNMENT, (GLint*)&alignment);

	texelMap.alignment = alignment;
	BPPType bpp = GLenumToBpp(internalFormat_);

	texelMap.data.resize((bpp / 8) * Math::RoundUpToPowerOfTwo(GetWidth(), alignment) * GetHeight());
	GL::GetTexImage(GL_TEXTURE_2D, 0, internalFormat_, GL_UNSIGNED_BYTE, &texelMap.data[0]);
	UnbindTexture();

	texelMap.textureFormat = (bpp == BPPType24) ? TextureFormatRgb : TextureFormatRgba;
	texelMap.colorStreamFormat = (bpp == BPPType24) ? ColorStreamFormatRgb : ColorStreamFormatRgba;

	return ImageCodec::Encode(data, type, texelMap);
}

TextureCubeInternal::TextureCubeInternal() : TextureInternal(ObjectTypeTextureCube) {
}

TextureCubeInternal::~TextureCubeInternal() {
}

bool TextureCubeInternal::Load(const std::string(&textures)[6]) {
	TexelMap texelMaps[6];
	for (int i = 0; i < 6; ++i) {
		if (!ImageCodec::Decode(texelMaps[i], Resources::instance()->GetTextureDirectory() + textures[i])) {
			return false;
		}
	}

	DestroyTexture();

	GL::GenTextures(1, &texture_);
	BindTexture();

	for(int i = 0; i < 6; ++i) {
		GLenum glFormat[2];
		ColorStreamFormatToGLenum(glFormat, texelMaps[i].colorStreamFormat);
		GLenum internalFormat = TextureFormatToGLenum(texelMaps[i].textureFormat);
		GL::TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, texelMaps[i].width, texelMaps[i].height, 0, glFormat[0], glFormat[1], &texelMaps[i].data[0]);

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

RenderTexture RenderTexture::GetDefault() {
	static RenderTexture screen;
	
	if (!screen) {
		screen.reset(MEMORY_NEW(ScreenRenderTextureInternal));
		screen->Create(RenderTextureFormatRgb, 0, 0);
	}

	return screen;
}

RenderTexture RenderTexture::GetTemporary(uint width, uint height) {
	return nullptr;
}

RenderTextureInternal::RenderTextureInternal() 
	: bindStatus_(StatusNone), format_(RenderTextureFormatRgba) {
}

RenderTextureInternal::~RenderTextureInternal() {
	DestroyFramebuffer();
}

bool RenderTextureInternal::Create(RenderTextureFormat format, uint width, uint height) {
	DestroyTexture();
	DestroyFramebuffer();

	width_ = width;
	height_ = height;

	framebuffer_ = MEMORY_NEW(Framebuffer);

	GL::GenTextures(1, &texture_);
	BindTexture();

	format_ = format;
	ResizeStorage(width, height);

	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (format == RenderTextureFormatShadow) {
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	}

	UnbindTexture();

	if (!ContainsDepthInfo()) {
		framebuffer_->SetViewport(0, 0, width, height);
		framebuffer_->CreateDepthRenderbuffer();
		framebuffer_->SetRenderTexture(FramebufferAttachment0, texture_);
	}
	else {
		framebuffer_->SetDepthTexture(texture_);
	}

	return true;
}

void RenderTextureInternal::Clear(const Rect& normalizedRect, const glm::vec4& value) {
	if (!SetViewport(width_, height_, normalizedRect)) {
		return;
	}

	if (ContainsDepthInfo()) {
		framebuffer_->SetClearDepth(value.w);
		framebuffer_->Clear(FramebufferClearMaskDepth);
	}
	else {
		framebuffer_->SetClearDepth(value.w);
		framebuffer_->SetClearColor(glm::vec3(value));
		framebuffer_->Clear(FramebufferClearMaskColorDepth);
	}
}

void RenderTextureInternal::Resize(uint width, uint height) {
	if (width_ != width || height_ != height) {
		BindTexture();
		ResizeStorage(width, height);
		UnbindTexture();
	}
}

void RenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	bindStatus_ = StatusWrite;

	SetViewport(width_, height_, normalizedRect);
	framebuffer_->BindWrite();
}

void RenderTextureInternal::Bind(uint index) {
	bindStatus_ = StatusRead;
	TextureInternal::Bind(index);
}

void RenderTextureInternal::Unbind() {
	if (bindStatus_ == StatusWrite) {
		framebuffer_->Unbind();
	}
	else {
		TextureInternal::Unbind();
	}

	bindStatus_ = StatusNone;
}

void RenderTextureInternal::DestroyFramebuffer() {
	if (framebuffer_ != nullptr) {
		MEMORY_DELETE(framebuffer_);
		framebuffer_ = nullptr;
	}
}

void RenderTextureInternal::ResizeStorage(uint w, uint h) {
	GLenum glFormat[3];
	RenderTextureFormatToGLenum(format_, glFormat);
	GL::TexImage2D(GL_TEXTURE_2D, 0, glFormat[0], w, h, 0, glFormat[1], glFormat[2], nullptr);
	width_ = w;
	height_ = h;
	internalFormat_ = glFormat[0];
}

void RenderTextureInternal::RenderTextureFormatToGLenum(RenderTextureFormat input, GLenum(&parameters)[3]) {
	GLenum internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	switch (input) {
		case RenderTextureFormatRgb:
			internalFormat = GL_RGB;
			break;
		case RenderTextureFormatRgbSN:
			internalFormat = GL_RGB_SNORM;
			break;
		case  RenderTextureFormatRgba:
			internalFormat = GL_RGBA;
			break;
		case RenderTextureFormatRgbaSN:
			internalFormat = GL_RGBA_SNORM;
			break;
		case RenderTextureFormatRgbHDR:
			internalFormat = GL_RGB32F;
			format = GL_RGB;
			type = GL_FLOAT;
			break;
		case RenderTextureFormatRgbaHDR:
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

TextureBufferInternal::TextureBufferInternal() : TextureInternal(ObjectTypeTextureBuffer), buffer_(nullptr) {
}

TextureBufferInternal::~TextureBufferInternal() {
	DestroyBuffer();
}

bool TextureBufferInternal::Create(uint size) {
	DestroyBuffer();
	DestroyTexture();

	buffer_ = MEMORY_NEW(Buffer);
	buffer_->Create(GL_TEXTURE_BUFFER, size, nullptr, GL_STREAM_DRAW);
	
	GL::GenTextures(1, &texture_);
	BindTexture();
	GL::TexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_->GetNativePointer());
	UnbindTexture();

	return true;
}

uint TextureBufferInternal::GetSize() const {
	return buffer_->GetSize();
}

void TextureBufferInternal::Update(uint offset, uint size, const void* data) {
	buffer_->Update(offset, size, data);
}

inline void TextureBufferInternal::DestroyBuffer() {
	if (buffer_ != nullptr) {
		MEMORY_DELETE(buffer_);
		buffer_ = nullptr;
	}
}

#define LogUnsupportedRenderTextureOperation()	Debug::LogError("unsupported render texture operation %s.", __func__);

ScreenRenderTextureInternal::ScreenRenderTextureInternal() {
}

bool ScreenRenderTextureInternal::Create(RenderTextureFormat format, uint width, uint height) {
	framebuffer_ = Framebuffer0::Get();
	return true;
}

void ScreenRenderTextureInternal::Clear(const Rect& normalizedRect, const glm::vec4& value) {
	SetViewport(Screen::instance()->GetWidth(), Screen::instance()->GetHeight(), normalizedRect);

	framebuffer_->SetClearDepth(value.w);
	framebuffer_->SetClearColor(glm::vec3(value));
	framebuffer_->Clear(FramebufferClearMaskColorDepth);
}

uint ScreenRenderTextureInternal::GetWidth() const {
	return Screen::instance()->GetWidth();
}

uint ScreenRenderTextureInternal::GetHeight() const {
	return Screen::instance()->GetHeight();
}

GLenum ScreenRenderTextureInternal::GetGLTextureType() const {
	LogUnsupportedRenderTextureOperation();
	return 0;
}

GLenum ScreenRenderTextureInternal::GetGLTextureBindingName() const {
	LogUnsupportedRenderTextureOperation();
	return 0;
}

void ScreenRenderTextureInternal::Resize(uint w, uint h) {
	LogUnsupportedRenderTextureOperation();
}

void ScreenRenderTextureInternal::Bind(uint index) {
	LogUnsupportedRenderTextureOperation();
}

void ScreenRenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	SetViewport(Screen::instance()->GetWidth(), Screen::instance()->GetHeight(), normalizedRect);
	framebuffer_->BindWrite();
}

void ScreenRenderTextureInternal::Unbind() {
	framebuffer_->Unbind();
}

bool RenderTextureInternalBase::SetViewport(uint width, uint height, const Rect& normalizedRect) {
	Rect viewport = Rect::NormalizedToRect(Rect(0.f, 0.f, (float)width, (float)height), normalizedRect);
	framebuffer_->SetViewport((uint)viewport.GetXMin(), (uint)viewport.GetYMin(), (uint)viewport.GetWidth(), (uint)viewport.GetHeight());
	return (uint)viewport.GetWidth() > 0 && (uint)viewport.GetHeight() > 0;
}
