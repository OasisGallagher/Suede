#include "ambientocclusion.h"

#include "screen.h"
#include "graphics.h"
#include "resources.h"
#include "math/random.h"
#include "internal/base/renderdefines.h"

AmbientOcclusion::AmbientOcclusion(RenderTexture* target) : ssaoRT_(target) {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();
	ssaoTraversalRT_ = new MRTRenderTexture();
	ssaoTraversalRT_->Create(RenderTextureFormat::Depth, w, h);

	ssaoTraversalRT_->AddColorTexture(TextureFormat::Rgb32F);
	ssaoTraversalRT_->AddColorTexture(TextureFormat::Rgb32F);

	ssaoMaterial_ = new Material();
	ssaoMaterial_->SetShader(Shader::Find("builtin/ssao"));
	ssaoMaterial_->SetRenderQueue((int)RenderQueue::Background);

	ssaoTraversalMaterial_ = new Material();
	ssaoTraversalMaterial_->SetShader(Shader::Find("builtin/ssao_traversal"));
	ssaoTraversalMaterial_->SetRenderQueue((int)RenderQueue::Background);

	InitializeKernel();
}

void AmbientOcclusion::Run(RenderTexture* depthTexture, const Rect& normalizedRect) {
	RenderTexture* temp = RenderTexture::GetTemporary(RenderTextureFormat::Rgb, Screen::GetWidth(), Screen::GetHeight());

	ssaoMaterial_->SetPass(0);
	Graphics::Blit(depthTexture, temp, ssaoMaterial_.get(), normalizedRect);

	ssaoMaterial_->SetPass(1);
	Graphics::Blit(temp, ssaoRT_, ssaoMaterial_.get(), normalizedRect);

	RenderTexture::ReleaseTemporary(temp);
}

void AmbientOcclusion::Clear(const Rect& normalizedRect) {
	ssaoRT_->Clear(normalizedRect, Color::white, 1);
	ssaoTraversalRT_->Clear(normalizedRect, Color::black, 1);
}

void AmbientOcclusion::InitializeKernel() {
	Vector3 kernel[SSAO_KERNEL_SIZE];
	for (int i = 0; i < SUEDE_COUNTOF(kernel); ++i) {
		float scale = float(i) / SUEDE_COUNTOF(kernel);
		scale = Mathf::Lerp(0.1f, 1.f, scale * scale);

		Vector3 sample = Vector3(Random::FloatRange(-1.f, 1.f), Random::FloatRange(-1.f, 1.f), Random::FloatRange(0.f, 1.f));
		Vector3::Normalize(sample);
		sample *= Random::FloatRange(0.f, 1.f);
		kernel[i] = sample * scale;
	}

	Vector3 noise[4 * 4];
	for (int i = 0; i < SUEDE_COUNTOF(noise); ++i) {
		noise[i] = Vector3(Random::FloatRange(-1.f, 1.f), Random::FloatRange(-1.f, 1.f), 0);
	}

	ref_ptr<Texture2D> noiseTexture = new Texture2D();
	noiseTexture->Create(TextureFormat::Rgb32F, &noise, ColorStreamFormat::RgbF, 4, 4, 4);
	noiseTexture->SetWrapModeS(TextureWrapMode::Repeat);
	noiseTexture->SetWrapModeT(TextureWrapMode::Repeat);

	ssaoMaterial_->SetVector3Array("ssaoKernel", kernel, SSAO_KERNEL_SIZE);

	ssaoMaterial_->SetTexture("noiseTexture", noiseTexture.get());
	ssaoMaterial_->SetTexture("posTexture", ssaoTraversalRT_->GetColorTexture(0));
	ssaoMaterial_->SetTexture("normalTexture", ssaoTraversalRT_->GetColorTexture(1));
}
