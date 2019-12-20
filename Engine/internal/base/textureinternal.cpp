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
bool Texture2D::Create(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap) {
	return _suede_dptr()->Create(textureFormat, data, format, width, height, alignment, mipmap);
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
RenderTextureFormat RenderTexture::GetRenderTextureFormat() { return _suede_dptr()->GetRenderTextureFormat(); }
void RenderTexture::Resize(uint width, uint height) { _suede_dptr()->Resize(width, height); }
void RenderTexture::Clear(const Rect& normalizedRect, const Color& color, float depth) { _suede_dptr()->Clear(normalizedRect, color, depth); }
void RenderTexture::BindWrite(const Rect& normalizedRect) { _suede_dptr()->BindWrite(normalizedRect); }
RenderTexture* RenderTexture::GetDefault() { return RenderTextureInternal::GetDefault(); }
RenderTexture* RenderTexture::GetTemporary(RenderTextureFormat format, uint width, uint height) { return RenderTextureInternal::GetTemporary(format, width, height); }
void RenderTexture::ReleaseTemporary(RenderTexture* texture) { RenderTextureInternal::ReleaseTemporary(texture); }

MRTRenderTexture::MRTRenderTexture() : RenderTexture(new MRTRenderTextureInternal(Context::GetCurrent())) {}
bool MRTRenderTexture::AddColorTexture(TextureFormat format) { return _suede_dptr()->AddColorTexture(format); }
Texture2D* MRTRenderTexture::GetColorTexture(uint index) { return _suede_dptr()->GetColorTexture(index); }
uint MRTRenderTexture::GetColorTextureCount() { return _suede_dptr()->GetColorTextureCount(); }

ScreenRenderTexture::ScreenRenderTexture() : RenderTexture(new ScreenRenderTextureInternal(Context::GetCurrent())){}

TextureInternal::TextureInternal(ObjectType type, Context* context) :ObjectInternal(type)
	, context_(context), texture_(0), width_(0), height_(0), location_(0), internalFormat_(0) {
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
}

void TextureInternal::Unbind() {
	if (location_ != 0) {
		//context_->ActiveTexture(location_);
		UnbindTexture();
		location_ = 0;
	}
}

void TextureInternal::SetMinFilterMode(TextureMinFilterMode value) {
	BindTexture();
	context_->TexParameteri(GetGLTextureType(), GL_TEXTURE_MIN_FILTER, TextureMinFilterModeToGLenum(value));
	UnbindTexture();
}

TextureMinFilterMode TextureInternal::GetMinFilterMode() const {
	BindTexture();
	int parameter = 0;
	context_->GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_MIN_FILTER, &parameter);
	UnbindTexture();
	return GLenumToTextureMinFilterMode(parameter);
}

void TextureInternal::SetMagFilterMode(TextureMagFilterMode value) {
	BindTexture();
	context_->TexParameteri(GetGLTextureType(), GL_TEXTURE_MAG_FILTER, TextureMagFilterModeToGLenum(value));
	UnbindTexture();
}

TextureMagFilterMode TextureInternal::GetMagFilterMode() const {
	BindTexture();
	int parameter = 0;
	context_->GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_MAG_FILTER, &parameter);
	UnbindTexture();
	return GLenumToTextureMagFilterMode(parameter);
}

void TextureInternal::SetWrapModeS(TextureWrapMode value) {
	BindTexture();
	context_->TexParameteri(GetGLTextureType(), GL_TEXTURE_WRAP_S, TextureWrapModeToGLenum(value));
	UnbindTexture();
}

TextureWrapMode TextureInternal::GetWrapModeS() const {
	BindTexture();
	int parameter = 0;
	context_->GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_WRAP_S, &parameter);
	UnbindTexture();
	return GLenumToTextureWrapMode(parameter);
}

void TextureInternal::SetWrapModeT(TextureWrapMode value) {
	BindTexture();
	context_->TexParameteri(GetGLTextureType(), GL_TEXTURE_WRAP_T, TextureWrapModeToGLenum(value));
	UnbindTexture();
}

TextureWrapMode TextureInternal::GetWrapModeT() const {
	BindTexture();
	int parameter = 0;
	context_->GetTexParameteriv(GetGLTextureType(), GL_TEXTURE_WRAP_T, &parameter);
	UnbindTexture();
	return GLenumToTextureWrapMode(parameter);
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

BPPType TextureInternal::GLenumToBpp(uint format) const {
	switch (format) {
		case GL_RGB: return BPPType24;
		case GL_RGBA: return BPPType32;
	}

	Debug::LogError("unknown internal format 0x%x.", format);
	return BPPType24;
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

void TextureInternal::ColorStreamFormatToGLenum(uint(&parameters)[2], ColorStreamFormat format) const {
	uint glFormat = GL_RGBA, glType = GL_UNSIGNED_BYTE;
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

	parameters[0] = glFormat;
	parameters[1] = glType;
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
	TexelMap texelMap;
	if (!ImageCodec::Decode(texelMap, Resources::textureDirectory + path)) {
		return false;
	}

	return Create(texelMap.textureFormat, &texelMap.data[0], texelMap.colorStreamFormat, texelMap.width, texelMap.height, texelMap.alignment);
}

bool Texture2DInternal::Create(TextureFormat textureFormat, const void* data, ColorStreamFormat format, uint width, uint height, uint alignment, bool mipmap) {
	DestroyTexture();

	width_ = width;
	height_ = height;

	context_->GenTextures(1, &texture_);

	BindTexture();

	uint glFormat[2];
	ColorStreamFormatToGLenum(glFormat, format);
	uint internalFormat = TextureFormatToGLenum(textureFormat);

	format_ = textureFormat;

	int oldUnpackAlignment = 4;
	context_->GetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	context_->PixelStorei(GL_UNPACK_ALIGNMENT, alignment);

	context_->TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat[0], glFormat[1], data);

	context_->PixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);

	if (mipmap) {
		context_->GenerateMipmap(GL_TEXTURE_2D);
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
	context_->GetIntegerv(GL_UNPACK_ALIGNMENT, (int*)&alignment);

	texelMap.alignment = alignment;
	BPPType bpp = GLenumToBpp(internalFormat_);

	texelMap.data.resize((bpp / 8) * Mathf::RoundUpToPowerOfTwo(GetWidth(), alignment) * GetHeight());
	context_->GetTexImage(GL_TEXTURE_2D, 0, internalFormat_, GL_UNSIGNED_BYTE, &texelMap.data[0]);
	UnbindTexture();

	texelMap.textureFormat = (bpp == BPPType24) ? TextureFormat::Rgb : TextureFormat::Rgba;
	texelMap.colorStreamFormat = (bpp == BPPType24) ? ColorStreamFormat::Rgb : ColorStreamFormat::Rgba;

	return ImageCodec::Encode(data, type, texelMap);
}

TextureCubeInternal::TextureCubeInternal(Context* context) : TextureInternal(ObjectType::TextureCube, context) {
}

TextureCubeInternal::~TextureCubeInternal() {
}

bool TextureCubeInternal::Load(const std::string textures[6]) {
	TexelMap texelMaps[6];
	for (int i = 0; i < 6; ++i) {
		if (!ImageCodec::Decode(texelMaps[i], Resources::textureDirectory + textures[i])) {
			return false;
		}
	}

	DestroyTexture();

	context_->GenTextures(1, &texture_);
	BindTexture();

	for(int i = 0; i < 6; ++i) {
		uint glFormat[2];
		ColorStreamFormatToGLenum(glFormat, texelMaps[i].colorStreamFormat);
		uint internalFormat = TextureFormatToGLenum(texelMaps[i].textureFormat);
		context_->TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, texelMaps[i].width, texelMaps[i].height, 0, glFormat[0], glFormat[1], &texelMaps[i].data[0]);

		internalFormat_ = internalFormat;
		
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		context_->TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	UnbindTexture();
	return true;
}

RenderTexture* RenderTextureInternal::GetDefault() {
	static ref_ptr<RenderTexture> screen;

	if (!screen) {
		screen = new ScreenRenderTexture();
		screen->Create(RenderTextureFormat::Rgb, 0, 0);
	}

	return screen.get();
}

struct RenderTextureCacheKey {
	int busy;
	RenderTextureFormat format;
	uint width;
	uint height;

	bool operator<(const RenderTextureCacheKey& other) const {
		if (busy != other.busy) { return busy < other.busy; }
		if (format != other.format) { return format < other.format; }
		if (width != other.width) { return width < other.width; }
		return height < other.height;
	}
};

typedef std::multimap<RenderTextureCacheKey, ref_ptr<RenderTexture>> RenderTextureCacheContainer;
static RenderTextureCacheContainer renderTextureCache;

static RenderTexture* FindRenderTextureCache(RenderTextureFormat format, uint width, uint height, int busy) {
	RenderTextureCacheKey key = { busy, format, width,height };
	RenderTextureCacheContainer::iterator pos = renderTextureCache.find(key);
	if (pos != renderTextureCache.end()) {
		ref_ptr<RenderTexture> value = pos->second;
		renderTextureCache.erase(pos);
		key.busy = 1 - busy;
		renderTextureCache.insert(std::make_pair(key, value));

		return value.get();
	}

	return nullptr;
}

RenderTexture* RenderTextureInternal::GetTemporary(RenderTextureFormat format, uint width, uint height) {
	RenderTexture* cache = FindRenderTextureCache(format, width, height, 0);
	if (cache != nullptr) {
		return cache;
	}

	RenderTexture* texture = new RenderTexture();
	texture->Create(format, width, height);
	renderTextureCache.insert(std::make_pair(RenderTextureCacheKey { 1, format, width,height } , texture));
	return texture;
}

void RenderTextureInternal::ReleaseTemporary(RenderTexture* texture) {
	RenderTexture* cache = FindRenderTextureCache(texture->GetRenderTextureFormat(), texture->GetWidth(), texture->GetHeight(), 1);
	SUEDE_ASSERT(cache != nullptr);
}

RenderTextureInternal::RenderTextureInternal(Context* context)
	: TextureInternal(ObjectType::RenderTexture, context), bindStatus_(StatusNone), renderTextureFormat_(RenderTextureFormat::Rgba), framebuffer_(nullptr) {
}

RenderTextureInternal::~RenderTextureInternal() {
	DestroyFramebuffer();
}

bool RenderTextureInternal::Create(RenderTextureFormat format, uint width, uint height) {
	DestroyTexture();
	DestroyFramebuffer();

	width_ = width;
	height_ = height;

	framebuffer_ = new Framebuffer(context_);

	context_->GenTextures(1, &texture_);
	BindTexture();

	renderTextureFormat_ = format;
	ResizeStorage(width, height, format);

	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (format == RenderTextureFormat::Shadow) {
		context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		context_->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
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

void RenderTextureInternal::Clear(const Rect& normalizedRect, const Color& color, float depth) {
	if (!SetViewport(width_, height_, normalizedRect)) {
		return;
	}

	if (ContainsDepthInfo()) {
		framebuffer_->SetClearDepth(depth);
		framebuffer_->Clear(FramebufferClearMaskDepth);
	}
	else {
		framebuffer_->SetClearDepth(depth);
		framebuffer_->SetClearColor(color);
		framebuffer_->Clear(FramebufferClearMaskColorDepth);
	}
}

void RenderTextureInternal::Resize(uint width, uint height) {
	if (width_ != width || height_ != height) {
		BindTexture();
		ResizeStorage(width, height, renderTextureFormat_);
		UnbindTexture();
	}
}

void RenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	if (!VerifyBindStatus()) { return; }
	
	bindStatus_ = StatusWrite;
	SetViewport(width_, height_, normalizedRect);
	framebuffer_->BindWrite();
}

void RenderTextureInternal::Bind(uint index) {
	if (!VerifyBindStatus()) { return; }

	bindStatus_ = StatusRead;
	TextureInternal::Bind(index);
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

bool RenderTextureInternal::VerifyBindStatus() {
	if (bindStatus_ != StatusNone) {
		Debug::LogError("bind status error");
		return false;
	}

	return true;
}

void RenderTextureInternal::DestroyFramebuffer() {
	delete framebuffer_;
}

bool RenderTextureInternal::SetViewport(uint width, uint height, const Rect& normalizedRect) {
	Rect viewport = Rect::NormalizedToRect(Rect(0.f, 0.f, (float)width, (float)height), normalizedRect);
	framebuffer_->SetViewport((int)viewport.GetXMin(), (int)viewport.GetYMin(), (uint)Mathf::Max(0.f, viewport.GetWidth()), (uint)Mathf::Max(0.f, viewport.GetHeight()));
	return viewport.GetWidth() > 0 && viewport.GetHeight() > 0;
}

void RenderTextureInternal::ResizeStorage(uint w, uint h, RenderTextureFormat format) {
	uint glFormat[3];
	RenderTextureFormatToGLenum(format, glFormat);
	context_->TexImage2D(GL_TEXTURE_2D, 0, glFormat[0], w, h, 0, glFormat[1], glFormat[2], nullptr);
	width_ = w;
	height_ = h;
	internalFormat_ = glFormat[0];
}

void RenderTextureInternal::RenderTextureFormatToGLenum(RenderTextureFormat input, uint(&parameters)[3]) {
	uint internalFormat = GL_RGBA;
	uint format = GL_RGBA;
	uint type = GL_UNSIGNED_BYTE;

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
			internalFormat = GL_DEPTH_COMPONENT16;
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

	parameters[0] = internalFormat;
	parameters[1] = format;
	parameters[2] = type;
}

TextureBufferInternal::TextureBufferInternal(Context* context) : TextureInternal(ObjectType::TextureBuffer, context), buffer_(nullptr) {
}

TextureBufferInternal::~TextureBufferInternal() {
	DestroyBuffer();
}

bool TextureBufferInternal::Create(uint size) {
	DestroyBuffer();
	DestroyTexture();

	buffer_ = new Buffer(context_);
	buffer_->Create(GL_TEXTURE_BUFFER, size, nullptr, GL_STREAM_DRAW);
	
	context_->GenTextures(1, &texture_);
	BindTexture();
	context_->TexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_->GetNativePointer());
	UnbindTexture();

	return true;
}

uint TextureBufferInternal::GetSize() const {
	return buffer_->GetSize();
}

void TextureBufferInternal::Update(uint offset, uint size, const void* data) {
	buffer_->Update(offset, size, data);
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
	framebuffer_ = Framebuffer::GetDefault();
	return true;
}

void ScreenRenderTextureInternal::Clear(const Rect& normalizedRect, const Color& color, float depth) {
	SetViewport(Screen::GetWidth(), Screen::GetHeight(), normalizedRect);

	framebuffer_->SetClearDepth(depth);
	framebuffer_->SetClearColor(color);
	framebuffer_->Clear(FramebufferClearMaskColorDepth);
}

uint ScreenRenderTextureInternal::GetWidth() const {
	return Screen::GetWidth();
}

uint ScreenRenderTextureInternal::GetHeight() const {
	return Screen::GetHeight();
}

void ScreenRenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	SetViewport(Screen::GetWidth(), Screen::GetHeight(), normalizedRect);
	framebuffer_->BindWrite();
}

void ScreenRenderTextureInternal::Unbind() {
	framebuffer_->Unbind();
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
	if (format != +RenderTextureFormat::Depth) {
		Debug::LogError("only RenderTextureFormatDepth is supported for MRTRenderTexture.");
		return false;
	}

	DestroyFramebuffer();
	DestroyColorTextures();

	width_ = width;
	height_ = height;

	framebuffer_ = new Framebuffer(context_);
	framebuffer_->SetViewport(0, 0, width, height);
	framebuffer_->CreateDepthRenderbuffer();

	return true;
}

void MRTRenderTextureInternal::Resize(uint width, uint height) {
	if (width_ == width && height_ == height) {
		return;
	}

	width_ = width;
	height_ = height;

	for (int i = 0; i < index_; ++i) {
		ref_ptr<Texture2D>& texture = colorTextures_[i];
		colorTextures_[i]->Create(texture->GetFormat(), nullptr, ColorStreamFormat::Rgba, width, height, 4);
	}
}

void MRTRenderTextureInternal::Bind(uint index) {
	Debug::LogError("MRTRenderTexture is not readable.");
}

void MRTRenderTextureInternal::BindWrite(const Rect& normalizedRect) {
	RenderTextureInternal::BindWrite(normalizedRect);
}

bool MRTRenderTextureInternal::AddColorTexture(TextureFormat format) {
	if (index_ >= FramebufferAttachmentMax) {
		Debug::LogError("only %d color textures are supported.", FramebufferAttachmentMax);
		return false;
	}

	colorTextures_[index_] = new Texture2D();
	colorTextures_[index_]->Create(format, nullptr, ColorStreamFormat::Rgba, width_, height_, 4);
	framebuffer_->SetRenderTexture(FramebufferAttachment(FramebufferAttachment0 + index_), colorTextures_[index_]->GetNativePointer());
	++index_;
	return true;
}

Texture2D* MRTRenderTextureInternal::GetColorTexture(uint index) {
	SUEDE_VERIFY_INDEX(index, index_, nullptr);
	return colorTextures_[index].get();
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
	for (int i = 0; i < index_; ++i) {
		colorTextures_[i] = nullptr;
	}
}
