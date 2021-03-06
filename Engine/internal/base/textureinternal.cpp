#include "textureinternal.h"

#include "rect.h"
#include "screen.h"
#include "buffer.h"
#include "context.h"
#include "resources.h"
#include "math/mathf.h"
#include "os/filesystem.h"

#include "containers/freelist.h"

Texture::Texture(void* d) : Object(d) {}
void Texture::Bind(uint index) { _suede_dptr()->Bind(index); }
void Texture::Unbind() { _suede_dptr()->Unbind(); }
uint Texture::GetNativePointer() { return _suede_dptr()->GetNativePointer(); }
void Texture::SetMinFilterMode(TextureMinFilterMode value) { _suede_dptr()->SetMinFilterMode(value); }
TextureMinFilterMode Texture::GetMinFilterMode() const { return _suede_dptr()->GetMinFilterMode(); }
void Texture::SetMagFilterMode(TextureMagFilterMode value) { _suede_dptr()->SetMagFilterMode(value); }
TextureMagFilterMode Texture::GetMagFilterMode() const { return _suede_dptr()->GetMagFilterMode(); }
void Texture::SetWrapModeS(TextureWrapMode value) { _suede_dptr()->SetWrapModeS(value); }
TextureWrapMode Texture::GetWrapModeS() const { return _suede_dptr()->GetWrapModeS(); }
void Texture::SetWrapModeT(TextureWrapMode value) { _suede_dptr()->SetWrapModeT(value); }
TextureWrapMode Texture::GetWrapModeT() const { return _suede_dptr()->GetWrapModeT(); }
uint Texture::GetWidth() const { return _suede_dptr()->GetWidth(); }
uint Texture::GetHeight() const { return _suede_dptr()->GetHeight(); }

Texture2D::Texture2D() : Texture(new Texture2DInternal(Context::GetCurrent())) {}
Texture2D* Texture2D::GetWhiteTexture() {
	static ref_ptr<Texture2D> texture;
	if (!texture) {
		uint color = 0xffffffff;
		texture = new Texture2D();
		texture->Create(TextureFormat::Rgba, &color, ColorStreamFormat::Rgba, 1, 1, 4);
	}

	return texture.get();
}

Texture2D* Texture2D::GetBlackTexture() {
	static ref_ptr<Texture2D> texture;
	if (!texture) {
		uint color = 0xff000000;
		texture = new Texture2D();
		texture->Create(TextureFormat::Rgba, &color, ColorStreamFormat::Rgba, 1, 1, 4);
	}

	return texture.get();
}

bool Texture2D::Load(const std::string& path) { return _suede_dptr()->Load(path); }
bool Texture2D::Create(TextureFormat textureFormat, const void* data, ColorStreamFormat colorStreamFormat, uint width, uint height, uint alignment, bool mipmap) {
	return _suede_dptr()->Create(textureFormat, data, colorStreamFormat, width, height, alignment, mipmap);
}
TextureFormat Texture2D::GetFormat() { return _suede_dptr()->GetFormat(); }
bool Texture2D::EncodeToPNG(std::vector<uchar>& data) { return _suede_dptr()->EncodeToPNG(data); }
bool Texture2D::EncodeToJPG(std::vector<uchar>& data) { return _suede_dptr()->EncodeToJPG(data); }

TextureCube::TextureCube() : Texture(new TextureCubeInternal(Context::GetCurrent())) {}
bool TextureCube::Load(const std::string textures[6]) { return _suede_dptr()->Load(textures); }

TextureBuffer::TextureBuffer() : Texture(new TextureBufferInternal(Context::GetCurrent())) {}
uint TextureBuffer::GetSize() const { return _suede_dptr()->GetSize(); }
bool TextureBuffer::Create(uint size) { return _suede_dptr()->Create(size); }
void TextureBuffer::Update(uint offset, uint size, const void* data) { _suede_dptr()->Update(offset, size, data); }

RenderTexture::RenderTexture() : Texture(new RenderTextureInternal(Context::GetCurrent())) {}
RenderTexture::RenderTexture(void* d) : Texture(d) {}
bool RenderTexture::Create(RenderTextureFormat format, uint width, uint height) { return _suede_dptr()->Create(format, width, height); }
void RenderTexture::__tmpApplyClear() { _suede_dptr()->__tmpApplyClear(); }
RenderTextureFormat RenderTexture::GetRenderTextureFormat() { return _suede_dptr()->GetRenderTextureFormat(); }
void RenderTexture::Resize(uint width, uint height) { _suede_dptr()->Resize(width, height); }
void RenderTexture::Clear(const Rect& normalizedRect, const Color& color, float depth, int stencil) { _suede_dptr()->Clear(normalizedRect, color, depth, stencil); }
void RenderTexture::BindWrite(const Rect& normalizedRect) { _suede_dptr()->BindWrite(normalizedRect); }
RenderTexture* RenderTexture::GetDefault() {
	static ref_ptr<RenderTexture> screen;

	if (!screen) {
		screen = new ScreenRenderTexture();
		screen->Create(RenderTextureFormat::Rgb, 0, 0);
	}

	return screen.get();
}

struct RenderTextureCacheKey {
	int busy_;
	RenderTextureFormat format;
	uint width;
	uint height;

	bool operator<(const RenderTextureCacheKey& other) const {
		if (busy_ != other.busy_) { return busy_ < other.busy_; }
		if (format != other.format) { return format < other.format; }
		if (width != other.width) { return width < other.width; }
		return height < other.height;
	}
};

typedef std::multimap<RenderTextureCacheKey, ref_ptr<RenderTexture>> RenderTextureCacheContainer;
static RenderTextureCacheContainer renderTextureCache;

static RenderTexture* ToggleRenderTextureBusyState(RenderTextureFormat format, uint width, uint height, int busy_) {
	RenderTextureCacheKey key = { busy_, format, width, height };
	RenderTextureCacheContainer::iterator pos = renderTextureCache.find(key);
	if (pos != renderTextureCache.end()) {
		ref_ptr<RenderTexture> value = pos->second;
		renderTextureCache.erase(pos);
		key.busy_ = !busy_;
		renderTextureCache.insert(std::make_pair(key, value));

		return value.get();
	}

	return nullptr;
}

RenderTexture* RenderTexture::GetTemporary(RenderTextureFormat format, uint width, uint height) {
	RenderTexture* cache = ToggleRenderTextureBusyState(format, width, height, 0);
	if (cache != nullptr) {
		cache->Clear(Rect::unit, Color::black, 1, 1);
		return cache;
	}

	RenderTexture* texture = new RenderTexture();
	texture->Create(format, width, height);
	renderTextureCache.insert(std::make_pair(RenderTextureCacheKey{ 1, format, width,height }, texture));
	return texture;
}

MRTRenderTexture::MRTRenderTexture() : RenderTexture(new MRTRenderTextureInternal(Context::GetCurrent())) {}
bool MRTRenderTexture::AddColorTexture(TextureFormat format) { return _suede_dptr()->AddColorTexture(format); }
Texture2D* MRTRenderTexture::GetColorTexture(uint index) { return _suede_dptr()->GetColorTexture(index); }
uint MRTRenderTexture::GetColorTextureCount() { return _suede_dptr()->GetColorTextureCount(); }

ScreenRenderTexture::ScreenRenderTexture() : RenderTexture(new ScreenRenderTextureInternal(Context::GetCurrent())) {}

TextureInternal::TextureInternal(ObjectType type, Context* context) : ObjectInternal(type), GLObjectMaintainer(context) {
}

TextureInternal::~TextureInternal() {
	DestroyTexture();
}

void TextureInternal::Bind(uint index) {
	if (!context_->IsTexture(texture_)) {
		Debug::LogError("invalid texture");
		return;
	}

	location_ = index + GL_TEXTURE0;
	context_->ActiveTexture(location_);
	BindTexture();

	if (SupportsSampler() && samplerDirty_) {
		ApplySampler();
	}
}

void TextureInternal::Unbind() {
	if (location_ != 0) {
		UnbindTexture();
		location_ = 0;
	}
}

uint TextureInternal::GetNativePointer() {
	if (texture_ == 0) {
		Debug::LogWarning("Texture does not initialized");
	}

	return texture_;
}

void TextureInternal::SetMinFilterMode(TextureMinFilterMode value) {
	if (sampler_.minFilter != value) {
		sampler_.minFilter = value;
		samplerDirty_ = true;
	}
}

void TextureInternal::SetMagFilterMode(TextureMagFilterMode value) {
	if (sampler_.magFilter != value) {
		sampler_.magFilter = value;
		samplerDirty_ = true;
	}
}

void TextureInternal::SetWrapModeS(TextureWrapMode value) {
	if (sampler_.wrapS != value) {
		sampler_.wrapS = value;
		samplerDirty_ = true;
	}
}

void TextureInternal::SetWrapModeT(TextureWrapMode value) {
	if (sampler_.wrapT != value) {
		sampler_.wrapT = value;
		samplerDirty_ = true;
	}
}

void TextureInternal::BindTexture() const {
	context_->GetIntegerv(GetGLTextureBindingName(), &oldBindingTexture_);
	context_->BindTexture(GetGLTextureType(), texture_);
}

void TextureInternal::UnbindTexture() const {
	context_->BindTexture(GetGLTextureType(), oldBindingTexture_);
	oldBindingTexture_ = 0;
}

void TextureInternal::DestroyTexture() {
	if (texture_ != 0) {
		context_->DeleteTextures(1, &texture_);
		texture_ = 0;
	}
}

void TextureInternal::ApplySampler() {
	uint type = GetGLTextureType();
	context_->TexParameteri(type, GL_TEXTURE_MIN_FILTER, TextureMinFilterModeToGLenum(sampler_.minFilter));
	context_->TexParameteri(type, GL_TEXTURE_MAG_FILTER, TextureMagFilterModeToGLenum(sampler_.magFilter));
	context_->TexParameteri(type, GL_TEXTURE_WRAP_S, TextureWrapModeToGLenum(sampler_.wrapS));
	context_->TexParameteri(type, GL_TEXTURE_WRAP_T, TextureWrapModeToGLenum(sampler_.wrapT));

	samplerDirty_ = false;
}

void TextureInternal::OnContextDestroyed() {
	DestroyTexture();
	GLObjectMaintainer::OnContextDestroyed();
}

uint TextureInternal::TextureFormatToGLenum(TextureFormat textureFormat) const {
	switch (textureFormat) {
		case TextureFormat::Rgb: return GL_RGB;
		case TextureFormat::Rgb16F: return GL_RGB16F;
		case TextureFormat::Rgb32F: return GL_RGB32F;
		case TextureFormat::Rgba: return GL_RGBA;
		case TextureFormat::Rgba16F: return GL_RGBA16F;
		case TextureFormat::Rgba32F: return GL_RGBA32F;
		case TextureFormat::RgbaS: return GL_RGBA_SNORM;
		case TextureFormat::RgbS: return GL_RGB_SNORM;
	}

	Debug::LogError("invalid texture format %d.", textureFormat);
	return GL_RGB;
}

void TextureInternal::ColorStreamFormatToGLenum(uint& glFormat, uint& glType, ColorStreamFormat format) const {
	glFormat = GL_RGBA;
	glType = GL_UNSIGNED_BYTE;

	switch (format) {
		case ColorStreamFormat::Rgb:
			glFormat = GL_RGB;
			break;
		case ColorStreamFormat::RgbF:
			glFormat = GL_RGB;
			glType = GL_FLOAT;
			break;
		case ColorStreamFormat::Bgr:
			glFormat = GL_BGR;
			break;
		case ColorStreamFormat::Rgba:
			glFormat = GL_RGBA;
			break;
		case ColorStreamFormat::RgbaF:
			glFormat = GL_RGBA;
			glType = GL_FLOAT;
			break; 
		case ColorStreamFormat::Argb:
			glFormat = GL_BGRA;
			glType = GL_UNSIGNED_INT_8_8_8_8_REV;
			break;
		case ColorStreamFormat::Bgra:
			glFormat = GL_BGRA;
			break;
		case ColorStreamFormat::LuminanceAlpha:
			glFormat = GL_LUMINANCE_ALPHA;
			break;
	}
}

uint TextureInternal::TextureMinFilterModeToGLenum(TextureMinFilterMode mode) const {
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

uint TextureInternal::TextureMagFilterModeToGLenum(TextureMagFilterMode mode) const {
	switch (mode) {
		case TextureMagFilterMode::Nearest: return GL_NEAREST;
		case TextureMagFilterMode::Linear: return GL_LINEAR;
	}

	Debug::LogError("invalid TextureMagFilterMode %d.", mode);
	return GL_LINEAR;
}

uint TextureInternal::TextureWrapModeToGLenum(TextureWrapMode mode) const {
	switch (mode) {
		case TextureWrapMode::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case TextureWrapMode::MirroredRepeat: return GL_MIRRORED_REPEAT;
		case TextureWrapMode::Repeat: return GL_REPEAT;
	}

	Debug::LogError("invalid TextureWrapMode %d.", mode);
	return GL_REPEAT;
}

TextureMinFilterMode TextureInternal::GLenumToTextureMinFilterMode(uint value) const {
	switch (value) {
		case GL_NEAREST: return TextureMinFilterMode::Nearest;
		case GL_LINEAR: return TextureMinFilterMode::Linear;
		case GL_NEAREST_MIPMAP_NEAREST: return TextureMinFilterMode::NearestMipmapNearest;
		case GL_LINEAR_MIPMAP_NEAREST: return TextureMinFilterMode::LinearMipmapNearest;
		case GL_NEAREST_MIPMAP_LINEAR: return TextureMinFilterMode::NearestMipmapLinear;
		case GL_LINEAR_MIPMAP_LINEAR: return TextureMinFilterMode::LinearMipmapLinear;
	}

	Debug::LogError("invalid uint %d.", value);
	return TextureMinFilterMode::Linear;
}

TextureMagFilterMode TextureInternal::GLenumToTextureMagFilterMode(uint value) const {
	switch (value) {
		case GL_NEAREST: return TextureMagFilterMode::Nearest;
		case GL_LINEAR: return TextureMagFilterMode::Linear;
	}

	Debug::LogError("invalid uint %d.", value);
	return TextureMagFilterMode::Linear;
}

TextureWrapMode TextureInternal::GLenumToTextureWrapMode(uint value) const {
	switch (value) {
		case GL_CLAMP_TO_EDGE: return TextureWrapMode::ClampToEdge;
		case GL_MIRRORED_REPEAT: return TextureWrapMode::MirroredRepeat;
		case GL_REPEAT: return TextureWrapMode::Repeat;
	}

	Debug::LogError("invalid uint %d.", value);
	return TextureWrapMode::Repeat;
}

Texture2DInternal::Texture2DInternal(Context* context) : TextureInternal(ObjectType::Texture2D, context) {
}

Texture2DInternal::~Texture2DInternal() {
}

bool Texture2DInternal::Load(const std::string& path) {
	RawImage rawImage;
	if (!ImageCodec::Decode(rawImage, Resources::textureDirectory + path)) {
		return false;
	}

	return Create(rawImage.textureFormat, rawImage.pixels.data(), rawImage.colorStreamFormat, rawImage.width, rawImage.height, rawImage.alignment);
}

bool Texture2DInternal::Create(TextureFormat textureFormat, const void* data, ColorStreamFormat colorStreamFormat, uint width, uint height, uint alignment, bool mipmap) {
	width_ = width;
	height_ = height;

	ColorStreamFormatToGLenum(glFormat_, glType_, colorStreamFormat);

	mipmap_ = mipmap;
	internalFormat_ = TextureFormatToGLenum(textureFormat);

	alignment_ = alignment;
	textureFormat_ = textureFormat;
	colorStreamFormat_ = colorStreamFormat;
	
	shadowDataSize_ = ColorStreamBytes(colorStreamFormat_) * Mathf::RoundUpToPowerOfTwo(GetWidth(), alignment) * GetHeight();

	if (data != nullptr) {
		shadowData_.reset(new uchar[shadowDataSize_]);
		memcpy(shadowData_.get(), data, shadowDataSize_);
	}
	else {
		shadowData_ = nullptr;
	}

	dataDirty_ = true;

	return true;
}

bool Texture2DInternal::EncodeToPNG(std::vector<uchar>& data) {
	return EncodeTo(data, ImageType::PNG);
}

bool Texture2DInternal::EncodeToJPG(std::vector<uchar>& data) {
	return EncodeTo(data, ImageType::JPG);
}

void Texture2DInternal::Bind(uint index) {
	if (dataDirty_) {
		ApplyData();
	}

	TextureInternal::Bind(index);
}

bool Texture2DInternal::EncodeTo(std::vector<uchar>& data, ImageType type) {
	RawImage rawImage;
	rawImage.width = width_;
	rawImage.height = height_;
	rawImage.alignment = alignment_;
	rawImage.textureFormat = textureFormat_;
	rawImage.colorStreamFormat = colorStreamFormat_;
	rawImage.pixels.assign(shadowData_.get(), shadowData_.get() + shadowDataSize_);

	return ImageCodec::Encode(data, type, rawImage);
}

void Texture2DInternal::ApplyData() {
	if (texture_ == 0) {
		context_->GenTextures(1, &texture_);
	}

	BindTexture();

	int oldUnpackAlignment = 4;
	context_->GetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	context_->PixelStorei(GL_UNPACK_ALIGNMENT, alignment_);

	context_->TexImage2D(GL_TEXTURE_2D, 0, internalFormat_, width_, height_, 0, glFormat_, glType_, shadowData_.get());

	context_->PixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);

	if (mipmap_) {
		context_->GenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		SetMinFilterMode(TextureMinFilterMode::Nearest);
	}

	UnbindTexture();
	dataDirty_ = false;
}

TextureCubeInternal::TextureCubeInternal(Context* context) : TextureInternal(ObjectType::TextureCube, context) {
}

TextureCubeInternal::~TextureCubeInternal() {
}

bool TextureCubeInternal::Load(const std::string textures[6]) {
	std::unique_ptr<RawImage[]> images(new RawImage[6]);
	for (int i = 0; i < 6; ++i) {
		if (!ImageCodec::Decode(images[i], Resources::textureDirectory + textures[i])) {
			return false;
		}
	}

	rawImages_ = std::move(images);

	return true;
}

void TextureCubeInternal::Bind(uint index) {
	if (rawImages_) {
		ApplyContent();
	}

	TextureInternal::Bind(index);
}

void TextureCubeInternal::ApplyContent() {
	DestroyTexture();

	context_->GenTextures(1, &texture_);
	BindTexture();

	for (int i = 0; i < 6; ++i) {
		uint glFormat, glType;
		ColorStreamFormatToGLenum(glFormat, glType, rawImages_[i].colorStreamFormat);
		uint internalFormat = TextureFormatToGLenum(rawImages_[i].textureFormat);
		context_->TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, rawImages_[i].width, rawImages_[i].height, 0, glFormat, glType, rawImages_[i].pixels.data());

		internalFormat_ = internalFormat;

		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	UnbindTexture();

	rawImages_ = nullptr;
}

void RenderTexture::ReleaseTemporary(RenderTexture* texture) {
	RenderTexture* cache = ToggleRenderTextureBusyState(texture->GetRenderTextureFormat(), texture->GetWidth(), texture->GetHeight(), 1);
	SUEDE_ASSERT(cache != nullptr);
}

RenderTextureInternal::RenderTextureInternal(Context* context) : TextureInternal(ObjectType::RenderTexture, context) {
}

RenderTextureInternal::~RenderTextureInternal() {
	DestroyFramebuffer();
}

bool RenderTextureInternal::Create(RenderTextureFormat format, uint width, uint height) {
	width_ = width;
	height_ = height;
	renderTextureFormat_ = format;
	configDirty_ = true;

	return true;
}

void RenderTextureInternal::Clear(const Rect& normalizedRect, const Color& color, float depth, int stencil) {
	clearArgument_.normalizedRect = normalizedRect;
	clearArgument_.color = color;
	clearArgument_.depth = depth;
	clearArgument_.stencil = stencil;
	clearArgument_.dirty = true;
}

void RenderTextureInternal::Resize(uint width, uint height) {
	if (width_ != width || height_ != height) {
		width_ = width;
		height_ = height;

		sizeDirty_ = true;
	}
}

void RenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	if (VerifyBindStatus()) {
		if (configDirty_) { ApplyConfig(); }
		else {
			if (sizeDirty_) { ApplySize(); }
			if (clearArgument_.dirty) { ApplyClearContent(); }
		}

		bindStatus_ = StatusWrite;
		SetViewport(Rect::NormalizedToRect(Rect(0.f, 0.f, (float)width_, (float)height_), normalizedRect));
		framebuffer_->BindWrite();
	}
}

void RenderTextureInternal::Bind(uint index) {
	if (VerifyBindStatus()) {
		if (configDirty_) { ApplyConfig(); }
		else {
			if (sizeDirty_) { ApplySize(); }
			if (clearArgument_.dirty) { ApplyClearContent(); }
		}

		bindStatus_ = StatusRead;
		TextureInternal::Bind(index);
	}
}

void RenderTextureInternal::Unbind() {
	if (bindStatus_ == StatusNone) { return; }

	if (bindStatus_ == StatusWrite) {
		framebuffer_->Unbind();
	}
	else {
		TextureInternal::Unbind();
	}

	bindStatus_ = StatusNone;
}

void RenderTextureInternal::ApplySize() {
	BindTexture();
	ResizeStorage(width_, height_, renderTextureFormat_);
	UnbindTexture();

	sizeDirty_ = false;
}

void RenderTextureInternal::ApplyConfig() {
	DestroyTexture();
	DestroyFramebuffer();

	framebuffer_ = new Framebuffer(context_);

	context_->GenTextures(1, &texture_);
	BindTexture();

	ResizeStorage(width_, height_, renderTextureFormat_);

	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (renderTextureFormat_ == RenderTextureFormat::Shadow) {
		context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	}

	UnbindTexture();

	if (!IsDepthOrStencilTexture()) {
		framebuffer_->SetViewport(0, 0, width_, height_);
		framebuffer_->CreateDepthRenderbuffer();
		framebuffer_->SetRenderTexture(FramebufferAttachment0, texture_);
	}
	else {
		framebuffer_->SetDepthTexture(texture_);
	}

	configDirty_ = false;
}

void RenderTextureInternal::ApplyClearContent() {
	if (SetViewport(Rect::NormalizedToRect(Rect(0.f, 0.f, (float)GetWidth(), (float)GetHeight()), clearArgument_.normalizedRect))) {
		int flags = 0;
		if (clearArgument_.depth >= 0) {
			flags |= FramebufferClearMaskDepth;
			framebuffer_->SetClearDepth(clearArgument_.depth);
		}

		if (!IsDepthOrStencilTexture()) {
			flags |= FramebufferClearMaskColor;
			framebuffer_->SetClearColor(clearArgument_.color);
		}

		if (renderTextureFormat_ == RenderTextureFormat::DepthStencil && clearArgument_.stencil >= 0) {
			flags |= FramebufferClearMaskStencil;
			framebuffer_->SetClearStencil(clearArgument_.stencil);
		}

		framebuffer_->Clear((FramebufferClearMask)flags);
	}

	clearArgument_.dirty = false;
}

bool RenderTextureInternal::VerifyBindStatus() {
	if (bindStatus_ != StatusNone) {
		Debug::LogError("bind status error");
		return false;
	}

	return true;
}

void RenderTextureInternal::DestroyFramebuffer() {
	delete framebuffer_;
	framebuffer_ = nullptr;
}

bool RenderTextureInternal::SetViewport(const Rect& viewport) {
	framebuffer_->SetViewport((int)viewport.GetXMin(), (int)viewport.GetYMin(), (uint)Mathf::Max(0.f, viewport.GetWidth()), (uint)Mathf::Max(0.f, viewport.GetHeight()));
	return viewport.GetWidth() > 0 && viewport.GetHeight() > 0;
}

void RenderTextureInternal::OnContextDestroyed() {
	DestroyFramebuffer();
	TextureInternal::OnContextDestroyed();
}

void RenderTextureInternal::ResizeStorage(uint w, uint h, RenderTextureFormat renderTextureFormat) {
	uint internalFormat, format, type;
	RenderTextureFormatToGLenum(renderTextureFormat, internalFormat, format, type);
	context_->TexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, nullptr);
	internalFormat_ = internalFormat;
}

void RenderTextureInternal::RenderTextureFormatToGLenum(RenderTextureFormat input, uint& internalFormat, uint& format, uint& type) {
	internalFormat = GL_RGBA;
	format = GL_RGBA;
	type = GL_UNSIGNED_BYTE;

	switch (input) {
		case RenderTextureFormat::Rgb:
			internalFormat = GL_RGB;
			break;
		case RenderTextureFormat::RgbS:
			internalFormat = GL_RGB_SNORM;
			break;
		case  RenderTextureFormat::Rgba:
			internalFormat = GL_RGBA;
			break;
		case RenderTextureFormat::RgbaS:
			internalFormat = GL_RGBA_SNORM;
			break;
		case RenderTextureFormat::RgbF:
			internalFormat = GL_RGB32F;
			format = GL_RGB;
			type = GL_FLOAT;
			break;
		case RenderTextureFormat::RgbaF:
			internalFormat = GL_RGBA32F;
			type = GL_FLOAT;
			break;
		case RenderTextureFormat::Depth:
		case RenderTextureFormat::Shadow:
			internalFormat = GL_DEPTH_COMPONENT24;
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
			break;
		case RenderTextureFormat::DepthStencil:
			internalFormat = GL_DEPTH32F_STENCIL8;
			format = GL_DEPTH_STENCIL;
			type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
			break;
		default:
			Debug::LogError("invalid render texture format: %d.", input);
			break;
	}
}

TextureBufferInternal::TextureBufferInternal(Context* context) : TextureInternal(ObjectType::TextureBuffer, context) {
}

TextureBufferInternal::~TextureBufferInternal() {
	DestroyBuffer();
}

bool TextureBufferInternal::Create(uint size) {
	if (size_ != size) {
		size_ = size;
		sizeDirty_ = true;
	}

	return true;
}

void TextureBufferInternal::Update(uint offset, uint size, const void* data) {
	newContentArgument_.offset = offset;
	newContentArgument_.size = size;
	newContentArgument_.data.reset(new uchar[size]);
	memcpy(newContentArgument_.data.get(), data, size);
}

void TextureBufferInternal::Bind(uint index) {
	if (sizeDirty_) {
		ApplySize();
	}

	if (newContentArgument_.data) {
		ApplyContent();
	}

	TextureInternal::Bind(index);
}

void TextureBufferInternal::ApplyContent() {
	buffer_->Update(newContentArgument_.offset, newContentArgument_.size, newContentArgument_.data.get());
	newContentArgument_.data = nullptr;
}

void TextureBufferInternal::OnContextDestroyed() {
	DestroyBuffer();
	TextureInternal::OnContextDestroyed();
}

void TextureBufferInternal::ApplySize() {
	DestroyBuffer();
	DestroyTexture();

	buffer_ = new Buffer(context_);
	buffer_->Create(GL_TEXTURE_BUFFER, size_, nullptr, GL_STREAM_DRAW);

	context_->GenTextures(1, &texture_);
	BindTexture();
	context_->TexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_->GetNativePointer());
	UnbindTexture();

	sizeDirty_ = false;
}

void TextureBufferInternal::DestroyBuffer() {
	if (buffer_ != nullptr) {
		delete buffer_;
		buffer_ = nullptr;
	}
}

ScreenRenderTextureInternal::ScreenRenderTextureInternal(Context* context) : RenderTextureInternal(context) {
}

ScreenRenderTextureInternal::~ScreenRenderTextureInternal() {
	framebuffer_ = nullptr;
}

bool ScreenRenderTextureInternal::Create(RenderTextureFormat format, uint width, uint height) {
	return true;
}

uint ScreenRenderTextureInternal::GetWidth() const {
	return Screen::GetWidth();
}

uint ScreenRenderTextureInternal::GetHeight() const {
	return Screen::GetHeight();
}

void ScreenRenderTextureInternal::Bind(uint index) {
	if (framebuffer_ == nullptr) {
		framebuffer_ = Framebuffer::GetDefault();
	}

	RenderTextureInternal::Bind(index);
}

void ScreenRenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	if (framebuffer_ == nullptr) {
		framebuffer_ = Framebuffer::GetDefault();
	}

	if (clearArgument_.dirty) { ApplyClearContent(); }

	SetViewport(Rect::NormalizedToRect(Rect(0.f, 0.f, (float)Screen::GetWidth(), (float)Screen::GetHeight()), normalizedRect));
	framebuffer_->BindWrite();
}

void ScreenRenderTextureInternal::Unbind() {
	framebuffer_->Unbind();
}

void ScreenRenderTextureInternal::OnContextDestroyed() {
	framebuffer_ = nullptr;
	RenderTextureInternal::OnContextDestroyed();
}

void ScreenRenderTextureInternal::Resize(uint width, uint height) {
	Debug::LogError("screen render texture is not resizable.");
}

uint ScreenRenderTextureInternal::GetGLTextureType() const {
	Debug::LogError("unsupported operation for screen render texture.");
	return 0;
}

uint ScreenRenderTextureInternal::GetGLTextureBindingName() const {
	Debug::LogError("unsupported operation for screen render texture.");
	return 0;
}

bool MRTRenderTextureInternal::Create(RenderTextureFormat format, uint width, uint height) {
	if (format != RenderTextureFormat::Depth) {
		Debug::LogError("only RenderTextureFormatDepth is supported for MRTRenderTexture.");
		return false;
	}

	width_ = width;
	height_ = height;
	renderTextureFormat_ = format;
	configDirty_ = true;

	return true;
}

void MRTRenderTextureInternal::Resize(uint width, uint height) {
	if (width_ == width && height_ == height) {
		return;
	}

	width_ = width;
	height_ = height;
	sizeDirty_ = true;
}

void MRTRenderTextureInternal::Bind(uint index) {
	Debug::LogError("MRTRenderTexture is not readable.");
}

void MRTRenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	for (; newColorTextureFrom_ < currentIndex_; ++newColorTextureFrom_) {
		framebuffer_->SetRenderTexture(
			FramebufferAttachment(FramebufferAttachment0 + newColorTextureFrom_),
			colorTextures_[newColorTextureFrom_]->GetNativePointer()
		);
	}

	RenderTextureInternal::BindWrite(normalizedRect);
}

bool MRTRenderTextureInternal::AddColorTexture(TextureFormat format) {
	if (currentIndex_ >= FramebufferAttachmentMax) {
		Debug::LogError("only %d color textures are supported.", FramebufferAttachmentMax);
		return false;
	}

	colorTextures_[currentIndex_] = new Texture2D();
	colorTextures_[currentIndex_]->Create(format, nullptr, ColorStreamFormat::Rgba, width_, height_, 4);
	
	++currentIndex_;
	return true;
}

Texture2D* MRTRenderTextureInternal::GetColorTexture(uint index) {
	SUEDE_ASSERT(index < currentIndex_);
	return colorTextures_[index].get();
}

void MRTRenderTextureInternal::ApplySize() {
	for (int i = 0; i < currentIndex_; ++i) {
		ref_ptr<Texture2D>& texture = colorTextures_[i];
		colorTextures_[i]->Create(texture->GetFormat(), nullptr, ColorStreamFormat::Rgba, width_, height_, 4);
	}
}

void MRTRenderTextureInternal::ApplyConfig() {
	DestroyFramebuffer();
	DestroyColorTextures();
	framebuffer_ = new Framebuffer(context_);
	framebuffer_->SetViewport(0, 0, width_, height_);
	framebuffer_->CreateDepthRenderbuffer();
	configDirty_ = false;
}

uint MRTRenderTextureInternal::GetGLTextureType() const {
	Debug::LogError("unsupported operation for MRT render texture.");
	return 0;
}

uint MRTRenderTextureInternal::GetGLTextureBindingName() const {
	Debug::LogError("unsupported operation for MRT render texture.");
	return 0;
}

void MRTRenderTextureInternal::DestroyColorTextures() {
	for (int i = 0; i < currentIndex_; ++i) {
		colorTextures_[i] = nullptr;
	}
}
